#include "MeshCollision.h"
#include "../math/Math.h"
#include <cassert>
#include <cstdlib>
#include <memory.h>

MeshCollision::MeshCollision()
{
    m_pWorldMtx = nullptr;
    m_eRadius.Set(0,0,0);
    m_vOffset.Set(0,0,0);
    m_Polygons = nullptr;
    m_nPolygonCount = 0;
}

MeshCollision::~MeshCollision()
{
    if ( m_Polygons )
    {
        for (int p=0; p<m_nPolygonCount; p++)
        {
            xlDelete[] m_Polygons[p].pVertexLS;
            xlDelete[] m_Polygons[p].pVertexES;
        }
        xlDelete [] m_Polygons;
        m_Polygons = nullptr;
    }
}

void MeshCollision::Transform()
{
    Vector3 eScale( 1.0f / m_eRadius.x, 1.0f / m_eRadius.y, 1.0f / m_eRadius.z );

    for (int p=0; p<m_nPolygonCount; p++)
    {
        for (int v=0; v<m_Polygons[p].numVerts; v++)
        {
            m_Polygons[p].pVertexES[v] = m_pWorldMtx->TransformVector( m_Polygons[p].pVertexLS[v] ) + m_vOffset;
            m_Polygons[p].pVertexES[v] = m_Polygons[p].pVertexES[v] * eScale;
        }

        //compute polygon plane.
        if ( m_Polygons[p].plane.Build( m_Polygons[p].pVertexES[0], m_Polygons[p].pVertexES[1], m_Polygons[p].pVertexES[2] ) == false )
        {
            m_Polygons[p].bValid = false;
        }
        else
        {
            m_Polygons[p].bValid = true;
        }
    }
}

void MeshCollision::SetMaxPolygonCount(int maxPolyCount)
{
    m_Polygons = xlNew CollisionPolygon[maxPolyCount];
}

void MeshCollision::AddPolygon(int numVerts, Vector3 *pvVertices )
{
    m_Polygons[ m_nPolygonCount ].bValid    = false;
    m_Polygons[ m_nPolygonCount ].numVerts  = numVerts;
    m_Polygons[ m_nPolygonCount ].pVertexLS = xlNew Vector3[numVerts];
    m_Polygons[ m_nPolygonCount ].pVertexES = xlNew Vector3[numVerts];

    memcpy( m_Polygons[ m_nPolygonCount ].pVertexLS, pvVertices, numVerts*sizeof(Vector3) );
    m_nPolygonCount++;
}

bool MeshCollision::Collide(CollisionPacket *packet, Matrix *pWorldMtx, const Vector3& vOffset)
{
    //transform the collision mesh if needed.
    if ( pWorldMtx != m_pWorldMtx || m_eRadius != packet->eRadius || vOffset.x != m_vOffset.x || vOffset.y != m_vOffset.y || vOffset.z != m_vOffset.z )
    {
        m_pWorldMtx = pWorldMtx;
        m_eRadius   = packet->eRadius;
        m_vOffset   = vOffset;
        Transform();
    }

    //check for oollisions.
    bool bCollisionFound = false;
    for (int p=0; p<m_nPolygonCount; p++)
    {
        //find a triangle collision.
        for (int ii=0; ii<m_Polygons[p].numVerts-2; ii++)
        {
            if ( CheckTriangle(packet, m_Polygons[p].pVertexES[0], m_Polygons[p].pVertexES[ii+1],  m_Polygons[p].pVertexES[ii+2], m_Polygons[p].plane) )
            {
                bCollisionFound = true;
            }
        }
    }

    return bCollisionFound;
}

bool MeshCollision::Raycast(RaycastPacket *packet, Matrix *pWorldMtx, Object *parent, Sector *pSector, const Vector3& vOffset)
{
    //transform the collision mesh if needed.
    if ( pWorldMtx != m_pWorldMtx || m_eRadius != Vector3::One || vOffset.x != m_vOffset.x || vOffset.y != m_vOffset.y || vOffset.z != m_vOffset.z )
    {
        m_pWorldMtx = pWorldMtx;
        m_eRadius   = Vector3::One;
        m_vOffset   = vOffset;
        Transform();
    }

    //check for oollisions.
    bool bCollisionFound = false;
    for (int p=0; p<m_nPolygonCount; p++)
    {
        //find a triangle collision.
        for (int ii=0; ii<m_Polygons[p].numVerts-2; ii++)
        {
            if ( RayTriangle(packet, m_Polygons[p].pVertexES[0], m_Polygons[p].pVertexES[ii+1], m_Polygons[p].pVertexES[ii+2], m_Polygons[p].plane) )
            {
                bCollisionFound = true;
                packet->pCollisionObj    = parent;
                packet->pCollisionSector = pSector;
            }
        }
    }

    return bCollisionFound;
}

#define in(a) ((uint32_t&) a)
bool CheckPointInTriangle(const Vector3& point, const Vector3& pa, const Vector3& pb, const Vector3& pc)
{
    Vector3 e10 = pb - pa;
    Vector3 e20 = pc - pa;

    float a = e10.Dot(e10);
    float b = e10.Dot(e20);
    float c = e20.Dot(e20);
    float ac_bb = (a*c) - (b*b);
    Vector3 vp(point.x-pa.x, point.y-pa.y, point.z-pa.z);

    float d = vp.Dot(e10);
    float e = vp.Dot(e20);
    float x = (d*c) - (e*b);
    float y = (e*a) - (d*b);
    float z = x+y - ac_bb;

    return (( in(z)& ~(in(x)|in(y)) ) & 0x80000000) ? true : false;
}

bool getLowestRoot(float a, float b, float c, float maxR, float *root)
{
    //Does the solution exist?
    float determinant = b*b - 4.0f*a*c;
    if (determinant < 0.0f) return false;

    //calculate the two roots
    float sqrtD = sqrtf(determinant);
    float r1 = (-b - sqrtD) / (2.0f*a);
    float r2 = (-b + sqrtD) / (2.0f*a);

    //sort x1 <= x2
    if ( r1 > r2 )
    {
        float temp = r2;
        r2 = r1;
        r1 = temp;
    }

    if ( r1 > 0.0f && r1 < maxR )
    {
        *root = r1;
        return true;
    }

    if ( r2 > 0.0f && r2 < maxR )
    {
        *root = r2;
        return true;
    }

    //no valid solutions.
    return false;
}

bool MeshCollision::CheckTriangle(CollisionPacket *col, const Vector3& p1, const Vector3& p2, const Vector3& p3, const Plane& plane)
{
    bool bCollisionFound = false;
    float facing = plane.Dot(col->R3VelocityNorm);
    Vector3 Nt(plane.a, plane.b, plane.c);
    if ( facing <= 0.0f )
    {
        float t0, t1;
        bool embeddedInPlane = false;

        Vector3 triNrml( plane.a, plane.b, plane.c );

        float signedDistToTrianglePlane = plane.Distance(col->R3Position);
        float normalDotVelocity = plane.Dot( col->R3Velocity );

        if ( normalDotVelocity == 0.0f )
        {
            if ( fabs(signedDistToTrianglePlane) >= 1.0f )
            {
                //not embedded, no collision possible.
                return false;
            }
            else
            {
                //sphere is embedded in the plane
                embeddedInPlane = true;
                t0 = 0.0f; t1 = 1.0f;
            }
        }
        else
        {
            t0 = ( -1.0f-signedDistToTrianglePlane ) / normalDotVelocity;
            t1 = (  1.0f-signedDistToTrianglePlane ) / normalDotVelocity;

            if ( t0 > t1 )  //swap
            {
                float temp = t1;
                t1 = t0;
                t0 = temp;
            }

            if ( t0 > 1.0f || t1 < 0.0f )
            {
                //outside of range, no collision
                return false;
            }

            t0 = Math::clamp(t0, 0.0, 1.0);
            t1 = Math::clamp(t1, 0.0, 1.0);
        }

        //the collision interval, if it exists, is [t0,t1]
        Vector3 collisionPoint;
        bool foundCollision = false;
        float t = 1.0f;

        if ( embeddedInPlane == false )
        {
            Vector3 planeIntersectionPoint = (col->R3Position - Nt) + col->R3Velocity*(float)t0;
            if ( CheckPointInTriangle(planeIntersectionPoint, p1, p2, p3) )
            {
                foundCollision = true;
                t = t0;
                collisionPoint = planeIntersectionPoint;
            }
        }
        else if ( signedDistToTrianglePlane > 0.0f && triNrml.z > 0.0f )
        {
            //compute a push vector?
            Vector3 planeIntersectionPoint = col->R3Position + triNrml*(1.0f - signedDistToTrianglePlane);
            if ( CheckPointInTriangle(col->R3Position - triNrml*signedDistToTrianglePlane, p1, p2, p3) )
            {
                col->bEmbedded = true;
                col->pushVec = (planeIntersectionPoint - col->R3Position) * col->eRadius;
            }
        }

        if ( foundCollision == false )
        {
            Vector3 velocity = col->R3Velocity;
            Vector3 base = col->R3Position;
            float velocitySquaredLength = velocity.Mag2();
            float a, b, c;
            float newT;

            //solve quadratic equation to determine edge/vertex collision
            a = velocitySquaredLength;

            //check vertices
            //P1
            Vector3 pb = base-p1;
            b = 2.0f * velocity.Dot(pb);
            c = pb.Mag2() - 1.0f;
            if ( getLowestRoot(a, b, c, t, &newT) )
            {
                t = newT;
                foundCollision = true;
                collisionPoint = p1;
            }

            //P2
            pb = base-p2;
            b = 2.0f * velocity.Dot(pb);
            c = pb.Mag2() - 1.0f;
            if ( getLowestRoot(a, b, c, t, &newT) )
            {
                t = newT;
                foundCollision = true;
                collisionPoint = p2;
            }

            //P3
            pb = base-p3;
            b = 2.0f * velocity.Dot(pb);
            c = pb.Mag2() - 1.0f;
            if ( getLowestRoot(a, b, c, t, &newT) )
            {
                t = newT;
                foundCollision = true;
                collisionPoint = p3;
            }

            //check edges
            //p1 -> p2
            Vector3 edge = p2 - p1;
            Vector3 baseToVertex = p1 - base;
            float edgeSquaredLength = edge.Mag2();
            float edgeDotVelocity = edge.Dot(velocity);
            float edgeDotBaseToVertex = edge.Dot(baseToVertex);

            a = -edgeSquaredLength*velocitySquaredLength + edgeDotVelocity*edgeDotVelocity;
            b =  edgeSquaredLength*(2.0f*velocity.Dot(baseToVertex)) - 2.0f*edgeDotVelocity*edgeDotBaseToVertex;
            c =  edgeSquaredLength*(1.0f-baseToVertex.Mag2()) + edgeDotBaseToVertex*edgeDotBaseToVertex;

            if ( getLowestRoot(a, b, c, t, &newT) )
            {
                //is the intersection within the line segment?
                float f = (edgeDotVelocity*newT - edgeDotBaseToVertex) / edgeSquaredLength;
                if ( f >= 0.0f && f <= 1.0f )
                {
                    t = newT;
                    foundCollision = true;
                    collisionPoint = p1 + edge*f;
                }
            }

            //p2 -> p3
            edge = p3 - p2;
            baseToVertex = p2 - base;
            edgeSquaredLength = edge.Mag2();
            edgeDotVelocity = edge.Dot(velocity);
            edgeDotBaseToVertex = edge.Dot(baseToVertex);

            a = -edgeSquaredLength*velocitySquaredLength + edgeDotVelocity*edgeDotVelocity;
            b =  edgeSquaredLength*(2.0f*velocity.Dot(baseToVertex)) - 2.0f*edgeDotVelocity*edgeDotBaseToVertex;
            c =  edgeSquaredLength*(1.0f-baseToVertex.Mag2()) + edgeDotBaseToVertex*edgeDotBaseToVertex;

            if ( getLowestRoot(a, b, c, t, &newT) )
            {
                //is the intersection within the line segment?
                float f = (edgeDotVelocity*newT - edgeDotBaseToVertex) / edgeSquaredLength;
                if ( f >= 0.0f && f <= 1.0f )
                {
                    t = newT;
                    foundCollision = true;
                    collisionPoint = p2 + edge*f;
                }
            }

            //p3 -> p1
            edge = p1 - p3;
            baseToVertex = p3 - base;
            edgeSquaredLength = edge.Mag2();
            edgeDotVelocity = edge.Dot(velocity);
            edgeDotBaseToVertex = edge.Dot(baseToVertex);

            a = -edgeSquaredLength*velocitySquaredLength + edgeDotVelocity*edgeDotVelocity;
            b =  edgeSquaredLength*(2.0f*velocity.Dot(baseToVertex)) - 2.0f*edgeDotVelocity*edgeDotBaseToVertex;
            c =  edgeSquaredLength*(1.0f-baseToVertex.Mag2()) + edgeDotBaseToVertex*edgeDotBaseToVertex;

            if ( getLowestRoot(a, b, c, t, &newT) )
            {
                //is the intersection within the line segment?
                float f = (edgeDotVelocity*newT - edgeDotBaseToVertex) / edgeSquaredLength;
                if ( f >= 0.0f && f <= 1.0f )
                {
                    t = newT;
                    foundCollision = true;
                    collisionPoint = p3 + edge*f;
                }
            }
        }
        //The final result
        if ( foundCollision == true  )
        {
            float distToCollision = t*col->R3Velocity.Length();

            if ( col->bFoundCollision == false || distToCollision < col->nearestDist )
            {
                col->nearestDist = distToCollision;
                col->intersectionPoint = collisionPoint;
                col->bFoundCollision = true;
                col->intNormal = triNrml;
                bCollisionFound = true;
                if (Nt.z > 0.2f )
                {
                    col->bOnGround = true;
                }
                else
                {
                    col->bOnGround = false;
                }
            }
        }
    }
    return bCollisionFound;
}

bool MeshCollision::RayTriangle(RaycastPacket *col, const Vector3& p1, const Vector3& p2, const Vector3& p3, const Plane& plane)
{
    bool bCollisionFound = false;
    float fDenom = -plane.Dot(col->rayDir);
    if ( fDenom )
    {
        float dist = plane.Distance(col->rayOrigin) / fDenom;
        if ( dist > 0.0f && dist < col->nearestDist )
        {
            Vector3 vI = col->rayOrigin + col->rayDir*dist;
            if ( CheckPointInTriangle(vI, p1, p2, p3) )
            {
                bCollisionFound = true;
                col->bFoundIntersection = true;
                col->nearestDist = dist;
                col->intersectionPoint = vI;
            }
        }
    }
    return bCollisionFound;
}
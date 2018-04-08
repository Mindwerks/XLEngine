#include "WorldCell.h"
#include "Sector.h"
#include "Sector_2_5D.h"
#include "../ui/XL_Console.h"
#include "../render/IDriver3D.h"
#include "../render/RenderQue.h"
#include "../math/Math.h"
#include "../world/CollisionComponent.h"
#include "../world/Object.h"
#include "../world/LogicDef.h"
#include <string.h>
#include <stdio.h>

#define MAX_COLLISION_ITER 3

WorldCell::WorldCell()
{
    m_Bounds[0].Set(0.0f, 0.0f, 0.0f);
    m_Bounds[1].Set(0.0f, 0.0f, 0.0f);
    m_bLockCamera = false;
    m_nCollisionIter = 0;
    m_nType = 0;
}

WorldCell::~WorldCell()
{
    for (uint32_t i=0; i<(uint32_t)m_Sectors.size(); i++)
    {
        Sector *pSector = m_Sectors[i];
        if ( pSector )
        {
            xlDelete m_Sectors[i];
        }
    }
    m_Sectors.clear();
}

void WorldCell::SetBounds(const Vector3& rvMin, const Vector3& rvMax)
{
    m_Bounds[0] = rvMin;
    m_Bounds[1] = rvMax;
}

void WorldCell::GetBounds(Vector3& rvMin, Vector3& rvMax)
{
    rvMin = m_Bounds[0];
    rvMax = m_Bounds[1];
}

uint32_t WorldCell::GetSectorCount()
{
    return (uint32_t)m_Sectors.size();
}

void WorldCell::AddSector(Sector *pSector)
{
    pSector->m_uID = (uint32_t)m_Sectors.size();
    m_Sectors.push_back( pSector );
}

Sector *WorldCell::GetSector(uint32_t uIndex)
{
    return m_Sectors[uIndex];
}

int32_t WorldCell::FindSector(const char *pszName)
{
    //for now just render the sectors directly.
    for (uint32_t i=0; i<(uint32_t)m_Sectors.size(); i++)
    {
        if ( stricmp(m_Sectors[i]->m_szName, pszName) == 0 )
        {
            return i;
        }
    }
    return -1;
}

void WorldCell::SetSkyTex(uint32_t uSkyIndex, TextureHandle hTex, uint32_t uWidth, uint32_t uHeight)
{
    m_aSkyTex[ uSkyIndex ] = hTex;
    m_uSkyWidth  = uWidth;
    m_uSkyHeight = uHeight;
}

void WorldCell::Render(IDriver3D *pDriver, Camera *pCamera, uint32_t uSectorTypeVis)
{
    Camera *pUpdateCam = pCamera;
    if ( m_bLockCamera )
    {
        pUpdateCam = &m_LockCamera;
    }

    if ( pCamera->GetSector() == 0xffff || pUpdateCam->GetSector() >= m_Sectors.size() )
        return;

    //prepare RenderQue
    RenderQue::Reset();

    Sector *pStartSector = m_Sectors[ pUpdateCam->GetSector() ];
    if ( pStartSector->m_uTypeFlags&SECTOR_TYPE_25D )
    {
        Sector_2_5D::RenderSectors( pDriver, this, pUpdateCam, (Sector_2_5D *)pStartSector, m_Sectors );
    }
    else
    {
        //for now just render the sectors directly, 
        //add culling.
        for (uint32_t i=0; i<(uint32_t)m_Sectors.size(); i++)
        {
            if ( !(m_Sectors[i]->m_uTypeFlags&uSectorTypeVis) )
                continue;

            //if ( pUpdateCam->GetSector() != i )
            {
                if ( pUpdateCam->AABBInsideFrustum(m_Sectors[i]->m_Bounds[0], m_Sectors[i]->m_Bounds[1], m_Sectors[i]->m_x, m_Sectors[i]->m_y) == Camera::FRUSTUM_OUT )
                {
                    continue;
                }
            }

            m_Sectors[i]->Render(pDriver, pUpdateCam);
        }

        //Execute GPU render commands.
        pDriver->EnableFog(true, 400.0f);
        RenderQue::Render();
        pDriver->EnableFog(false);
    }
}

void WorldCell::Collide(Vector3 *p0, Vector3 *p1, uint32_t& uSector, float fRadius, uint32_t uSectorTypeVis, bool bPassThruAdjoins, int32_t worldX, int32_t worldY)
{
    Sector *pStartSector = m_Sectors[ uSector ];
    if ( pStartSector->m_uTypeFlags&SECTOR_TYPE_25D )
    {
        Sector_2_5D::Collide( p0, p1, uSector, fRadius, m_Sectors, bPassThruAdjoins );
    }
    else
    {
        m_nCollisionIter = 0;
        m_vStartLoc = *p0;
        Collide_Recursive(p0, p1, uSector, fRadius, uSectorTypeVis, bPassThruAdjoins, worldX, worldY);
    }
}

void WorldCell::Collide_Recursive(Vector3 *p0, Vector3 *p1, uint32_t& uSector, float fRadius, uint32_t uSectorTypeVis, bool bPassThruAdjoins, int32_t worldX, int32_t worldY)
{
    const float veryCloseDistance = 0.00001f;
    if ( m_nCollisionIter >= MAX_COLLISION_ITER )
    {
        *p1 = *p0;
        return;
    }
    m_nCollisionIter++;

    CollisionPacket collision;
    Vector3 eRadiusInv(1.0f/4.0f, 1.0f/4.0f, 1.0f/8.9f);    //8.4

    Vector3 vel = *p1 - *p0;
    Vector3 bounds[2];
    collision.eRadius.Set(4.0f, 4.0f, 8.9f);
    collision.velocity = vel;
    collision.normalizedVelocity = vel;
    collision.normalizedVelocity.Normalize();
    collision.basePoint = *p0;
    collision.bFoundCollision = false;
    collision.bOnGround = false;
    collision.bEmbedded = false;

    vel = vel*eRadiusInv;
    collision.R3Velocity = vel;
    collision.R3VelocityNorm = collision.R3Velocity;
    collision.R3VelocityNorm.Normalize();
    collision.R3Position = collision.basePoint * eRadiusInv;

    if ( fabsf(vel.x) < 0.00001f && fabsf(vel.y) < 0.00001f && fabsf(vel.z) < 0.00001f )
    {
        *p1 = *p0;
        return;
    }
    
    Vector3 rScaled = collision.eRadius;
    bounds[0].Set( Math::Min(p0->x,p1->x) - rScaled.x, Math::Min(p0->y,p1->y) - rScaled.y, Math::Min(p0->z,p1->z) - rScaled.z );
    bounds[1].Set( Math::Max(p0->x,p1->x) + rScaled.x, Math::Max(p0->y,p1->y) + rScaled.y, Math::Max(p0->z,p1->z) + rScaled.z );

    for (uint32_t i=0; i<(uint32_t)m_Sectors.size(); i++)
    {
        if ( !(m_Sectors[i]->m_uTypeFlags&uSectorTypeVis) )
            continue;

        Vector3 vOffs((m_Sectors[i]->m_x - worldX) * 1024.0f, (m_Sectors[i]->m_y - worldY) * 1024.0f, 0.0f);
        Vector3 sBounds[2];
        sBounds[0] = m_Sectors[i]->m_Bounds[0] + vOffs;
        sBounds[1] = m_Sectors[i]->m_Bounds[1] + vOffs;

        if ( Math::AABB_Overlap3D( sBounds[0], sBounds[1], bounds[0], bounds[1] ) )
        {
            m_Sectors[i]->Collide( &collision, bounds, vOffs );
        }
    }

    if ( collision.bFoundCollision == false )
    {
        return;
    }
    else
    {
        //Handle collision.
        Vector3 destinationPoint = collision.R3Position + collision.R3Velocity;
        Vector3 newBasePoint = collision.R3Position;

        //if ( collision.nearestDist >= veryCloseDistance )
        {
            Vector3 V = collision.R3Velocity;
            V.Normalize();

            newBasePoint = collision.R3Position + V*(collision.nearestDist - veryCloseDistance);
            collision.intersectionPoint = collision.intersectionPoint - V*MIN(collision.nearestDist, veryCloseDistance);
        }

        Vector3 slidePlaneOrigin = collision.intersectionPoint;
        Vector3 slidePlaneNormal = newBasePoint - collision.intersectionPoint;
        //handle surfaces that are angled downwards... 0.2f
        if ( (slidePlaneNormal.z < 0.0f && slidePlaneNormal.z > -0.98f) || (slidePlaneNormal.z < 0.30f && slidePlaneNormal.z > 0.0f)  )
              slidePlaneNormal.z = 0;
        slidePlaneNormal.Normalize();
        float slidePlaneD = -slidePlaneNormal.Dot(slidePlaneOrigin);

        *p0 = newBasePoint * collision.eRadius;
        Vector3 newVel = destinationPoint - newBasePoint;
        Vector3 T = newVel - slidePlaneNormal*(slidePlaneNormal.Dot(newVel));
        *p1 = newBasePoint + T*0.99f;
        *p1 = (*p1) * collision.eRadius;

        /*if ( collision.bEmbedded )
        {
            *p0 = (*p0) + collision.pushVec;
            *p1 = (*p1) + collision.pushVec;
        }*/

        if ( vel.x != 0.0f || vel.y != 0.0f || p1->z >= p0->z )
        {
            Collide_Recursive(p0, p1, uSector, fRadius, uSectorTypeVis, bPassThruAdjoins, worldX, worldY);
        }
        else
        {
            *p1 = *p0;
        }
    }
}

void WorldCell::RayCastAndActivate(Vector3 *p0, Vector3 *p1, uint32_t& uSector, uint32_t uSectorTypeVis, int32_t worldX, int32_t worldY)
{
    Sector *pStartSector = m_Sectors[ uSector ];
    if ( pStartSector->m_uTypeFlags&SECTOR_TYPE_25D )
    {
        Sector_2_5D::RayCastAndActivate( p0, p1, uSector, m_Sectors );
    }
    else
    {
        RaycastPacket raycastPacket;
        raycastPacket.rayOrigin = *p0;
        raycastPacket.rayDir    = (*p1) - (*p0);
        raycastPacket.nearestDist = raycastPacket.rayDir.Normalize();
        raycastPacket.bFoundIntersection = false;
        raycastPacket.bounds[0].Set( Math::Min(p0->x, p1->x), Math::Min(p0->y, p1->y), Math::Min(p0->z, p1->z) );
        raycastPacket.bounds[1].Set( Math::Max(p0->x, p1->x), Math::Max(p0->y, p1->y), Math::Max(p0->z, p1->z) );
        raycastPacket.pCollisionObj = NULL;
        raycastPacket.pCollisionSector = NULL;

        for (uint32_t i=0; i<(uint32_t)m_Sectors.size(); i++)
        {
            if ( !(m_Sectors[i]->m_uTypeFlags&uSectorTypeVis) )
                continue;

            Vector3 vOffs((m_Sectors[i]->m_x - worldX) * 1024.0f, (m_Sectors[i]->m_y - worldY) * 1024.0f, 0.0f);
            Vector3 sBounds[2];
            sBounds[0] = m_Sectors[i]->m_Bounds[0] + vOffs;
            sBounds[1] = m_Sectors[i]->m_Bounds[1] + vOffs;

            if ( Math::AABB_Overlap3D( sBounds[0], sBounds[1], raycastPacket.bounds[0], raycastPacket.bounds[1] ) )
            {
                m_Sectors[i]->Raycast( &raycastPacket, vOffs );
            }
        }

        if ( raycastPacket.bFoundIntersection && raycastPacket.pCollisionObj )
        {
            raycastPacket.pCollisionObj->SendMessage(LMSG_ACTIVATE, 0);
        }
    }
}

bool WorldCell::Raycast(Vector3 *p0, Vector3 *p1, Vector3 *pInter, uint32_t uSectorTypeVis, int32_t worldX, int32_t worldY)
{
    Sector *pStartSector = m_Sectors[0];
    if ( pStartSector->m_uTypeFlags&SECTOR_TYPE_25D )
    {
        //Sector_2_5D::RayCastAndActivate( p0, p1, uSector, m_Sectors );
    }
    else
    {
        RaycastPacket raycastPacket;
        raycastPacket.rayOrigin = *p0;
        raycastPacket.rayDir    = (*p1) - (*p0);
        raycastPacket.nearestDist = raycastPacket.rayDir.Normalize();
        raycastPacket.bFoundIntersection = false;
        raycastPacket.bounds[0].Set( Math::Min(p0->x, p1->x), Math::Min(p0->y, p1->y), Math::Min(p0->z, p1->z) );
        raycastPacket.bounds[1].Set( Math::Max(p0->x, p1->x), Math::Max(p0->y, p1->y), Math::Max(p0->z, p1->z) );
        raycastPacket.pCollisionObj = NULL;
        raycastPacket.pCollisionSector = NULL;

        for (uint32_t i=0; i<(uint32_t)m_Sectors.size(); i++)
        {
            if ( !(m_Sectors[i]->m_uTypeFlags&uSectorTypeVis) )
                continue;

            Vector3 vOffs((m_Sectors[i]->m_x - worldX) * 1024.0f, (m_Sectors[i]->m_y - worldY) * 1024.0f, 0.0f);
            Vector3 sBounds[2];
            sBounds[0] = m_Sectors[i]->m_Bounds[0] + vOffs;
            sBounds[1] = m_Sectors[i]->m_Bounds[1] + vOffs;

            if ( Math::AABB_Overlap3D( sBounds[0], sBounds[1], raycastPacket.bounds[0], raycastPacket.bounds[1] ) )
            {
                m_Sectors[i]->Raycast( &raycastPacket, vOffs );
            }
        }

        if ( raycastPacket.bFoundIntersection )
        {
            *pInter = raycastPacket.intersectionPoint;
        }

        return raycastPacket.bFoundIntersection;
    }
    return false;
}

void WorldCell::Update(Camera *pCamera, float dt, uint32_t uSectorTypeVis)
{
    Camera *pUpdateCam = pCamera;
    if ( m_bLockCamera )
    {
        pUpdateCam = &m_LockCamera;
    }

    Sector *pStartSector = m_Sectors[ pUpdateCam->GetSector() ];
    if ( pStartSector->m_uTypeFlags&SECTOR_TYPE_25D )
    {
    }
    else
    {
        //for now just render the sectors directly, 
        //add culling.
        for (uint32_t i=0; i<(uint32_t)m_Sectors.size(); i++)
        {
            if ( !(m_Sectors[i]->m_uTypeFlags&uSectorTypeVis) )
                continue;
            
            m_Sectors[i]->Update(dt);
        }
    }
}

void WorldCell::LockCamera(Camera *pCamera, bool bLock)
{
    if ( bLock )
    {
        m_LockCamera = *pCamera;
    }
    m_bLockCamera = bLock;
}

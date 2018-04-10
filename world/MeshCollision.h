#ifndef MESHCOLLISION_H
#define MESHCOLLISION_H

#include "../CommonTypes.h"
#include "../world/CollisionComponent.h"
#include "../math/Vector3.h"
#include "../math/Plane.h"

class VertexBuffer;
class IndexBuffer;

class MeshCollision : public CollisionComponent
{
public:
    MeshCollision();
    virtual ~MeshCollision();

    virtual bool Collide(CollisionPacket *packet, Matrix *pWorldMtx, const Vector3& vOffset) override;
    virtual bool Raycast(RaycastPacket *packet, Matrix *pWorldMtx, Object *parent, Sector *pSector, const Vector3& vOffset) override;
    void TransformUpdateRequired() { m_pWorldMtx = 0; }
    void AddPolygon(int numVerts, Vector3 *pvVertices );
    void SetMaxPolygonCount(int maxPolyCount);

private:
    struct CollisionPolygon
    {
        int numVerts;
        Vector3 *pVertexLS; //local space vertices.
        Vector3 *pVertexES; //ellipsoid space vertices.
        Plane plane;
        bool bValid;        //true if this polygon should be tested.
    };
    int m_nPolygonCount;
    CollisionPolygon *m_Polygons;
    Vector3 m_Bounds[2];
    Matrix *m_pWorldMtx;
    Vector3 m_eRadius;
    Vector3 m_vOffset;

    void Transform();
    bool CheckTriangle(CollisionPacket *col, const Vector3& p1, const Vector3& p2, const Vector3& p3, const Plane& plane);
    bool RayTriangle(RaycastPacket *col, const Vector3& p1, const Vector3& p2, const Vector3& p3, const Plane& plane);
};

#endif //MESHCOLLISION_H
#ifndef COLLISIONCOMPONENT_H
#define COLLISIONCOMPONENT_H

#include "../CommonTypes.h"
#include "../math/Vector3.h"
#include "../math/Matrix.h"

class Object;

class Sector;

struct CollisionPacket {
    Vector3 eRadius;    //ellipsoid radius

    Vector3 R3Velocity;
    Vector3 R3VelocityNorm;
    Vector3 R3Position;

    Vector3 velocity;
    Vector3 normalizedVelocity;
    Vector3 basePoint;

    Vector3 intNormal;

    bool bFoundCollision;
    bool bOnGround;
    bool bEmbedded;
    float nearestDist;
    Vector3 intersectionPoint;
    Vector3 pushVec;
};

struct RaycastPacket {
    Vector3 rayOrigin;
    Vector3 rayDir;

    bool bFoundIntersection;
    float nearestDist;
    Vector3 intersectionPoint;
    Object *pCollisionObj;
    Sector *pCollisionSector;

    Vector3 bounds[2];
};

class CollisionComponent {
public:
    CollisionComponent() {};

    virtual ~CollisionComponent() {};

    virtual bool Collide(CollisionPacket *packet, Matrix *pWorldMtx, const Vector3 &vOffset) { return false; }

    virtual bool Raycast(RaycastPacket *packet, Matrix *pWorldMtx, Object *parent, Sector *pSector,
                         const Vector3 &vOffset) { return false; }
};

#endif //COLLISIONCOMPONENT_H

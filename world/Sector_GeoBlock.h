#ifndef SECTOR_GEOBLOCK
#define SECTOR_GEOBLOCK

#include "Sector.h"
#include "../math/Vector2.h"
#include "../math/Vector3.h"
#include "../render/Camera.h"
#include "../world/LevelFunc.h"

/*********************************************
Geometry block:
A set of 3D models and surfaces.
 *********************************************/

class IDriver3D;
class Object;
class WorldCell;

class Sector_GeoBlock : public Sector
{
public:
    Sector_GeoBlock() = default;
    virtual ~Sector_GeoBlock();

    void Render(IDriver3D *pDriver, Camera *pCamera) override;
    void Collide(CollisionPacket *packet, Vector3 *bounds, const Vector3& vOffset) override;
    void Raycast(RaycastPacket *packet, const Vector3& vOffset) override;
    void Update(float dt) override;
public:
};

#endif //SECTOR_GEOBLOCK

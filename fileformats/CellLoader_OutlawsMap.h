#ifndef CELLLOADER_OUTLAWSMAP_H
#define CELLLOADER_OUTLAWSMAP_H

#include "../CommonTypes.h"
#include "CellLoader.h"

class WorldCell;
class World;

class CellLoader_OutlawsMap : public CellLoader
{
public:
    CellLoader_OutlawsMap() = default;
    virtual ~CellLoader_OutlawsMap();

    WorldCell *Load( IDriver3D *pDriver, World *pWorld, uint8_t *pData, uint32_t uLen, const std::string& sFile, int32_t worldX, int32_t worldY ) override;

private:

private:
};

#endif //CELLLOADER_OUTLAWSMAP_H
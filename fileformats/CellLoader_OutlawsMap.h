#ifndef CELLLOADER_OUTLAWSMAP_H
#define CELLLOADER_OUTLAWSMAP_H

#include "../CommonTypes.h"
#include "CellLoader.h"

class WorldCell;
class World;

class CellLoader_OutlawsMap : public CellLoader
{
public:
	CellLoader_OutlawsMap();
	~CellLoader_OutlawsMap();

	WorldCell *Load( IDriver3D *pDriver, World *pWorld, uint8_t *pData, uint32_t uLen, const string& sFile, int32_t worldX, int32_t worldY );

private:

private:
};

#endif //CELLLOADER_OUTLAWSMAP_H
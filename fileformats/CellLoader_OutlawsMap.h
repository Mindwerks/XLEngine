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

	WorldCell *Load( IDriver3D *pDriver, World *pWorld, u8 *pData, u32 uLen, const string& sFile, s32 worldX, s32 worldY );

private:

private:
};

#endif //CELLLOADER_OUTLAWSMAP_H
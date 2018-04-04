#ifndef CELLLOADER_DAGGERFALL_H
#define CELLLOADER_DAGGERFALL_H

#include "../CommonTypes.h"
#include "../math/Vector3.h"
#include "MeshLoader_Daggerfall.h"
#include "CellLoader.h"

class WorldCell;
class Sector;

class CellLoader_Daggerfall : public CellLoader
{
public:
	CellLoader_Daggerfall();
	~CellLoader_Daggerfall();

	WorldCell *Load(IDriver3D *pDriver, World *pWorld, u8 *pData, u32 uLen, const string& sFile, s32 worldX, s32 worldY);
	WorldCell *LoadFromLocation(IDriver3D *pDriver, World *pWorld, void *pLocPtr);

	//returns true if this CellLoader handles file handling itself.
	bool UsesOwnFiles() { return true; }

private:
	Sector *LoadBlock(IDriver3D *pDriver, u32 uLength, int& index, char *pData, const Vector3& vBlockLoc, Vector3& vStartTagLoc, bool bStartBlock, s32 worldX, s32 worldY, int blockType);
	Sector *LoadBlock_Ext(IDriver3D *pDriver, u32 uLength, char *pData, s32 nClimate, float fTileHeight, s32 worldX, s32 worldY, u8 *pTexData);

private:
	MeshLoader_Daggerfall m_MeshLoader;
};

#endif //CELLLOADER_DAGGERFALL_H
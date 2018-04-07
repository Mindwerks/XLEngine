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

	WorldCell *Load(IDriver3D *pDriver, World *pWorld, uint8_t *pData, uint32_t uLen, const string& sFile, int32_t worldX, int32_t worldY);
	WorldCell *LoadFromLocation(IDriver3D *pDriver, World *pWorld, void *pLocPtr);

	//returns true if this CellLoader handles file handling itself.
	bool UsesOwnFiles() { return true; }

private:
	Sector *LoadBlock(IDriver3D *pDriver, uint32_t uLength, int& index, char *pData, const Vector3& vBlockLoc, Vector3& vStartTagLoc, bool bStartBlock, int32_t worldX, int32_t worldY, int blockType);
	Sector *LoadBlock_Ext(IDriver3D *pDriver, uint32_t uLength, char *pData, int32_t nClimate, float fTileHeight, int32_t worldX, int32_t worldY, uint8_t *pTexData);

private:
	MeshLoader_Daggerfall m_MeshLoader;
};

#endif //CELLLOADER_DAGGERFALL_H
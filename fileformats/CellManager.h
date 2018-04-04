#ifndef CELLMANAGER_H
#define CELLMANAGER_H

#include "../CommonTypes.h"
#include "CellTypes.h"
#include <string>

using namespace std;
class WorldCell;
class CellLoader;
class Archive;
class IDriver3D;
class World;

class CellManager
{
public:
	static void Init();
	static void Destroy();

	//This loads a cell from disk and then formats it for the engine.
	//The WorldCell is then returned, ready to be added to the world.
	//Cell types include: levels, dungeon tiles, location tiles, etc.
	static WorldCell *LoadCell(IDriver3D *pDriver, World *pWorld, u32 uCellType, Archive *pCellArchive, const string& sFile, s32 worldX, s32 worldY);
	static WorldCell *LoadFromLocation(IDriver3D *pDriver, World *pWorld, u32 uCellType, void *pLocPtr);
	
private:
	static CellLoader *m_CellLoaders[];
};

#endif //CELLMANAGER_H
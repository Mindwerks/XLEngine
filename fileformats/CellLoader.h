#ifndef CELLLOADER_H
#define CELLLOADER_H

#include "../CommonTypes.h"
#include <string>
class WorldCell;
class IDriver3D;
class World;

using namespace std;

class CellLoader
{
public:
	CellLoader() {};
	virtual ~CellLoader() {};

	//returns true if this CellLoader handles file handling itself.
	virtual bool UsesOwnFiles() { return false; }
	//load (normal)
	virtual WorldCell *Load( IDriver3D *pDriver, World *pWorld, u8 *pData, u32 uLen, const string& sFile, s32 worldX, s32 worldY ) {return 0;}
	virtual WorldCell *LoadFromLocation(IDriver3D *pDriver, World *pWorld, void *pLocPtr) {return 0;}
};

#endif //CELLLOADER_H
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
	virtual WorldCell *Load( IDriver3D *pDriver, World *pWorld, uint8_t *pData, uint32_t uLen, const string& sFile, int32_t worldX, int32_t worldY ) {return 0;}
	virtual WorldCell *LoadFromLocation(IDriver3D *pDriver, World *pWorld, void *pLocPtr) {return 0;}
};

#endif //CELLLOADER_H
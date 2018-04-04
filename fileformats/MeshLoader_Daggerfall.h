#ifndef MESHLOADER_DAGGERFALL_H
#define MESHLOADER_DAGGERFALL_H

#include "../CommonTypes.h"
#include "../render/Mesh.h"
#include "../world/MeshCollision.h"
#include <string>
#include <vector>
#include <map>

using namespace std;
class IDriver3D;

class MeshLoader_Daggerfall
{
public:
	MeshLoader_Daggerfall();
	~MeshLoader_Daggerfall();

	bool Load(IDriver3D *pDriver, Mesh *pMesh, MeshCollision *pMeshCol, char *ID, int region, int type);

	static void BuildTextureName(char *pszTexName, int FileIndex);

private:
	bool LoadMesh(IDriver3D *pDriver, Mesh *pMesh, MeshCollision *pMeshCol, char *pData, u32 uLength, int region, int type);
};

#endif //MESHLOADER_DAGGERFALL_H
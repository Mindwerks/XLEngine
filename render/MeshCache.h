#ifndef MESHCACHE_H
#define MESHCACHE_H

#include "../CommonTypes.h"

#include <string>
#include <map>

using namespace std;

class Mesh;
class MeshCollision;

class MeshCache
{
	typedef map<string, Mesh *> MeshMap;
	typedef map<string, MeshCollision *> MeshCollisionMap;

public:
	static bool Init();
	static void Destroy();

	static Mesh *GetMesh(const string& sName);
	static MeshCollision *GetMeshCollision(const string& sName);

private:
	static MeshMap m_MeshMap;
	static MeshCollisionMap m_MeshCollisionMap;
};

#endif //MESHCACHE_H

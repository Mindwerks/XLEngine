#ifndef MESHCACHE_H
#define MESHCACHE_H

#include "../CommonTypes.h"

#include <string>
#include <map>

class Mesh;
class MeshCollision;

class MeshCache
{
    typedef std::map<std::string, Mesh *> MeshMap;
    typedef std::map<std::string, MeshCollision *> MeshCollisionMap;

public:
    static bool Init();
    static void Destroy();

    static Mesh *GetMesh(const std::string& sName);
    static MeshCollision *GetMeshCollision(const std::string& sName);

private:
    static MeshMap m_MeshMap;
    static MeshCollisionMap m_MeshCollisionMap;
};

#endif //MESHCACHE_H

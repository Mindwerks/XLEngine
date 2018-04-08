#include "MeshCache.h"
#include "Mesh.h"
#include "../world/MeshCollision.h"

MeshCache::MeshMap MeshCache::m_MeshMap;
MeshCache::MeshCollisionMap MeshCache::m_MeshCollisionMap;

bool MeshCache::Init() {
    return true;
}

void MeshCache::Destroy() {
    MeshMap::iterator iMesh = m_MeshMap.begin();
    MeshMap::iterator eMesh = m_MeshMap.end();
    for (; iMesh != eMesh; ++iMesh)
    {
        delete iMesh->second;
    }
    m_MeshMap.clear();

    MeshCollisionMap::iterator iMeshCol = m_MeshCollisionMap.begin();
    MeshCollisionMap::iterator eMeshCol = m_MeshCollisionMap.end();
    for (; iMeshCol != eMeshCol; ++iMeshCol)
    {
        delete iMeshCol->second;
    }
    m_MeshCollisionMap.clear();
}

Mesh *MeshCache::GetMesh(const string &sName) {
    Mesh *pMesh = NULL;
    MeshMap::iterator iMesh = m_MeshMap.find(sName);
    if (iMesh != m_MeshMap.end())
    {
        pMesh = iMesh->second;
    }
    else
    {
        pMesh = new Mesh();
        m_MeshMap[sName] = pMesh;
    }

    return pMesh;
}

MeshCollision *MeshCache::GetMeshCollision(const string &sName) {
    MeshCollision *pMeshCollision = NULL;
    MeshCollisionMap::iterator iMeshCol = m_MeshCollisionMap.find(sName);
    if (iMeshCol != m_MeshCollisionMap.end())
    {
        pMeshCollision = iMeshCol->second;
    }
    else
    {
        pMeshCollision = new MeshCollision();
        m_MeshCollisionMap[sName] = pMeshCollision;
    }

    return pMeshCollision;
}

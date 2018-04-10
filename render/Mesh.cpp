#include "Mesh.h"
#include "IDriver3D.h"
#include "RenderQue.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "../world/Object.h"
#include <cassert>
#include <cstdlib>
#include <memory.h>

Mesh::Mesh()
{
    m_pVB = nullptr;
    m_pIB = nullptr;
    m_pMaterials = nullptr;
    m_nMtlCnt = 0;
    m_bLoaded = false;
}

Mesh::~Mesh()
{
    if (m_pVB)
    {
        delete m_pVB;
    }
    if (m_pIB)
    {
        delete m_pIB;
    }
    SafeDeleteArr(m_pMaterials);
}

void Mesh::Render(Object *pObj, IDriver3D *pDriver, float fIntensity, const Vector3& vOffset)
{
    for (int m=0; m<m_nMtlCnt; m++)
    {
        MaterialEntry *pEntry = RenderQue::GetEntry(RBUCKET_OPAQUE);
        if ( pEntry )
        {
            pEntry->hTex        = m_pMaterials[m].hTex;
            pEntry->startIndex  = m_pMaterials[m].uIndexOffset;
            pEntry->primCount   = m_pMaterials[m].uPrimCount;
            pEntry->pVB         = m_pVB;
            pEntry->pIB         = m_pIB;
            pEntry->mWorld      = pObj->GetMatrixPtr();
            pEntry->worldX      = pObj->GetWorldX();
            pEntry->worldY      = pObj->GetWorldY();
        }
    }
}

void Mesh::GetBounds(Vector3& vMin, Vector3& vMax)
{
    vMin = m_Bounds[0];
    vMax = m_Bounds[1];
}
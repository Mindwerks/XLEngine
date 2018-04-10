#ifndef MESH_H
#define MESH_H

#include "../CommonTypes.h"
#include "../world/RenderComponent.h"

class VertexBuffer;
class IndexBuffer;

class Mesh : public RenderComponent
{
public:
    Mesh();
    virtual ~Mesh();

    virtual void Render(Object *pObj, IDriver3D *pDriver, float fIntensity, const Vector3& vOffset) override;
    virtual void GetBounds(Vector3& vMin, Vector3& vMax) override;

    bool IsLoaded() { return m_bLoaded; }
    void SetLoaded() { m_bLoaded = true; }

//private:
    struct Material
    {
        TextureHandle hTex;
        uint32_t uIndexOffset;
        uint32_t uPrimCount;

        Vector3 vBounds[2];
    };
    VertexBuffer *m_pVB;
    IndexBuffer  *m_pIB;
    int           m_nMtlCnt;
    Material     *m_pMaterials;
    
    Vector3       m_Bounds[2];
    bool          m_bLoaded;
};

#endif //MESH_H
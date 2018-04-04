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
	~Mesh();

	void Render(Object *pObj, IDriver3D *pDriver, f32 fIntensity, const Vector3& vOffset);
	void GetBounds(Vector3& vMin, Vector3& vMax);

	bool IsLoaded() { return m_bLoaded; }
	void SetLoaded() { m_bLoaded = true; }

//private:
	struct Material
	{
		TextureHandle hTex;
		u32 uIndexOffset;
		u32 uPrimCount;

		Vector3 vBounds[2];
	};
	VertexBuffer *m_pVB;
	IndexBuffer  *m_pIB;
	int			  m_nMtlCnt;
	Material	 *m_pMaterials;
	
	Vector3		  m_Bounds[2];
	bool		  m_bLoaded;
};

#endif //MESH_H
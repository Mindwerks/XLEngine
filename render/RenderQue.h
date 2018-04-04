#ifndef RENDERQUE_H
#define RENDERQUE_H

#include "../CommonTypes.h"
#include "../math/Vector2.h"
#include "../math/Vector3.h"
#include "../math/Vector4.h"
#include "../math/Matrix.h"
#include <vector>

class IDriver3D;
class VertexBuffer;
class IndexBuffer;
class LightObject;

using namespace std;

struct RenderQuad
{
	TextureHandle hTex;
	Vector3 posList[4];
	Vector2 uvList[4];
	Vector4 color;
	bool bForceZWrite;
	bool bApplyLighting;

	//lights.
	int nLightCnt;
	const LightObject **pLightList;
};

struct MaterialEntry
{
	TextureHandle hTex;
	Matrix *mWorld;
	VertexBuffer *pVB;
	IndexBuffer  *pIB;
	u16 startIndex;
	u16 primCount;
	s32 worldX;
	s32 worldY;

	//lights.
	int nLightCnt;
	const LightObject **pLightList;
};

enum RenderBuckets
{
	RBUCKET_OPAQUE=0,
	RBUCKET_ALPHA,
	RBUCKET_COUNT
};

class RenderQue
{
public:
	static bool Init(IDriver3D *pDriver);
	static void Destroy();
	
	static void Reset();

	//Quad rendering.
	static RenderQuad *GetRenderQuad();
	static void AddQuad(TextureHandle hTex, Vector3 *posList, Vector2 *uvList, const Vector4& color, bool bForceZWrite=false);
	//Material rendering.
	static MaterialEntry *GetEntry(RenderBuckets bucket);
	//Lights.
	static void SetLightData( int nLightCnt, const LightObject **pLightList ) { m_nCurLightCnt = nLightCnt; m_pCurLightList = pLightList; }

	static void Render();

private:
	static IDriver3D *m_pDriver;

	static RenderQuad *m_pQuads;
	static u32 m_uQuadCnt;

	static vector<MaterialEntry *> m_apRenderBuckets[RBUCKET_COUNT];
	static vector<MaterialEntry>   m_RenderEntryPool;
	static u32 m_uRenderEntryLoc;

	static int m_nCurLightCnt;
	static const LightObject **m_pCurLightList;

	static bool SortCB_Opaque(MaterialEntry*& d1, MaterialEntry*& d2);
};

#endif //RENDERQUE_H

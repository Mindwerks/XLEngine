#ifndef DRIVER3D_SOFT_H
#define DRIVER3D_SOFT_H

#include "IDriver3D.h"
#include "Driver3D_IPlatform.h"
#include "../math/Matrix.h"
#include "../math/Vector2.h"
#include "../math/Vector4.h"
#include <assert.h>
#include <vector>

class IndexBuffer;
struct Texture;
struct VFmt_Pos_UV_Screen;
struct VBO;
struct IBO;
struct TriGradients;
struct TriEdge;
class Camera;

using namespace std;

struct Texture
{
	s32 m_nWidth;
	s32 m_nHeight;
	s32 m_nMipCnt;
	bool m_bIsPow2;
	s32 m_nFrameCnt;

	u32 *m_pData[32];
};

//optional triangle data.
//if sent with the index buffer, this is used to speed up triangle rendering.
struct PolygonData
{
	Vector3 nrmlWS;
	Vector3 cenWS;
	float   radius2_WS;
};

class Driver3D_Soft : public IDriver3D
{
    public:
        Driver3D_Soft();
        virtual ~Driver3D_Soft();

        bool Init(s32 w, s32 h);
        void Present();
		void Clear(bool bClearColor=true);

		void SetWorldMatrix(Matrix *pMtx, s32 worldX, s32 worldY);
		void SetViewMatrix(Matrix *pMtx, Vector3 *pLoc, Vector3 *pDir);
		void SetProjMtx(Matrix *pMtx);
		void SetCamera(Camera *pCamera);
		
        void ChangeWindowSize(s32 w, s32 h);

		//Texture Functions.
		void SetTexture(s32 slot, TextureHandle hTex, u32 uFilter=FILTER_NORMAL, bool bWrap=true, s32 frame=-1);
		void SetColor(Vector4 *pColor=0);
		TextureHandle CreateTexture(u32 uWidth, u32 uHeight, u32 uFormat=TEX_FORMAT_RGBA8, u8 *pData=0, bool bGenMips=false, s32 nFrameCnt=1);
		void FillTexture(TextureHandle hTex, u8 *pData, u32 uWidth, u32 uHeight, bool bGenMips=false);
		void FreeTexture(TextureHandle hTex);

		//VBO/IBO Support.
		u32 CreateVBO();
		void AllocVBO_Mem(u32 uID, u32 uVtxCnt, u32 uSize, bool bDynamic);
		void FillVBO(u32 uID, void *pData, u32 uSize, bool bDynamic);
		void SetVBO(u32 uID, u32 uStride, u32 uVBO_Flags);
		u32 CreateIB();
		void FillIB(u32 uID, void *pData, u32 uSize, bool bDynamic);
		void ResetIBFlags(u32 uID);
		void DeleteBuffer(u32 uID);
		void ClearDrawData();

		//Draw!
		void RenderIndexedTriangles(IndexBuffer *pIB, s32 nTriCnt, s32 startIndex=0);
		void RenderScreenQuad(const Vector4& posScale, const Vector2& uvTop, const Vector2& uvBot, const Vector4& colorTop, const Vector4& colorBot);
		void RenderWorldQuad(const Vector3& pos0, const Vector3& pos1, const Vector2& uv0, const Vector2& uv1, const Vector4& color);
		void RenderWorldQuad(const Vector3 *posList, const Vector2 *uvList, const Vector4& color, bool bRecieveLighting=false);
		void RenderWorldQuad(const Vector3 *posList, const Vector2 *uvList, const Vector4 *color, bool bRecieveLighting=false);

		//Render States
		void EnableDepthWrite(bool bEnable);
		void EnableDepthRead(bool bEnable);
		void EnableStencilWriting(bool bEnable, u32 uValue);
		void EnableStencilTesting(bool bEnable);
		void EnableCulling(bool bEnable);
		void EnableAlphaTest(bool bEnable, u8 uAlphaCutoff=128);
		void SetBlendMode(u32 uMode=BLEND_NONE);
		void EnableFog(bool bEnable, f32 fEnd=10000.0f);
		void SetFogDensity(f32 fDensity=1.0f);

		//Sorting
		bool ApplyOpaqueSort() { return false; }
		bool ApplyTransSort()  { return false; }

		//Software Rendering specific.
		void SetBitDepth(s32 bitDepth);
		s32 GetBitDepth() { return m_nBitDepth; }
		int GetAffineLength();
		int GetFrameWidth();
		int GetFrameHeight();
		bool GetBilinear() { return m_bBilinear; }
		bool GetGouraud()  { return m_bGouraud; }
		Camera *GetCamera() { return m_pRenderCamera; }
		void SetClearColorFromTex(TextureHandle hTex);
		//optional polygon data to accelerate triangle processing.
		static u8 GetColormapID() { return s_uColormapID; }
		static Texture *GetCurTex() { return m_pCurTex; }

		//Driver extensions
		void SetExtension_Data(u32 uExtension, void *pData0, void *pData1);

    protected:

		void GenerateMips(u32 uWidth, u32 uHeight, u8 *pData);

    private:
		vector<Texture *> m_Textures;
		vector<VBO *> m_VBO;
		vector<IBO *> m_IBO;
		s32 m_nBitDepth;
		Matrix m_ProjMtx;
		Matrix m_ViewProj;
		Camera *m_pRenderCamera;
		static Texture *m_pCurTex;
		static u32 s_uColormapID;

		VBO *m_pCurVBO;
		float *m_PosStream;
		float *m_TCoordStream;
		float *m_NrmlStream;
		u32 m_uVBO_Stride;
		bool m_bBilinear;
		bool m_bGouraud;

		u32 m_uBlendMode;
		bool m_bAlphaTest;
		float m_fTimer;

		u32 m_uDbg_AllocSize;
		TextureHandle *m_pTexArray;
		u16 *m_pTexIndex;
		PolygonData *m_pCurPolygonData;

		Texture *CreateCheckPattern();
		
		void WireframeTriangle(VFmt_Pos_UV_Screen *pScrVert);
		void LocalToWorld(VBO *pVBO);
		void WorldToClip(VBO *pVBO);
		void ConvertFrameBufferTo32bpp(u32 *pal);
		void BuildTransTable();
		void BuildColorTables_32bpp(int refPalIndex=112);

		void RenderScreenQuad_8bpp(const Vector4& posScale, const Vector2& uvTop, const Vector2& uvBot, const Vector4& colorTop, const Vector4& colorBot);
		void RenderOverlays();
};

#endif // DRIVER3D_SOFT_H

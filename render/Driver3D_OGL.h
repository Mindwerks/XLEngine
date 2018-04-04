#ifndef DRIVER3D_OGL_H
#define DRIVER3D_OGL_H

#include "IDriver3D.h"
#include "Driver3D_IPlatform.h"
#include "../math/Matrix.h"
#include "../math/Vector2.h"
#include "../math/Vector4.h"
#include <assert.h>

class IndexBuffer;

class Driver3D_OGL : public IDriver3D
{
	public:

    public:
        Driver3D_OGL();
        virtual ~Driver3D_OGL();

        bool Init(s32 w, s32 h);
        void Present();
        void Clear(bool bClearColor=true);

		void SetWorldMatrix(Matrix *pMtx, s32 worldX, s32 worldY);
		void SetViewMatrix(Matrix *pMtx, Vector3 *pLoc, Vector3 *pDir);
		void SetProjMtx(Matrix *pMtx);

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
		bool ApplyOpaqueSort() { return true; }
		bool ApplyTransSort()  { return true; }

    protected:

		void GenerateMips(u32 uWidth, u32 uHeight, u8 *pData);

    private:
		u32 m_Textures[16384];
		u32 m_uTextureCnt;
};

#endif // DRIVER3D_OGL_H

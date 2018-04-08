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

        bool Init(int32_t w, int32_t h);
        void Present();
        void Clear(bool bClearColor=true);

        void SetWorldMatrix(Matrix *pMtx, int32_t worldX, int32_t worldY);
        void SetViewMatrix(Matrix *pMtx, Vector3 *pLoc, Vector3 *pDir);
        void SetProjMtx(Matrix *pMtx);
        void SetCamera(Camera *pCamera);

        void ChangeWindowSize(int32_t w, int32_t h);

        //Texture Functions.
        void SetTexture(int32_t slot, TextureHandle hTex, uint32_t uFilter=FILTER_NORMAL, bool bWrap=true, int32_t frame=-1);
        void SetColor(Vector4 *pColor=0);
        TextureHandle CreateTexture(uint32_t uWidth, uint32_t uHeight, uint32_t uFormat=TEX_FORMAT_RGBA8, uint8_t *pData=0, bool bGenMips=false, int32_t nFrameCnt=1);
        void FillTexture(TextureHandle hTex, uint8_t *pData, uint32_t uWidth, uint32_t uHeight, bool bGenMips=false);
        void FreeTexture(TextureHandle hTex);

        //VBO/IBO Support.
        uint32_t CreateVBO();
        void AllocVBO_Mem(uint32_t uID, uint32_t uVtxCnt, uint32_t uSize, bool bDynamic);
        void FillVBO(uint32_t uID, void *pData, uint32_t uSize, bool bDynamic);
        void SetVBO(uint32_t uID, uint32_t uStride, uint32_t uVBO_Flags);
        uint32_t CreateIB();
        void FillIB(uint32_t uID, void *pData, uint32_t uSize, bool bDynamic);
        void DeleteBuffer(uint32_t uID);
        void ClearDrawData();

        //Draw!
        void RenderIndexedTriangles(IndexBuffer *pIB, int32_t nTriCnt, int32_t startIndex=0);
        void RenderScreenQuad(const Vector4& posScale, const Vector2& uvTop, const Vector2& uvBot, const Vector4& colorTop, const Vector4& colorBot);
        void RenderWorldQuad(const Vector3& pos0, const Vector3& pos1, const Vector2& uv0, const Vector2& uv1, const Vector4& color);
        void RenderWorldQuad(const Vector3 *posList, const Vector2 *uvList, const Vector4& color, bool bRecieveLighting=false);
        void RenderWorldQuad(const Vector3 *posList, const Vector2 *uvList, const Vector4 *color, bool bRecieveLighting=false);

        //Render States
        void EnableDepthWrite(bool bEnable);
        void EnableDepthRead(bool bEnable);
        void EnableStencilWriting(bool bEnable, uint32_t uValue);
        void EnableStencilTesting(bool bEnable);
        void EnableCulling(bool bEnable);
        void EnableAlphaTest(bool bEnable, uint8_t uAlphaCutoff=128);
        void SetBlendMode(uint32_t uMode=BLEND_NONE);
        void EnableFog(bool bEnable, float fEnd=10000.0f);
        void SetFogDensity(float fDensity=1.0f);

        //Sorting
        bool ApplyOpaqueSort() { return true; }
        bool ApplyTransSort()  { return true; }

        Camera *GetCamera() { return m_pRenderCamera; }

    protected:

        void GenerateMips(uint32_t uWidth, uint32_t uHeight, uint8_t *pData);

    private:
        uint32_t m_Textures[16384];
        uint32_t m_uTextureCnt;

        Camera *m_pRenderCamera;
};

#endif // DRIVER3D_OGL_H

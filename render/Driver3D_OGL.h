#ifndef DRIVER3D_OGL_H
#define DRIVER3D_OGL_H

#include "IDriver3D.h"
#include "Driver3D_IPlatform.h"
#include "../math/Matrix.h"
#include "../math/Vector2.h"
#include "../math/Vector4.h"
#include <cassert>

class IndexBuffer;

class Driver3D_OGL : public IDriver3D
{
    public:

    public:
        Driver3D_OGL();
        virtual ~Driver3D_OGL();

        virtual bool Init(int32_t w, int32_t h) override;
        virtual void Present() override;
        virtual void Clear(bool bClearColor=true) override;

        virtual void SetWorldMatrix(Matrix *pMtx, int32_t worldX, int32_t worldY) override;
        virtual void SetViewMatrix(Matrix *pMtx, Vector3 *pLoc, Vector3 *pDir) override;
        virtual void SetProjMtx(Matrix *pMtx) override;
        virtual void SetCamera(Camera *pCamera) override;

        virtual void ChangeWindowSize(int32_t w, int32_t h) override;

        //Texture Functions.
        virtual void SetTexture(int32_t slot, TextureHandle hTex, uint32_t uFilter=FILTER_NORMAL, bool bWrap=true, int32_t frame=-1) override;
        virtual void SetColor(Vector4 *pColor=0) override;
        virtual TextureHandle CreateTexture(uint32_t uWidth, uint32_t uHeight, uint32_t uFormat=TEX_FORMAT_RGBA8, uint8_t *pData=0, bool bGenMips=false, int32_t nFrameCnt=1) override;
        virtual void FillTexture(TextureHandle hTex, uint8_t *pData, uint32_t uWidth, uint32_t uHeight, bool bGenMips=false) override;
        virtual void FreeTexture(TextureHandle hTex) override;

        //VBO/IBO Support.
        virtual uint32_t CreateVBO() override;
        virtual void AllocVBO_Mem(uint32_t uID, uint32_t uVtxCnt, uint32_t uSize, bool bDynamic) override;
        virtual void FillVBO(uint32_t uID, void *pData, uint32_t uSize, bool bDynamic) override;
        virtual void SetVBO(uint32_t uID, uint32_t uStride, uint32_t uVBO_Flags) override;
        virtual uint32_t CreateIB() override;
        virtual void FillIB(uint32_t uID, void *pData, uint32_t uSize, bool bDynamic) override;
        virtual void DeleteBuffer(uint32_t uID) override;
        virtual void ClearDrawData() override;

        //Draw!
        virtual void RenderIndexedTriangles(IndexBuffer *pIB, int32_t nTriCnt, int32_t startIndex=0) override;
        virtual void RenderScreenQuad(const Vector4& posScale, const Vector2& uvTop, const Vector2& uvBot, const Vector4& colorTop, const Vector4& colorBot) override;
        virtual void RenderWorldQuad(const Vector3& pos0, const Vector3& pos1, const Vector2& uv0, const Vector2& uv1, const Vector4& color) override;
        virtual void RenderWorldQuad(const Vector3 *posList, const Vector2 *uvList, const Vector4& color, bool bRecieveLighting=false) override;
        virtual void RenderWorldQuad(const Vector3 *posList, const Vector2 *uvList, const Vector4 *color, bool bRecieveLighting=false) override;

        //Render States
        virtual void EnableDepthWrite(bool bEnable) override;
        virtual void EnableDepthRead(bool bEnable) override;
        virtual void EnableStencilWriting(bool bEnable, uint32_t uValue) override;
        virtual void EnableStencilTesting(bool bEnable) override;
        virtual void EnableCulling(bool bEnable) override;
        virtual void EnableAlphaTest(bool bEnable, uint8_t uAlphaCutoff=128) override;
        virtual void SetBlendMode(uint32_t uMode=BLEND_NONE) override;
        virtual void EnableFog(bool bEnable, float fEnd=10000.0f) override;
        virtual void SetFogDensity(float fDensity=1.0f) override;

        //Sorting
        virtual bool ApplyOpaqueSort() override { return true; }
        virtual bool ApplyTransSort() override { return true; }

        virtual Camera *GetCamera() override { return m_pRenderCamera; }

    protected:

        void GenerateMips(uint32_t uWidth, uint32_t uHeight, uint8_t *pData);

    private:
        uint32_t m_Textures[16384];
        uint32_t m_uTextureCnt;

        Camera *m_pRenderCamera;
};

#endif // DRIVER3D_OGL_H

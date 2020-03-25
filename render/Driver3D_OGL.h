#ifndef DRIVER3D_OGL_H
#define DRIVER3D_OGL_H

#include "IDriver3D.h"
#include "Driver3D_IPlatform.h"
#include "../math/Matrix.h"
#include "../math/Vector2.h"
#include "../math/Vector4.h"
#include <cassert>


struct TextureOGL
{
    int32_t m_nWidth;
    int32_t m_nHeight;
    int32_t m_nMipCnt;
    bool m_bIsPow2;
    int32_t m_nFrameCnt;

    uint32_t *m_pData[32];
};

struct PolygonDataOGL
{
    Vector3 nrmlWS;
    Vector3 cenWS;
    float   radius2_WS;
};
class IndexBuffer;

class Driver3D_OGL : public IDriver3D
{
    public:

    public:
        Driver3D_OGL();
        virtual ~Driver3D_OGL();

        bool Init(int32_t w, int32_t h) override;
        void Present() override;
        void Clear(bool bClearColor=true) override;

        void SetWorldMatrix(Matrix *pMtx, int32_t worldX, int32_t worldY) override;
        void SetViewMatrix(Matrix *pMtx, const Vector3 *pLoc, const Vector3 *pDir) override;
        void SetProjMtx(Matrix *pMtx) override;
        void SetCamera(Camera *pCamera) override;

        void ChangeWindowSize(int32_t w, int32_t h) override;

        //Texture Functions.
        void SetTexture(int32_t slot, TextureHandle hTex, uint32_t uFilter=FILTER_NORMAL, bool bWrap=true, int32_t frame=-1) override;
        void SetColor(const Vector4 *pColor=nullptr) override;
        TextureHandle CreateTexture(uint32_t uWidth, uint32_t uHeight, uint32_t uFormat=TEX_FORMAT_RGBA8, uint8_t *pData=0, bool bGenMips=false, int32_t nFrameCnt=1) override;
        void FillTexture(TextureHandle hTex, uint8_t *pData, uint32_t uWidth, uint32_t uHeight, bool bGenMips=false) override;
        void FreeTexture(TextureHandle hTex) override;

        //VBO/IBO Support.
        uint32_t CreateVBO() override;
        void AllocVBO_Mem(uint32_t uID, uint32_t uVtxCnt, uint32_t uSize, bool bDynamic) override;
        void FillVBO(uint32_t uID, void *pData, uint32_t uSize, bool bDynamic) override;
        void SetVBO(uint32_t uID, uint32_t uStride, uint32_t uVBO_Flags) override;
        uint32_t CreateIB() override;
        void FillIB(uint32_t uID, void *pData, uint32_t uSize, bool bDynamic) override;
        void DeleteBuffer(uint32_t uID) override;
        void ClearDrawData() override;

        //Draw!
        void RenderIndexedTriangles(IndexBuffer *pIB, int32_t nTriCnt, int32_t startIndex=0) override;
        void RenderScreenQuad(const Vector4& posScale, const Vector2& uvTop, const Vector2& uvBot, const Vector4& colorTop, const Vector4& colorBot) override;
        void RenderWorldQuad(const Vector3& pos0, const Vector3& pos1, const Vector2& uv0, const Vector2& uv1, const Vector4& color) override;
        void RenderWorldQuad(const Vector3 *posList, const Vector2 *uvList, const Vector4& color, bool bRecieveLighting=false) override;
        void RenderWorldQuad(const Vector3 *posList, const Vector2 *uvList, const Vector4 *color, bool bRecieveLighting=false) override;

        //Render States
        void EnableDepthWrite(bool bEnable) override;
        void EnableDepthRead(bool bEnable) override;
        void EnableStencilWriting(bool bEnable, uint32_t uValue) override;
        void EnableStencilTesting(bool bEnable) override;
        void EnableCulling(bool bEnable) override;
        void EnableAlphaTest(bool bEnable, uint8_t uAlphaCutoff=128) override;
        void SetBlendMode(uint32_t uMode=BLEND_NONE) override;
        void EnableFog(bool bEnable, float fEnd=10000.0f) override;
        void SetFogDensity(float fDensity=1.0f) override;

        //Sorting
        bool ApplyOpaqueSort() override { return true; }
        bool ApplyTransSort() override { return true; }

        Camera *GetCamera() override { return m_pRenderCamera; }
        //Driver extensions
        void SetExtension_Data(uint32_t uExtension, void *pData0, void *pData1) override;
        void SetClearColorFromTex(TextureHandle hTex) override;
        static uint8_t GetColormapID() { return s_uColormapID; }
        static TextureOGL *GetCurTex() { return m_pCurTex; }

    protected:

        void GenerateMips(uint32_t uWidth, uint32_t uHeight, uint8_t *pData);

    private:
        uint32_t m_Textures[16384];
        uint32_t m_uTextureCnt;
        TextureHandle *m_pTexArray;
        uint16_t *m_pTexIndex;

        Camera *m_pRenderCamera;
        void BuildColorTables_32bpp(int refPalIndex=112);
        // TextureOGL *CreateCheckPattern();
        static TextureOGL *m_pCurTex;
        static uint32_t s_uColormapID;
        PolygonDataOGL *m_pCurPolygonData;
};

#endif // DRIVER3D_OGL_H

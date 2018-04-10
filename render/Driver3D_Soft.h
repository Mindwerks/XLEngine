#ifndef DRIVER3D_SOFT_H
#define DRIVER3D_SOFT_H

#include "IDriver3D.h"
#include "Driver3D_IPlatform.h"
#include "../math/Matrix.h"
#include "../math/Vector2.h"
#include "../math/Vector4.h"
#include <cassert>
#include <vector>

class IndexBuffer;
struct Texture;
struct VFmt_Pos_UV_Screen;
struct VBO;
struct IBO;
struct TriGradients;
struct TriEdge;
class Camera;

struct Texture
{
    int32_t m_nWidth;
    int32_t m_nHeight;
    int32_t m_nMipCnt;
    bool m_bIsPow2;
    int32_t m_nFrameCnt;

    uint32_t *m_pData[32];
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

        bool Init(int32_t w, int32_t h) override;
        void Present() override;
        void Clear(bool bClearColor=true) override;

        void SetWorldMatrix(Matrix *pMtx, int32_t worldX, int32_t worldY) override;
        void SetViewMatrix(Matrix *pMtx, Vector3 *pLoc, Vector3 *pDir) override;
        void SetProjMtx(Matrix *pMtx) override;
        void SetCamera(Camera *pCamera) override;
        
        void ChangeWindowSize(int32_t w, int32_t h) override;

        //Texture Functions.
        void SetTexture(int32_t slot, TextureHandle hTex, uint32_t uFilter=FILTER_NORMAL, bool bWrap=true, int32_t frame=-1) override;
        void SetColor(Vector4 *pColor=0) override;
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
        void ResetIBFlags(uint32_t uID) override;
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
        bool ApplyOpaqueSort() override { return false; }
        bool ApplyTransSort() override { return false; }

        Camera *GetCamera() override { return m_pRenderCamera; }

        //Software Rendering specific.
        void SetBitDepth(int32_t bitDepth);
        int32_t GetBitDepth() { return m_nBitDepth; }
        int GetAffineLength();
        int GetFrameWidth();
        int GetFrameHeight();
        bool GetBilinear() { return m_bBilinear; }
        bool GetGouraud()  { return m_bGouraud; }
        void SetClearColorFromTex(TextureHandle hTex);
        //optional polygon data to accelerate triangle processing.
        static uint8_t GetColormapID() { return s_uColormapID; }
        static Texture *GetCurTex() { return m_pCurTex; }

        //Driver extensions
        void SetExtension_Data(uint32_t uExtension, void *pData0, void *pData1) override;

    protected:

        void GenerateMips(uint32_t uWidth, uint32_t uHeight, uint8_t *pData);

    private:
        std::vector<Texture *> m_Textures;
        std::vector<VBO *> m_VBO;
        std::vector<IBO *> m_IBO;
        int32_t m_nBitDepth;
        Matrix m_ProjMtx;
        Matrix m_ViewProj;
        Camera *m_pRenderCamera;
        static Texture *m_pCurTex;
        static uint32_t s_uColormapID;

        VBO *m_pCurVBO;
        float *m_PosStream;
        float *m_TCoordStream;
        float *m_NrmlStream;
        uint32_t m_uVBO_Stride;
        bool m_bBilinear;
        bool m_bGouraud;

        uint32_t m_uBlendMode;
        bool m_bAlphaTest;
        float m_fTimer;

        uint32_t m_uDbg_AllocSize;
        TextureHandle *m_pTexArray;
        uint16_t *m_pTexIndex;
        PolygonData *m_pCurPolygonData;

        Texture *CreateCheckPattern();
        
        void WireframeTriangle(VFmt_Pos_UV_Screen *pScrVert);
        void LocalToWorld(VBO *pVBO);
        void WorldToClip(VBO *pVBO);
        void ConvertFrameBufferTo32bpp(uint32_t *pal);
        void BuildTransTable();
        void BuildColorTables_32bpp(int refPalIndex=112);

        void RenderScreenQuad_8bpp(const Vector4& posScale, const Vector2& uvTop, const Vector2& uvBot, const Vector4& colorTop, const Vector4& colorBot);
        void RenderOverlays();
};

#endif // DRIVER3D_SOFT_H

#include "Driver3D_Soft.h"
#include "../Engine.h"
#include "../math/Math.h"
#include "../math/FixedPoint.h"
#include "../EngineSettings.h"
#include "../fileformats/TextureLoader.h"
#include "Camera.h"
#include "triangleRasterizer/TriangleRasterizer.h"
#include "triangleRasterizer/DrawScanline.h"
#include "IndexBuffer.h"
#include "../os/Clock.h"

#include <cstdio>
#include <malloc.h>
#include <cfloat>
#include <cstdlib>
#include <cstdint>

#if PLATFORM_WIN    //we have to include Windows.h before gl.h on Windows platforms.
    #define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
    // Windows Header Files:
    #include <windows.h>
#endif

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

#define TEST_COLORMAP 0
#define BUFFER_OFFSET(i) ((char *)nullptr + (i))

Texture *Driver3D_Soft::m_pCurTex;
uint32_t Driver3D_Soft::s_uColormapID;

Matrix *_prevWorldMtxPtr_Soft = nullptr;
int32_t _prevWorldX = 0;
int32_t _prevWorldY = 0;
uint32_t *m_pFrameBuffer_32bpp;
uint8_t  *m_pFrameBuffer_8bpp;
uint16_t *m_pDepthBuffer;
uint32_t m_FrameWidth;
uint32_t m_FrameHeight;
GLuint m_VideoFrameBuffer;
bool m_bVideoMemAllocated=false;
uint8_t  _fog_table[256];
uint8_t m_colorRed=0xff, m_colorGrn=0xff, m_colorBlue=0xff;
int m_nMatrixViewKey  = 0;
uint32_t m_uMatrixWorldKey = 0;
int m_AffineLength;
float m_QuadWidthScale  = 1.0f;
float m_QuadHeightScale = 1.0f;
float m_fTexAnimFrameRate = 8.0f;

int32_t _trianglesPerFrame = 0;
static uint32_t *_pCurPal = nullptr;

//#define MAX_FRAMEBUFFER_WIDTH 2048
#define MAX_FRAMEBUFFER_WIDTH 1024

void *aligned_malloc(size_t size, size_t align_size)
{
    char *ptr, *ptr2, *aligned_ptr;
    int align_mask = (int)align_size-1;

    ptr = (char *)malloc( size + align_size + sizeof(int) );
    if ( ptr == nullptr)
        return nullptr;

    ptr2 = ptr + sizeof(int);
    aligned_ptr = ptr2 + ( align_size - ((size_t)ptr2 & align_mask) );

    ptr2 = aligned_ptr - sizeof(int);
    *((int *)ptr2) = (int)(aligned_ptr - ptr);

    return aligned_ptr;
}

void aligned_free(void *ptr)
{
    if ( ptr )
    {
        int *ptr2 = (int *)ptr - 1;
        ptr = (char *)ptr - *ptr2;
        free(ptr);
    }
}

Driver3D_Soft::Driver3D_Soft() : IDriver3D()
{
    m_nBitDepth = 32;
    m_pCurVBO   = nullptr;
    m_pTexArray = nullptr;
    m_pTexIndex = nullptr;
    m_pRenderCamera = nullptr;
    m_pCurPolygonData = nullptr;
    m_bBilinear = false;
    m_bGouraud  = false;
    m_uClearColor = 0;
    m_Textures.clear();
    TriangleRasterizer::BuildTables();
    _pCurPal = xlNew uint32_t[256];
    m_fTimer = 0.0f;

    m_uExtensions = EXT_TEXTURE_INDEX | EXT_GOURAUD | EXT_POLYGON_DATA;
}

Driver3D_Soft::~Driver3D_Soft()
{
    //delete textures.
    if ( m_Textures.size() )
    {
        for (Texture *tex : m_Textures)
        {
            if (tex != nullptr)
            {
                for (int32_t f = 0; f < tex->m_nFrameCnt; f++)
                {
                    free(tex->m_pData[f]);
                }

                delete tex;
            }
        }

        m_Textures.clear();
    }

    aligned_free(m_pFrameBuffer_32bpp);
    aligned_free(m_pFrameBuffer_8bpp);
    aligned_free(m_pDepthBuffer);

    xlDelete [] _pCurPal;
}

void Driver3D_Soft::ChangeWindowSize(int32_t w, int32_t h)
{
    m_nWindowWidth  = w;
    m_nWindowHeight = h;
}

Texture *Driver3D_Soft::CreateCheckPattern()
{
    Texture *pTex = new Texture();

    //create a check test pattern... 64x64
    pTex->m_nWidth  = 64;
    pTex->m_nHeight = 64;
    pTex->m_nMipCnt = 1;
    pTex->m_bIsPow2 = true;
    uint32_t *pImage = (uint32_t *)malloc(64*64*4);
    pTex->m_nFrameCnt = 1;
    pTex->m_pData[0] = pImage;

    for (int y=0; y<64; y++)
    {
        for (int x=0; x<64; x++)
        {
            int xx = x>>2;
            int yy = y>>2;

            int on = 0;
            if ( yy&1 )
                on = xx&1;
            else
                on = (xx+1)&1;

            pImage[x] = on ? 0xff202080 : 0xff808080;
        }
        pImage += 64;
    }

    m_Textures.push_back( pTex );
    return pTex;
}

bool Driver3D_Soft::Init(int32_t w, int32_t h)
{
    //initialize GLEW for extension loading.
    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        return false;
    }
    if ( GLEW_VERSION_1_1 == false )
    {
        #ifdef _WINDOWS
            OutputDebugString("OpenGL Version 1.1 is not supported. Aborting XL Engine startup.");
        #endif
        printf("OpenGL Version 1.1 is not supported. Aborting XL Engine startup.");
    }

    glDisable(GL_DEPTH_TEST); /* enable depth buffering */

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    /* establish initial viewport */
    glViewport(0, 0, w, h);

    m_nWindowWidth  = w;
    m_nWindowHeight = h;
    
    //frame size.
    if ( EngineSettings::get().IsFeatureEnabled(EngineSettings::EMULATE_320x200) )
    {
        m_FrameWidth  = 240*w/h;
        m_FrameHeight = 200;
    }
    else
    {
        m_FrameWidth  = MIN(w, MAX_FRAMEBUFFER_WIDTH);
        m_FrameHeight = MIN(h, MAX_FRAMEBUFFER_WIDTH*h/w);
    }
    m_QuadWidthScale  = (float)m_FrameWidth /(float)m_nWindowWidth;
    m_QuadHeightScale = (float)m_FrameHeight/(float)m_nWindowHeight;

    m_AffineLength = 8;
    if ( m_FrameWidth > 640 )
         m_AffineLength = 16;

    glActiveTexture(GL_TEXTURE0);

    //Create a copy of the framebuffer on the GPU so we can upload the results there.
    glGenTextures(1, &m_VideoFrameBuffer);
    glBindTexture(GL_TEXTURE_2D, m_VideoFrameBuffer);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glBindTexture(GL_TEXTURE_2D, 0);
    glEnable(GL_TEXTURE_2D);
    
    glFlush();

    //allocate the depth buffer.
    if ( m_nBitDepth == 32 )
    {
        m_pFrameBuffer_32bpp = (uint32_t *)aligned_malloc( m_FrameWidth*m_FrameHeight*sizeof(uint32_t), 16 );
        m_pFrameBuffer_8bpp  = nullptr;
    }
    else
    {
        m_pFrameBuffer_8bpp  = (uint8_t *)aligned_malloc( m_FrameWidth*m_FrameHeight*sizeof(uint8_t), 16 );
        m_pFrameBuffer_32bpp = (uint32_t *)aligned_malloc( m_FrameWidth*m_FrameHeight*sizeof(uint32_t), 16 );
    }
    m_pDepthBuffer = (uint16_t *)aligned_malloc( m_FrameWidth*m_FrameHeight*sizeof(uint16_t), 16 );

    //init fog table
    for (int z=0; z<256; z++)
    {
        float fZ = (float)z / 255.0f;
        float F = 1.0f - fZ;
        if ( F < 0.0f ) F = 0.0f;
        if ( F > 1.0f ) F = 1.0f;
        _fog_table[z] = (uint8_t)(F*255.0f);
    }

    //Create a default texture.
    CreateCheckPattern();

    m_uBlendMode = BLEND_NONE;
    m_bAlphaTest = false;

    s_uColormapID = m_uColormapID;

    DrawScanline::_nAffineLength = m_AffineLength;
    DrawScanline::_nFrameWidth   = m_FrameWidth;
    DrawScanline::_nFrameHeight  = m_FrameHeight;
    DrawScanline::_uColormapID   = s_uColormapID;
    DrawScanline::_pCurTex       = nullptr;
    DrawScanline::_pFrameBuffer_32 = m_pFrameBuffer_32bpp;
    DrawScanline::_pFrameBuffer_8  = m_pFrameBuffer_8bpp;
    DrawScanline::_pDepthBuffer    = m_pDepthBuffer;

    return true;
}

void Driver3D_Soft::EnableDepthWrite(bool bEnable)
{
}

void Driver3D_Soft::EnableDepthRead(bool bEnable)
{
}

void Driver3D_Soft::EnableCulling(bool bEnable)
{
}

void Driver3D_Soft::EnableAlphaTest(bool bEnable, uint8_t uAlphaCutoff)
{
    m_bAlphaTest = bEnable;
}

void Driver3D_Soft::SetBlendMode(uint32_t uMode)
{
    m_uBlendMode = uMode;
}

void Driver3D_Soft::SetFogDensity(float fDensity)
{
}

void Driver3D_Soft::EnableFog(bool bEnable, float fEnd)
{
    DrawScanline::_useFog = bEnable;
}

void Driver3D_Soft::EnableStencilWriting(bool bEnable, uint32_t uValue)
{
}

void Driver3D_Soft::EnableStencilTesting(bool bEnable)
{
}

void Driver3D_Soft::SetBitDepth(int32_t bitDepth)
{ 
    m_nBitDepth = bitDepth; 
    TextureLoader::SetTextureColorDepth(8);// m_nBitDepth );
}

void Driver3D_Soft::SetExtension_Data(uint32_t uExtension, void *pData0, void *pData1)
{
    if ( uExtension == EXT_TEXTURE_INDEX )
    {
        m_pTexArray = (TextureHandle *)pData0;
        m_pTexIndex = (uint16_t *)pData1;
    }
    else if ( uExtension == EXT_GOURAUD )
    {
        int32_t enable = *((int32_t *)pData0);
        m_bGouraud = enable ? true : false;
    }
    else if ( uExtension == EXT_POLYGON_DATA )
    {
        m_pCurPolygonData = (PolygonData *)pData0;
    }
}

int Driver3D_Soft::GetAffineLength()
{
    return m_AffineLength;
}

int Driver3D_Soft::GetFrameWidth()
{
    return m_FrameWidth;
}

int Driver3D_Soft::GetFrameHeight()
{
    return m_FrameHeight;
}

void Driver3D_Soft::SetClearColorFromTex(TextureHandle hTex)
{
    m_pCurTex = m_Textures[hTex];
    m_uClearColor = ((uint8_t *)m_pCurTex->m_pData[0])[ (m_pCurTex->m_nHeight-1)*m_pCurTex->m_nWidth ];
}

void Driver3D_Soft::ConvertFrameBufferTo32bpp(uint32_t *pal)
{
    uint8_t *pSource = m_pFrameBuffer_8bpp;
    uint32_t *pDest  = m_pFrameBuffer_32bpp;

    uint32_t uPixelCount = m_FrameWidth*m_FrameHeight;
    for (uint32_t p=0; p<uPixelCount; p++)
    {
        *pDest++ = pal[ *pSource++ ];
    }
}

//Given color A and background color B, the table contains the closest match for
//A*0.5+B*0.5
void Driver3D_Soft::BuildColorTables_32bpp(int refPalIndex/*=112*/)
{
    uint8_t *pColormap = TextureLoader::GetColormapData(s_uColormapID);

    int min_r = (_pCurPal[ pColormap[0] ]>>16)&0xff;
    int min_g = (_pCurPal[ pColormap[0] ]>> 8)&0xff;
    int min_b = (_pCurPal[ pColormap[0] ]    )&0xff;

    for (int c=255; c>=0; c--)
    {
        for (int l=0; l<256; l++)
        {
            int light = l>>2;
            int index = pColormap[refPalIndex + (light<<8)];
            int r, g, b;

            int r0 = (_pCurPal[index]>>16)&0xff;
            int g0 = (_pCurPal[index]>> 8)&0xff;
            int b0 = (_pCurPal[index]    )&0xff;

            if ( 0 && l < 255 )
            {
                index = pColormap[refPalIndex + ((light+1)<<8)];
                int r1 = (_pCurPal[index]>>16)&0xff;
                int g1 = (_pCurPal[index]>> 8)&0xff;
                int b1 = (_pCurPal[index]    )&0xff;

                int rem = l - (light<<2);
                r = ( (r0*(4-rem))>>2 ) + ((r1*rem)>>2);
                g = ( (g0*(4-rem))>>2 ) + ((g1*rem)>>2);
                b = ( (b0*(4-rem))>>2 ) + ((b1*rem)>>2);
            }
            else
            {
                r = r0;
                g = g0;
                b = b0;
            }

            DrawScanline::_colorMap32[0][c + (l<<8)] = Math::clamp(r*c/220, min_r, 255) << 16;
            DrawScanline::_colorMap32[1][c + (l<<8)] = Math::clamp(g*c/220, min_g, 255) <<  8;
            DrawScanline::_colorMap32[2][c + (l<<8)] = Math::clamp(b*c/220, min_b, 255);
        }
    }
}

void Driver3D_Soft::BuildTransTable()
{
    for (int cSrc=0; cSrc<256; cSrc++)
    {
        for (int cDst=0; cDst<256; cDst++)
        {
            int tIdx = cSrc + (cDst<<8);
            if ( cSrc == 0 )
            {
                DrawScanline::_aTransTable_Blend[tIdx] = cDst;
                DrawScanline::_aTransTable_Add[tIdx] = cDst;
            }
            else
            {
                //Compute the blended RGB value.
                int rSrc = (_pCurPal[cSrc]>>16)&0xff;
                int gSrc = (_pCurPal[cSrc]>> 8)&0xff;
                int bSrc = (_pCurPal[cSrc]    )&0xff;

                int rDst = (_pCurPal[cDst]>>16)&0xff;
                int gDst = (_pCurPal[cDst]>> 8)&0xff;
                int bDst = (_pCurPal[cDst]    )&0xff;

                int rBld = (rSrc + rDst)>>1;
                int gBld = (gSrc + gDst)>>1;
                int bBld = (bSrc + bDst)>>1;

                int rAdd = MIN(rSrc + rDst, 0xff);
                int gAdd = MIN(gSrc + gDst, 0xff);
                int bAdd = MIN(bSrc + bDst, 0xff);

                //Now search the entire palette for a matching RGB value.
                uint64_t uMinDist2 = 0xffffffffffffffff;
                int blendIndex = -1;
                for (int cBld=0; cBld<256; cBld++)
                {
                    int32_t rChk = (_pCurPal[cBld]>>16)&0xff;
                    int32_t gChk = (_pCurPal[cBld]>> 8)&0xff;
                    int32_t bChk = (_pCurPal[cBld]    )&0xff;

                    int32_t rDiff = rChk - rBld;
                    int32_t gDiff = gChk - gBld;
                    int32_t bDiff = bChk - bBld;

                    uint64_t rDist2 = (uint64_t)(rDiff*rDiff) + (uint64_t)(gDiff*gDiff) + (uint64_t)(bDiff*bDiff);
                    if ( rDist2 < uMinDist2 )
                    {
                        uMinDist2 = rDist2;
                        blendIndex = cBld;
                    }
                }
                assert(blendIndex > -1);
                DrawScanline::_aTransTable_Blend[tIdx] = blendIndex;

                //Now search the entire palette for a matching RGB value.
                uMinDist2 = 0xffffffffffffffff;
                blendIndex = -1;
                for (int cBld=0; cBld<256; cBld++)
                {
                    int32_t rChk = (_pCurPal[cBld]>>16)&0xff;
                    int32_t gChk = (_pCurPal[cBld]>> 8)&0xff;
                    int32_t bChk = (_pCurPal[cBld]    )&0xff;

                    int32_t rDiff = rChk - rAdd;
                    int32_t gDiff = gChk - gAdd;
                    int32_t bDiff = bChk - bAdd;

                    uint64_t rDist2 = (uint64_t)(rDiff*rDiff) + (uint64_t)(gDiff*gDiff) + (uint64_t)(bDiff*bDiff);
                    if ( rDist2 < uMinDist2 )
                    {
                        uMinDist2 = rDist2;
                        blendIndex = cBld;
                    }
                }
                assert(blendIndex > -1);
                DrawScanline::_aTransTable_Add[tIdx] = blendIndex;
            }
        }
    }
}

void Driver3D_Soft::Present()
{
    //test - write out the color map.
#if TEST_COLORMAP
    static bool bTestColormap = false;
    static TextureHandle hColorMap;
    static int nWait = 200;
    if ( nWait < 0 )
    {
        if ( bTestColormap == false )
        {
            bTestColormap = true;
            uint8_t *pColormap  = TextureLoader::GetColormapData(4);
            hColorMap      = CreateTexture(256, 64, TEX_FORMAT_RGBA8, pColormap);
        }

        Vector4 color(1,1,1,1);
        Vector4 posScale(0, 0, 512, 128);
        Vector2 uvTop(0, 0), uvBot(1, 1);
        SetTexture(0, hColorMap);
        RenderScreenQuad_8bpp(posScale, uvTop, uvBot, color, color);
    }
    else
    {
        nWait--;
    }
#endif

    //if ( m_nBitDepth == 8 )
    {
        static uint32_t _palIdx = 0xffffffff;
        if ( _pCurPal == nullptr || _palIdx != m_uPaletteID || m_bUpdatePal )
        {
            uint8_t *pal = TextureLoader::GetPaletteData(m_uPaletteID);
            int index = 0;
            for (uint32_t p=0; p<256; p++)
            {
                uint8_t r = pal[ index+0 ];
                uint8_t g = pal[ index+1 ];
                uint8_t b = pal[ index+2 ];
                uint8_t a = pal[ index+3 ];

                _pCurPal[p] = (a<<24) | (r<<16) | (g<<8) | b;
    
                index += 4;
            }
            _palIdx = m_uPaletteID;
            //BuildTransTable();
            //if ( m_nBitDepth == 32 )
            {
                //BuildColorTables_32bpp();
            }
            DrawScanline::_pCurPal = _pCurPal;

            m_bUpdatePal = false;
        }

        if ( m_nBitDepth == 8 )
        {
            ConvertFrameBufferTo32bpp(_pCurPal);
        }
    }
    //Render Overlays directly into the framebuffer.
    RenderOverlays();

    //update the video memory framebuffer.
    glBindTexture(GL_TEXTURE_2D, m_VideoFrameBuffer);
    if ( m_bVideoMemAllocated )
    {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_FrameWidth, m_FrameHeight, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, m_pFrameBuffer_32bpp);
    }
    else
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_FrameWidth, m_FrameHeight, 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, m_pFrameBuffer_32bpp);
    }
    
    //scale and display.
    Vector4 posScale(-1.0f, -1.0f, 2.0f, 2.0f);
    Vector2 uvTop(0, 0), uvBot(1, 1);

    glBegin(GL_QUADS);
        glTexCoord2f(uvTop.x, uvTop.y);
        glVertex3f(posScale.x, posScale.y, -1.0f);
        glTexCoord2f(uvBot.x, uvTop.y);
        glVertex3f(posScale.x+posScale.z, posScale.y, -1.0f);

        glTexCoord2f(uvBot.x, uvBot.y);
        glVertex3f(posScale.x+posScale.z, posScale.y+posScale.w, -1.0f);
        glTexCoord2f(uvTop.x, uvBot.y);
        glVertex3f(posScale.x, posScale.y+posScale.w, -1.0f);
    glEnd();

    ClearDrawData();
    m_Platform->Present();
    glBindTexture(GL_TEXTURE_2D, 0);

    m_fTimer += Clock::GetDeltaTime(1000000.0f);
}

void Driver3D_Soft::Clear(bool bClearColor)
{
    s_uColormapID = m_uColormapID;
    DrawScanline::_uColormapID = s_uColormapID;

    if ( bClearColor && m_nBitDepth == 32 )
    {
        uint8_t *pColormap = TextureLoader::GetColormapData(m_uColormapID);
        uint32_t uColor = _pCurPal ? _pCurPal[ pColormap[0] ] : 0;
        for (uint32_t i=0; i<m_FrameWidth*m_FrameHeight; i++) { m_pFrameBuffer_32bpp[i] = uColor; }
        //memset(m_pFrameBuffer_32bpp, _pCurPal ? _pCurPal[ pColormap[0] ] : 0, m_FrameWidth*m_FrameHeight*4);
    }
    else if ( bClearColor && m_nBitDepth == 8 )
    {
        uint8_t *pColormap = TextureLoader::GetColormapData(m_uColormapID);
        uint8_t uColor = m_uClearColor;
        if ( uColor == 0 )
        {
            uColor = pColormap[0];
        }
        memset(m_pFrameBuffer_8bpp, uColor, m_FrameWidth*m_FrameHeight);
    }

    memset(m_pDepthBuffer, 0xff, m_FrameWidth*m_FrameHeight*2);
}

void Driver3D_Soft::SetWorldMatrix(Matrix *pMtx, int32_t worldX, int32_t worldY)
{
    if ( pMtx == nullptr )
    {
        _prevWorldMtxPtr_Soft = nullptr;
    }
    else if ( pMtx != _prevWorldMtxPtr_Soft || worldX != _prevWorldX || worldY != _prevWorldY )
    {
        m_WorldMtx = *pMtx;
        //handle world position.
        if ( worldX || worldY )
        {
            m_WorldMtx.m[12] += (worldX - m_pRenderCamera->GetWorldPosX()) * 1024.0f;
            m_WorldMtx.m[13] += (worldY - m_pRenderCamera->GetWorldPosY()) * 1024.0f;
        }
        m_WorldView = m_ViewMtx.MatMul( *pMtx );
        
        _prevWorldMtxPtr_Soft = pMtx;
        _prevWorldX = worldX;
        _prevWorldY = worldY;
        m_uMatrixWorldKey = (intptr_t)pMtx;
    }
}

void Driver3D_Soft::SetViewMatrix(Matrix *pMtx, Vector3 *pLoc, Vector3 *pDir)
{
    m_ViewMtx = *pMtx;
    m_Eye = *pLoc;
    m_ViewDir = *pDir;
    _prevWorldMtxPtr_Soft = nullptr;   //reset so the world matrix applies the proper transform.

    m_ViewProj = m_ProjMtx.MatMul( m_ViewMtx );
    m_nMatrixViewKey++;
}

void Driver3D_Soft::SetProjMtx(Matrix *pMtx)
{
    m_ProjMtx  = *pMtx;
    m_ViewProj = m_ProjMtx.MatMul( m_ViewMtx );
    m_nMatrixViewKey++;
}

void Driver3D_Soft::SetCamera(Camera *pCamera)
{
    m_pRenderCamera = pCamera;
}

/************** TEXTURE SUPPORT ******************/
void Driver3D_Soft::SetTexture(int32_t slot, TextureHandle hTex, uint32_t uFilter, bool bWrap, int32_t frame)
{
    if ( hTex == XL_INVALID_TEXTURE )
         hTex = 0;

    m_pCurTex = m_Textures[hTex];
    DrawScanline::_pCurTex = m_pCurTex;
    //Set the frame...
    if ( m_pCurTex && m_pCurTex->m_nFrameCnt == 1 )
    {
        DrawScanline::_uCurFrame = 0;
    }
    else if ( frame < 0 )
    {
        DrawScanline::_uCurFrame = (uint32_t)(m_fTimer * m_fTexAnimFrameRate) % m_pCurTex->m_nFrameCnt;
    }
    else
    {
        DrawScanline::_uCurFrame = frame;
    }

    assert( m_pCurTex && m_pCurTex->m_pData[ DrawScanline::_uCurFrame ] );
}

void Driver3D_Soft::SetColor(Vector4 *pColor)
{
}

TextureHandle Driver3D_Soft::CreateTexture(uint32_t uWidth, uint32_t uHeight, uint32_t uFormat/*=TEX_FORMAT_RGBA8*/, uint8_t *pData/*=nullptr*/, bool bGenMips/*=false*/, int32_t nFrameCnt/*=1*/)
{
    Texture *pTex = new Texture();

    pTex->m_nWidth  = uWidth;
    pTex->m_nHeight = uHeight;
    pTex->m_nMipCnt = 1;
    pTex->m_bIsPow2 = ( Math::RoundNextPow2(uWidth) == uWidth && Math::RoundNextPow2(uHeight) == uHeight ) ? true : false;

    assert( nFrameCnt < 32 );
    for (int32_t f=0; f<32; f++)
    {
        pTex->m_pData[f] = nullptr;
    }

    int32_t nBytesPerPixel = (uFormat!=TEX_FORMAT_FORCE_32bpp) ? (m_nBitDepth>>3) : 4;
    if ( pTex->m_bIsPow2 == false )
    {
        uint32_t dataSize = (uWidth+2)*(uHeight+2)*nBytesPerPixel;
        m_uDbg_AllocSize = dataSize;
        pTex->m_nFrameCnt = nFrameCnt;
        for (int32_t f=0; f<nFrameCnt; f++)
        {
            pTex->m_pData[f] = (uint32_t *)malloc(dataSize);
            memset(pTex->m_pData[f], 0, dataSize);
        }
    }
    else
    {
        uint32_t dataSize = uWidth*uHeight*nBytesPerPixel;
        if ( bGenMips )
        {
            uint32_t w=uWidth>>1, h=uHeight>>1;
            while (w >= 1 && h >= 1)
            {
                dataSize += (w*h);
                w >>= 1;
                h >>= 1;
            };
        }
        m_uDbg_AllocSize = dataSize;
        pTex->m_nFrameCnt = nFrameCnt;
        for (int32_t f=0; f<nFrameCnt; f++)
        {
            pTex->m_pData[f] = (uint32_t *)malloc(dataSize);
        }
    }

    if ( m_nBitDepth == 32 || uFormat==TEX_FORMAT_FORCE_32bpp )
    {
        uint32_t uFrameOffset = 0;
        for (int32_t f=0; f<nFrameCnt; f++)
        {
            int32_t yOffs = 0;
            uint32_t *pDestData = pTex->m_pData[f];
            for (uint32_t y=0; y<uHeight; y++)
            {
                for (uint32_t x=0; x<uWidth; x++)
                {
                    uint8_t r = pData[uFrameOffset + ((yOffs+x)<<2) + 0];
                    uint8_t g = pData[uFrameOffset + ((yOffs+x)<<2) + 1];
                    uint8_t b = pData[uFrameOffset + ((yOffs+x)<<2) + 2];
                    uint8_t a = pData[uFrameOffset + ((yOffs+x)<<2) + 3];

                    pDestData[yOffs+x] = (a<<24) | (r<<16) | (g<<8) | b;
                }
                yOffs += uWidth;
            }

            uFrameOffset += uWidth*uHeight*4;
        }
    }
    else
    {   
        uint32_t uFrameOffset = 0;
        for (int32_t f=0; f<nFrameCnt; f++)
        {
            memcpy(pTex->m_pData[f], &pData[uFrameOffset], uWidth*uHeight);
            if ( pTex->m_bIsPow2 && bGenMips )
            {
                GenerateMips(uWidth, uHeight, (uint8_t *)pTex->m_pData[f]);
            }

            uFrameOffset += (uWidth*uHeight);
        }
    }

    TextureHandle hTex = (TextureHandle)m_Textures.size();
    m_Textures.push_back( pTex );

    return hTex;
}

void Driver3D_Soft::FreeTexture(TextureHandle hTex)
{
}

void Driver3D_Soft::FillTexture(TextureHandle hTex, uint8_t *pData, uint32_t uWidth, uint32_t uHeight, bool bGenMips/*=false*/)
{
    if ( hTex < m_Textures.size() )
    {
        int32_t nBytesPerPixel = m_nBitDepth>>3;

        Texture *pTex = m_Textures[hTex];
        memcpy(pTex->m_pData[0], pData, uWidth*uHeight*nBytesPerPixel);
    }
}

void Driver3D_Soft::GenerateMips(uint32_t uWidth, uint32_t uHeight, uint8_t *pData)
{
    //assume 8 bit for now, for testing.
    uint32_t w  = uWidth>>1, h = uHeight>>1;
    uint32_t pW = uWidth,   pH = uHeight;
    uint32_t uIndex = uWidth*uHeight;
    uint8_t *pDstData = &pData[uIndex];
    uint8_t *pSrcData = pData;
    uint32_t level = 1;

    uint8_t *pal = TextureLoader::GetPaletteData(m_uPaletteID);

    while (w > 1 || h > 1)
    {
        for (uint32_t y=0; y<h; y++)
        {
            for (uint32_t x=0; x<w; x++)
            {
                uint8_t r, g, b;
                int32_t I[4];
                uint8_t index[4];

                index[0] = pSrcData[ (y<<1)*pW + (x<<1) ];
                index[1] = pSrcData[ (y<<1)*pW + (x<<1) + 1 ];
                index[2] = pSrcData[ ((y<<1)+1)*pW + (x<<1) ];
                index[3] = pSrcData[ ((y<<1)+1)*pW + (x<<1) + 1 ];

                r = pal[ index[0]*4+0 ];
                g = pal[ index[0]*4+1 ];
                b = pal[ index[0]*4+2 ];
                I[0] = r + g + b;

                r = pal[ index[1]*4+0 ];
                g = pal[ index[1]*4+1 ];
                b = pal[ index[1]*4+2 ];
                I[1] = r + g + b;

                r = pal[ index[2]*4+0 ];
                g = pal[ index[2]*4+1 ];
                b = pal[ index[2]*4+2 ];
                I[2] = r + g + b;

                r = pal[ index[3]*4+0 ];
                g = pal[ index[3]*4+1 ];
                b = pal[ index[3]*4+2 ];
                I[3] = r + g + b;

                int32_t ave = (I[0] + I[1] + I[2] + I[3])>>2;
                I[0] = abs(I[0] - ave);
                I[1] = abs(I[1] - ave);
                I[2] = abs(I[2] - ave);
                I[3] = abs(I[3] - ave);

                uint8_t finalIdx = 3;
                if ( I[0] <= I[1] && I[0] <= I[2] && I[0] <= I[3] )
                     finalIdx = 0;
                else if ( I[1] <= I[0] && I[1] <= I[2] && I[1] <= I[3] )
                     finalIdx = 1;
                else if ( I[2] <= I[0] && I[2] <= I[1] && I[2] <= I[3] )
                     finalIdx = 2;

                pDstData[y*w+x] = index[ finalIdx ];
            }
        }
        pSrcData = pDstData;

        uIndex += (w*h);
        pDstData = &pData[uIndex];
        
        pW = w;
        pH = h;
        w >>= 1;
        h >>= 1;

        level++;
    };
}

/*************** VBO/IBO Support *****************/
uint32_t Driver3D_Soft::CreateVBO()
{
    uint32_t uVBO_ID = (uint32_t)m_VBO.size();

    VBO *pVBO = new VBO;
    pVBO->nMatrixViewKey = -1;
    pVBO->uMatrixWorldKey = -1;
    m_VBO.push_back( pVBO );

    return uVBO_ID;
}

void Driver3D_Soft::AllocVBO_Mem(uint32_t uID, uint32_t uVtxCnt, uint32_t uSize, bool bDynamic)
{
    VBO *pVBO = m_VBO[uID];
    if ( pVBO )
    {
        pVBO->nVtxCnt = (int)uVtxCnt;
        pVBO->pSrcVtx = (float *)malloc(uSize);
        pVBO->pVtx    = (VFmt_Pos_UV *)malloc(uVtxCnt*sizeof(VFmt_Pos_UV));
        pVBO->pVtx_Clipped = (VFmt_Pos_UV_Clip *)malloc(uVtxCnt*sizeof(VFmt_Pos_UV_Clip));
    }
}

void Driver3D_Soft::FillVBO(uint32_t uID, void *pData, uint32_t uSize, bool bDynamic)
{
    VBO *pVBO = m_VBO[uID];
    if ( pVBO )
    {
        memcpy(pVBO->pSrcVtx, pData, uSize);
    }
}

void Driver3D_Soft::SetVBO(uint32_t uID, uint32_t uStride, uint32_t uVBO_Flags)
{
    assert(uID < m_VBO.size());
    m_pCurVBO   = m_VBO[uID];
    m_PosStream = m_pCurVBO->pSrcVtx;

    uint32_t uOffset = 12;
    if ( uVBO_Flags & VBO_HAS_NORMALS )
    {
        m_NrmlStream = &m_pCurVBO->pSrcVtx[uOffset>>2];
        uOffset += 12;
    }
    if ( uVBO_Flags & VBO_HAS_COLORS )
    {
        uOffset += 12;
    }
    if ( uVBO_Flags & VBO_HAS_TEXCOORDS )
    {
        m_TCoordStream = &m_pCurVBO->pSrcVtx[uOffset>>2];
        uOffset += 8;
    }
    m_uVBO_Stride = uStride>>2;

    if ( m_nMatrixViewKey != m_pCurVBO->nMatrixViewKey || m_uMatrixWorldKey != m_pCurVBO->uMatrixWorldKey )
    {
        LocalToWorld(m_pCurVBO);
        WorldToClip(m_pCurVBO);
        m_pCurVBO->nMatrixViewKey  = m_nMatrixViewKey;
        m_pCurVBO->uMatrixWorldKey = m_uMatrixWorldKey;
    }

    m_pCurVBO->uFlags = uVBO_Flags;
}

void Driver3D_Soft::DeleteBuffer(uint32_t uID)
{
}

uint32_t Driver3D_Soft::CreateIB()
{
    IBO *ibo    = new IBO;
    uint32_t uIBO_ID = (uint32_t)m_IBO.size();
    ibo->uFlags = 0;
    ibo->pRendererData = nullptr;
    m_IBO.push_back(ibo);

    return uIBO_ID;
}

void Driver3D_Soft::FillIB(uint32_t uID, void *pData, uint32_t uSize, bool bDynamic)
{
    IBO *ibo = m_IBO[uID];
    if ( ibo )
    {
        ibo->pIndices = new uint16_t[uSize>>1];
        memcpy(ibo->pIndices, pData, uSize);
    }
}

void Driver3D_Soft::ResetIBFlags(uint32_t uID)
{
    IBO *ibo = m_IBO[uID];
    if ( ibo )
    {
        ibo->uFlags = 0;
    }
}

void Driver3D_Soft::ClearDrawData()
{
    _trianglesPerFrame = 0;
    m_pCurVBO = nullptr;
    m_pCurPolygonData = nullptr;
    m_uBlendMode = BLEND_NONE;
    m_bAlphaTest = false;
}

void Driver3D_Soft::LocalToWorld(VBO *pVBO)
{
    VFmt_Pos_UV *pWorld = pVBO->pVtx;

    float *posStream  = m_PosStream;
    float *uvStream   = m_TCoordStream;
    float *nrmlStream = m_NrmlStream;
    uint32_t offset = 0;
    if ( pVBO->uFlags&VBO_WORLDSPACE )
    {
        for (int v=0; v<pVBO->nVtxCnt; v++)
        {
            const Vector3 *pos = (Vector3 *)&posStream[offset];
            const Vector2 *uv  = (Vector2 *)&uvStream[offset];
            
            pWorld[v].pos[0] = pos->x;
            pWorld[v].pos[1] = pos->y;
            pWorld[v].pos[2] = pos->z;

            pWorld[v].uv[0]  = uv->x;
            pWorld[v].uv[1]  = uv->y;

            if ( m_bGouraud )
            {
                Vector3 *N  = (Vector3 *)&nrmlStream[offset];
                pWorld[v].g = fabsf(N->z)*255.0f;//Math::Max(N->Dot(m_SunlightDir), 0.0f) * 255.0f;
            }

            offset += m_uVBO_Stride;
        }
    }
    else
    {
        for (int v=0; v<pVBO->nVtxCnt; v++)
        {
            Vector3 *pos = (Vector3 *)&posStream[offset];
            const Vector2 *uv  = (Vector2 *)&uvStream[offset];
            
            Vector3 posWS = m_WorldMtx.TransformVector(*pos);
            pWorld[v].pos[0] = posWS.x;
            pWorld[v].pos[1] = posWS.y;
            pWorld[v].pos[2] = posWS.z;

            pWorld[v].uv[0]  = uv->x;
            pWorld[v].uv[1]  = uv->y;

            if ( m_bGouraud )
            {
                Vector3 *N  = (Vector3 *)&nrmlStream[offset];
                pWorld[v].g = fabsf(N->z)*255.0f;//Math::Max(N->Dot(m_SunlightDir), 0.0f) * 255.0f;
            }

            offset += m_uVBO_Stride;
        }
    }
}

void Driver3D_Soft::WorldToClip(VBO *pVBO)
{
    VFmt_Pos_UV      *pWorld = pVBO->pVtx;
    VFmt_Pos_UV_Clip *pClip  = pVBO->pVtx_Clipped;

    for (int v=0; v<pVBO->nVtxCnt; v++)
    {
        Vector4 vtxW( pWorld[v].pos[0], pWorld[v].pos[1], pWorld[v].pos[2], 1.0f );
        Vector4 vtxS = m_ViewProj.TransformVector(vtxW);

        pClip[v].x =  vtxS.x;
        pClip[v].y = -vtxS.y;
        pClip[v].z =  vtxS.z;
        pClip[v].w =  vtxS.w;

        pClip[v].u = pWorld[v].uv[0];
        pClip[v].v = pWorld[v].uv[1];

        pClip[v].lx = pWorld[v].pos[0];
        pClip[v].ly = pWorld[v].pos[1];
        pClip[v].lz = pWorld[v].pos[2];

        pClip[v].g  = pWorld[v].g;
    }
}

//The function assumes a vertex buffer has already been set.
void Driver3D_Soft::RenderIndexedTriangles(IndexBuffer *pIB, int32_t nTriCnt, int32_t startIndex/*=0*/)
{
    if ( m_pCurVBO == nullptr )
        return;

    uint32_t uIB_ID = pIB->GetID();
    IBO *pIbo  = m_IBO[uIB_ID];
    uint16_t *pIndices = pIbo->pIndices;

    int alphaMode = 0;
    if ( m_bAlphaTest )
        alphaMode = 1;
    else if ( m_uBlendMode == IDriver3D::BLEND_ALPHA )
        alphaMode = 2;

#if 0
    if ( (m_pCurVBO->uFlags&VBO_WORLDSPACE) && !pIbo->uFlags && startIndex == 0 )
    {
        //cache polygon data.
        pIbo->pRendererData = xlNew PolygonData[nTriCnt];
        VFmt_Pos_UV *pWorld = m_pCurVBO->pVtx;

        PolygonData *polyData = (PolygonData *)pIbo->pRendererData;
        for (int t=0, i=startIndex; t<nTriCnt; t++, i+=3)
        {
            uint16_t *pIdx = &pIbo->pIndices[i];
            polyData[t].cenWS.Set(0,0,0);
            for (int v=0; v<3; v++)
            {
                Vector3 pos(pWorld[pIdx[v]].pos[0], pWorld[pIdx[v]].pos[1], pWorld[pIdx[v]].pos[2]);
                polyData[t].cenWS = polyData[t].cenWS+pos;
            }
            float fOOCnt = 1.0f/3.0f;
            polyData[t].cenWS.x *= fOOCnt;
            polyData[t].cenWS.y *= fOOCnt;
            polyData[t].cenWS.z *= fOOCnt;

            polyData[t].radius2_WS = 0.0f;
            for (int v=0; v<3; v++)
            {
                Vector3 pos(pWorld[pIdx[v]].pos[0], pWorld[pIdx[v]].pos[1], pWorld[pIdx[v]].pos[2]);
                Vector3 offs = pos - polyData[t].cenWS;
                float d2 = offs.Dot(offs);
                if ( d2 > polyData[t].radius2_WS )
                     polyData[t].radius2_WS = d2;
            }
            
            //Compute the world space normal.
            Vector3 U( pWorld[pIdx[2]].pos[0] - pWorld[pIdx[0]].pos[0], pWorld[pIdx[2]].pos[1] - pWorld[pIdx[0]].pos[1], 
                pWorld[pIdx[2]].pos[2] - pWorld[pIdx[0]].pos[2] );
            Vector3 V( pWorld[pIdx[1]].pos[0] - pWorld[pIdx[0]].pos[0], pWorld[pIdx[1]].pos[1] - pWorld[pIdx[0]].pos[1], 
                pWorld[pIdx[1]].pos[2] - pWorld[pIdx[0]].pos[2] );
            polyData[t].nrmlWS.Cross(U, V);
            polyData[t].nrmlWS.Normalize();
        }

        pIbo->uFlags = 1;
    }
#endif

    PolygonData *polyData = nullptr;//(PolygonData *)pIbo->pRendererData;
    for (int t=0, i=startIndex; t<nTriCnt; t++, i+=3)
    {
        //EXT_TEXTURE_INDEX
        if ( m_pTexArray )
        {
            int32_t texIndex = m_pTexArray[m_pTexIndex[t>>1]&0xff];
            assert( (m_pTexIndex[t>>1]&0xff) < (56*4) );
            DrawScanline::_pCurTex = m_Textures[ texIndex ];
            DrawScanline::_texFlip = m_pTexIndex[t>>1]>>8;
        }

        TriangleRasterizer::DrawClippedNGon_Indexed(this, m_pCurVBO, 3, &pIndices[i], s_uColormapID == 0 ? true : false, alphaMode, nullptr);//polyData?&polyData[t]:nullptr);
    }

    _trianglesPerFrame += nTriCnt;

    DrawScanline::_pCurTex = m_pCurTex;
    DrawScanline::_texFlip = 0;
}

void _RenderClippedQuad(Vector4 *pvClipSpacePos)
{

}

void Driver3D_Soft::RenderOverlays()
{
    for (uint32_t i=0; i<m_uOverlayCount; i++)
    {
        int32_t tw = m_Textures[ m_Overlays[i].hTex ]->m_nWidth;
        int32_t th = m_Textures[ m_Overlays[i].hTex ]->m_nHeight;

        int32_t tx = m_Overlays[i].x;
        int32_t ty = m_Overlays[i].y;
        if ( tx >= (int32_t)m_FrameWidth || ty >= (int32_t)m_FrameHeight )
            continue;

        int32_t tw_Clipped = Math::Min( tx+tw*m_Overlays[i].scale, (int32_t)m_FrameWidth )-tx;
        int32_t th_Clipped = Math::Min( ty+th*m_Overlays[i].scale, (int32_t)m_FrameHeight )-ty;
        if ( tw_Clipped <= 0 || th_Clipped <= 0 )
            continue;

        uint32_t *pImage = (uint32_t *)m_Textures[ m_Overlays[i].hTex ]->m_pData[0];
        int32_t xOffset = 0;
        if ( tx < 0 )
             xOffset = -tx;
        int32_t yOffset = 0;
        if ( ty < 0 )
            yOffset = -ty;

        int32_t texel_xOffset = xOffset/m_Overlays[i].scale;
        int32_t texel_yOffset = yOffset/m_Overlays[i].scale;
        int32_t tex_y = texel_yOffset;
        int32_t stepsPerTexel = m_Overlays[i].scale;
        int32_t stepsY = stepsPerTexel;
        for (int32_t y=0; y<th_Clipped-yOffset; y++)
        {
            uint32_t *pImageV = &pImage[ tex_y*tw + texel_xOffset ];
            int32_t fy = m_FrameHeight-(ty+y+yOffset)-1;

            uint32_t *pLine = &m_pFrameBuffer_32bpp[ fy*m_FrameWidth + tx + xOffset ];
            int32_t tex_x = 0;
            int32_t stepsX = stepsPerTexel;
            for (int32_t x=0; x<tw_Clipped-xOffset; x++)
            {
                *pLine++ = pImageV[tex_x];

                stepsX--;
                if ( stepsX == 0 )
                {
                     tex_x++;
                     stepsX = stepsPerTexel;
                }
            }

            stepsY--;
            if ( stepsY == 0 )
            {
                 tex_y++;
                 stepsY = stepsPerTexel;
            }
        }
    }
    m_uOverlayCount = 0;
}

void Driver3D_Soft::RenderScreenQuad(const Vector4& posScale, const Vector2& uvTop, const Vector2& uvBot, const Vector4& colorTop, const Vector4& colorBot)
{
    if ( m_nBitDepth == 8 )
    {
        RenderScreenQuad_8bpp(posScale, uvTop, uvBot, colorTop, colorBot);
        return;
    }
    uint32_t *pImage = (uint32_t *)m_pCurTex->m_pData[0];
    uint32_t *pImageV;
    int32_t tw = m_pCurTex->m_nWidth;
    int32_t th = m_pCurTex->m_nHeight;

    int32_t w = (int32_t)(posScale.z*m_QuadWidthScale);
    int32_t h = (int32_t)(posScale.w*m_QuadHeightScale);
    if ( w > (int32_t)m_FrameWidth )  w = (int32_t)m_FrameWidth;
    if ( h > (int32_t)m_FrameHeight ) h = (int32_t)m_FrameHeight;

    int32_t x = (int32_t)(posScale.x*m_QuadWidthScale);
    int32_t y = (int32_t)(m_FrameHeight-posScale.y*m_QuadHeightScale-h);
    if ( y < 0 ) y = 0;
    
    int32_t dudx = Fixed16_16Math::FloatToFixed( (float)tw / (float)w );
    int32_t dvdy = Fixed16_16Math::FloatToFixed( (float)th / (float)h );

    int32_t v = Fixed16_16Math::IntToFixed(th)-1;
    if ( m_uBlendMode == BLEND_NONE && m_bAlphaTest == false )
    {
        for (int32_t yy=y; yy<y+h; yy++)
        {
            int32_t u = 0;
            pImageV = &pImage[ (v>>16)*tw ];
            uint32_t *pLine = &m_pFrameBuffer_32bpp[ yy*m_FrameWidth + x ];
            for (int32_t xx=0; xx<w; xx++)
            {
                int U = (u>>16);
                *pLine++ = pImageV[ U ];
                u += dudx;
            }
            v -= dvdy;
        }
    }
    else
    {
        for (int32_t yy=y; yy<y+h; yy++)
        {
            int32_t u = 0;
            pImageV = &pImage[ (v>>16)*tw ];
            uint32_t *pLine = &m_pFrameBuffer_32bpp[ yy*m_FrameWidth + x ];
            for (int32_t xx=0; xx<w; xx++)
            {
                int U = (u>>16);
                if ( pImageV[U]&0xff000000 )
                    *pLine++ = pImageV[ U ];
                else
                    pLine++;
                u += dudx;
            }
            v -= dvdy;
        }
    }
}

void Driver3D_Soft::RenderScreenQuad_8bpp(const Vector4& posScale, const Vector2& uvTop, const Vector2& uvBot, const Vector4& colorTop, const Vector4& colorBot)
{
    uint8_t *pImage = (uint8_t *)m_pCurTex->m_pData[0];
    uint8_t *pImageV;
    int32_t tw = m_pCurTex->m_nWidth;
    int32_t th = m_pCurTex->m_nHeight;

    int32_t w = (int32_t)(posScale.z*m_QuadWidthScale);
    int32_t h = (int32_t)(posScale.w*m_QuadHeightScale);
    if ( w > (int32_t)m_FrameWidth )  w = (int32_t)m_FrameWidth;
    if ( h > (int32_t)m_FrameHeight ) h = (int32_t)m_FrameHeight;

    int32_t x = (int32_t)(posScale.x*m_QuadWidthScale);
    int32_t y = (int32_t)(m_FrameHeight-posScale.y*m_QuadHeightScale-h);
    if ( y < 0 ) y = 0;
    
    int32_t dudx = Fixed16_16Math::FloatToFixed( (float)tw / (float)w );
    int32_t dvdy = Fixed16_16Math::FloatToFixed( (float)th / (float)h );

    int32_t v = Fixed16_16Math::IntToFixed(th)-1;
    if ( m_uBlendMode == BLEND_NONE && m_bAlphaTest == false )
    {
        for (int32_t yy=y, yOffs=yy*m_FrameWidth; yy<y+h; yy++, yOffs+=m_FrameWidth)
        {
            int32_t u = 0;
            pImageV = &pImage[ (v>>16)*tw ];
            uint8_t *pLine = &m_pFrameBuffer_8bpp[ yOffs + x ];
            for (int32_t xx=0; xx<w; xx++)
            {
                int U = (u>>16);
                *pLine++ = pImageV[ U ];
                u += dudx;
            }
            v -= dvdy;
        }
    }
    else
    {
        for (int32_t yy=y, yOffs=yy*m_FrameWidth; yy<y+h; yy++, yOffs+=m_FrameWidth)
        {
            int32_t u = 0;
            pImageV = &pImage[ (v>>16)*tw ];
            uint8_t *pLine = &m_pFrameBuffer_8bpp[ yOffs + x ];
            for (int32_t xx=0; xx<w; xx++)
            {
                int U = (u>>16);
                if ( pImageV[U] )
                    *pLine++ = pImageV[ U ];
                else
                    pLine++;
                u += dudx;
            }
            v -= dvdy;
        }
    }
}

void Driver3D_Soft::RenderWorldQuad(const Vector3& pos0, const Vector3& pos1, const Vector2& uv0, const Vector2& uv1, const Vector4& color)
{
}

struct QuadVertex
{
    Vector3 pos;
    Vector2 uv;
};

void Driver3D_Soft::RenderWorldQuad(const Vector3 *posList, const Vector2 *uvList, const Vector4& color, bool bRecieveLighting)
{
    static VBO vbo;
    static IBO ibo;
    static bool bQuadSetup = false;
    if ( !bQuadSetup )
    {
        bQuadSetup = true;
        vbo.nVtxCnt = 4;
        vbo.pSrcVtx = (float *)malloc(sizeof(VFmt_Pos_UV)*4);
        vbo.pVtx_Clipped = (VFmt_Pos_UV_Clip *)malloc(sizeof(VFmt_Pos_UV_Clip)*4);
        vbo.pVtx = (VFmt_Pos_UV *)vbo.pSrcVtx;

        ibo.pIndices = xlNew uint16_t[4];
        ibo.pIndices[0] = 0;
        ibo.pIndices[1] = 1;
        ibo.pIndices[2] = 2;
        ibo.pIndices[3] = 3;
    }

    VFmt_Pos_UV *pVtxPosUV = (VFmt_Pos_UV *)vbo.pSrcVtx;
    for (int v=0; v<4; v++)
    {
        pVtxPosUV[v].pos[0] = posList[v].x;
        pVtxPosUV[v].pos[1] = posList[v].y;
        pVtxPosUV[v].pos[2] = posList[v].z;
        pVtxPosUV[v].uv[0]  = uvList[v].x;
        pVtxPosUV[v].uv[1]  = uvList[v].y;
    }

    m_pCurVBO      = &vbo;
    m_PosStream    = m_pCurVBO->pSrcVtx;
    m_TCoordStream = &m_pCurVBO->pSrcVtx[3];
    m_uVBO_Stride  = 4;

    WorldToClip(m_pCurVBO);

    int alphaMode = 0;
    if ( m_bAlphaTest )
        alphaMode = 1;
    else if ( m_uBlendMode == IDriver3D::BLEND_ALPHA )
        alphaMode = 2;

    TriangleRasterizer::DrawClippedNGon_Indexed(this, m_pCurVBO, 4, ibo.pIndices, (bRecieveLighting && s_uColormapID == 0) ? true : false, alphaMode);
}

void Driver3D_Soft::RenderWorldQuad(const Vector3 *posList, const Vector2 *uvList, const Vector4 *color, bool bRecieveLighting)
{
}

#include "Driver3D_OGL.h"
#include "../Engine.h"
#include "../math/Math.h"
#include "IndexBuffer.h"
#include "Camera.h"
#include <cstdio>
#include <cstdlib>

#include <inttypes.h>
#include "../ui/XL_Console.h"
#include "../fileformats/TextureLoader.h"

#if PLATFORM_WIN    //we have to include Windows.h before gl.h on Windows platforms.
    #define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
    // Windows Header Files:
    #include <windows.h>
#endif

#include <GL/glew.h>
#ifdef __APPLE__
    #include <OpenGL/gl.h>
    #include <OpenGL/glu.h>
#else
    #include <GL/gl.h>
    #include <GL/glu.h>
#endif


#define BUFFER_OFFSET(i) ((char *)nullptr + (i))

#define MAX_PAL_COUNT 32

uint32_t _uPrevVBO = 0xffffffff;
uint32_t _uBlendFunc = 0;
uint32_t _uBindBufferVB = 0;
uint32_t _uBindBufferIB = 0;

uint32_t _uTexCoordStride = 0xffffffff;
uint32_t _uTexCoordOffset = 0xffffffff;
uint32_t _uVB_Stride = 0xffffffff;

bool _bVtxArrayEnabled   = false;
bool _bNrmlArrayEnabled  = false;
bool _bColorArrayEnabled = false;
bool _bTex0ArrayEnabled  = false;

bool _bTexEnabled = false;
TextureHandle _curTex = XL_INVALID_TEXTURE;

bool _bDepthWriteEnabled = true;
bool _bDepthReadEnabled = true;
bool _bCullingEnabled = false;
bool _bAlphaTestEnable = false;
bool _bStencilEnable = false;
bool _bStencilWriteEnabled = false;
bool _bStencilTestEnabled = false;

uint32_t _uAlphaCutoff = 0;
uint32_t _uStencilValue = 0xff;

bool _bFogEnable = false;
float _fFogDensity = 0.25f;
float _fFogEnd = 150.0f;

Matrix *_prevWorldMtxPtr = nullptr;

Vector4 _prevColor(1.0f, 1.0f, 1.0f, 1.0f);

TextureOGL *Driver3D_OGL::m_pCurTex;
uint32_t Driver3D_OGL::s_uColormapID;
static uint32_t *_pCurPal = nullptr;


// Values that control the material properties.
float Noemit[4] = {0.0, 0.0, 0.0, 1.0};

// Lighting values
// float ambientLight[4] = {0.5773502692, 0.5773502692, 0.5773502692, 1.0};

float ambientLight[4] = {0.7773502692, 0.7773502692, 0.7773502692, 1.0};


float Lt0amb[4] = {0.147, 0.12, 0.21, 1};
float Lt0diff[4] = {1.0, 1.0, 1.0, 1.0};
float Lt0spec[4] = {1.0, 1.0, 1.0, 1.0};

GLfloat fogColor[4] = {0.0147, 0.012, 0.021, 1}; //set the for color to grey

float zeroPos[4] = {0, 0, 0, 1};         // Origin (homogeneous representation)

Driver3D_OGL::Driver3D_OGL() : IDriver3D(), m_pRenderCamera(0)
{
    m_uTextureCnt = 0;
    m_pTexArray = nullptr;
    m_pTexIndex = nullptr;
    m_bGouraud  = false;
    // m_Textures.clear();
}

Driver3D_OGL::~Driver3D_OGL()
{
    //delete textures.
    if ( m_uTextureCnt )
    {
        glDeleteTextures( (GLsizei)m_uTextureCnt, (GLuint *)m_Textures );
    }
}

void Driver3D_OGL::ChangeWindowSize(int32_t w, int32_t h)
{
    glViewport(0, 0, w, h);

    m_nWindowWidth  = w;
    m_nWindowHeight = h;
}

bool Driver3D_OGL::Init(int32_t w, int32_t h)
{
    //initialize GLEW for extension loading.
    GLenum err = glewInit();
    if (err != GLEW_OK)
    {
        return false;
    }
    if ( GLEW_VERSION_2_1 == false )
    {
        #ifdef _WINDOWS
            OutputDebugString("OpenGL Version 2.1 is not supported. Aborting XL Engine startup.");
        #endif
        printf("OpenGL Version 2.1 is not supported. Aborting XL Engine startup.");
    }

    glEnable(GL_DEPTH_TEST); /* enable depth buffering */
    glDepthFunc(GL_LEQUAL);  /* pedantic, GL_LESS is the default */
    glClearDepth(1.0);       /* pedantic, 1.0 is the default */
    glClearStencil(0);       /* Clear The Stencil Buffer To 0 */

    /* frame buffer clears should be to black */
    // glClearColor(0.0, 0.0, 0.0, 0.0);

    /* set up projection transform */
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-1.0, 1.0, -1.0, 1.0, 1.0, 10.0);

    /* establish initial viewport */
    glViewport(0, 0, w, h);

    m_nWindowWidth  = w;
    m_nWindowHeight = h;

    s_uColormapID = m_uColormapID;
    //default fog settings
    glFogi(GL_FOG_MODE, GL_LINEAR);
    glFogfv(GL_FOG_COLOR, fogColor);
    glFogf(GL_FOG_DENSITY, 0.3);
    glHint(GL_FOG_HINT, GL_NICEST);
    glFogf(GL_FOG_START, 400);
    glFogf(GL_FOG_END, 800000);


    // lighting

   // // Position the light (before drawing the illuminated objects)
   // glPushMatrix();
   // glRotatef( CurrentAngle, 0.0, 35.0, 1.0 );      // Rotate through animation angle
   // glTranslatef( 7.0, 0.0, 0.0 );               // Translate rotation center to origin
   // glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, Lt0spec);   // Make sphere glow (emissive)
   // // glutSolidSphere(0.3, 5, 5);
   // glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, Noemit);   // Turn off emission
   //
   // if ( LightIsPositional==1 ) {
   //    glLightfv(GL_LIGHT0, GL_POSITION, zeroPos );   // Position is transformed by ModelView matrix
   // }
   // else {
   //    glLightfv(GL_LIGHT0, GL_POSITION, dirI );      // Direction is transformed by ModelView matrix
   // }
   // glPopMatrix();

    // glEnable(GL_LIGHTING);      // Enable lighting calculations
    // glEnable(GL_LIGHT0);      // Turn on light #0.
    glShadeModel (GL_SMOOTH); //set the shader to smooth shader

    // Set global ambient light
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambientLight);

    // Light 0 light values.  Its position is set in drawScene().
    glLightfv(GL_LIGHT0, GL_POSITION, zeroPos );   // Position is transformed by ModelView matrix
    glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION,.75);
    // glLightfv(GL_LIGHT0, GL_AMBIENT, Lt0amb);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, Lt0diff);
    glLightfv(GL_LIGHT0, GL_SPECULAR, Lt0spec);


    glActiveTexture(GL_TEXTURE0);

    glFlush();

    return true;
}

void Driver3D_OGL::EnableDepthWrite(bool bEnable)
{
    if ( _bDepthWriteEnabled != bEnable ) glDepthMask(bEnable ? GL_TRUE : GL_FALSE);
    _bDepthWriteEnabled = bEnable;
}

void Driver3D_OGL::EnableDepthRead(bool bEnable)
{
    if ( _bDepthReadEnabled != bEnable ) glDepthFunc(bEnable ? GL_LEQUAL : GL_ALWAYS);
    _bDepthReadEnabled = bEnable;
}

void Driver3D_OGL::EnableCulling(bool bEnable)
{
    if ( _bCullingEnabled != bEnable )
    {
        if ( bEnable )
            glEnable(GL_CULL_FACE);
        else
            glDisable(GL_CULL_FACE);
    }
    _bCullingEnabled = bEnable;
}

void Driver3D_OGL::EnableAlphaTest(bool bEnable, uint8_t uAlphaCutoff)
{
    if ( bEnable )
    {
        if ( _bAlphaTestEnable != bEnable ) glEnable(GL_ALPHA_TEST);
        if ( _uAlphaCutoff != uAlphaCutoff )
        {
            const float fOO255 = (1.0f/255.0f);

            glAlphaFunc(GL_GREATER, (float)uAlphaCutoff*fOO255);
            _uAlphaCutoff = uAlphaCutoff;
        }
    }
    else if ( _bAlphaTestEnable != bEnable )
    {
        glDisable(GL_ALPHA_TEST);
    }
    _bAlphaTestEnable = bEnable;
}

void Driver3D_OGL::SetBlendMode(uint32_t uMode)
{
    if ( uMode != _uBlendFunc )
    {
        switch(uMode)
        {
            case BLEND_NONE:
                glBlendFunc(GL_ONE, GL_ZERO);
                glDisable( GL_BLEND );
            break;
            case BLEND_ALPHA:
                if ( _uBlendFunc == 0 ) glEnable( GL_BLEND );
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            break;
            case BLEND_ADDITIVE:
                if ( _uBlendFunc == 0 ) glEnable( GL_BLEND );
                glBlendFunc(GL_ONE, GL_ONE);
            break;
            case BLEND_SKY:
                if ( _uBlendFunc == 0 ) glEnable( GL_BLEND );
                glBlendColor(1,0,1,1);
                glBlendFuncSeparate(GL_ONE_MINUS_CONSTANT_COLOR, GL_ONE ,GL_ONE_MINUS_CONSTANT_COLOR, GL_ZERO);
            break;
        };
        _uBlendFunc = uMode;
    }
}

void Driver3D_OGL::SetFogDensity(float fDensity)
{
    // if ( _fFogDensity != fDensity ) { glFogf(GL_FOG_DENSITY, fDensity); _fFogDensity = fDensity; }
}

void Driver3D_OGL::EnableFog(bool bEnable, float fEnd)
{
    // if ( _fFogEnd != fEnd ) { glFogf(GL_FOG_END, fEnd); _fFogEnd = fEnd; }
    if ( bEnable != _bFogEnable )
    {
        if ( bEnable )
            glEnable(GL_FOG);
        else
            glDisable(GL_FOG);
        _bFogEnable = bEnable;
    }
}

void Driver3D_OGL::EnableStencilWriting(bool bEnable, uint32_t uValue)
{
    if ( bEnable )
    {
        if ( _bStencilEnable != bEnable ) glEnable(GL_STENCIL_TEST);
        if ( _bStencilWriteEnabled == false || _uStencilValue != uValue ) { glStencilFunc(GL_ALWAYS, uValue, 0xff); _uStencilValue = uValue; }
        if ( _bStencilWriteEnabled == false )
        {
            glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE); // We Set The Stencil Buffer To 1 Where We Draw

            _bStencilWriteEnabled = true;
            _bStencilTestEnabled = false;
        }
    }
    else
    {
        if ( _bStencilEnable != bEnable ) glDisable(GL_STENCIL_TEST);
    }
    _bStencilEnable = bEnable;
}

void Driver3D_OGL::EnableStencilTesting(bool bEnable)
{
    if ( bEnable )
    {
        if ( _bStencilEnable != bEnable ) glEnable(GL_STENCIL_TEST);
        if ( _bStencilTestEnabled == false )
        {
            glStencilFunc(GL_EQUAL, 1, 1);            // Always Passes, 1 Bit Plane, 1 As Mask
            glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);   // Don't change the stencil buffer.

            _bStencilTestEnabled = true;
            _bStencilWriteEnabled = false;
        }
    }
    else
    {
        if ( _bStencilEnable != bEnable ) glDisable(GL_STENCIL_TEST);
    }
    _bStencilEnable = bEnable;
}

void Driver3D_OGL::Present()
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

            // _pCurPal[p] = (a<<24) | (r<<16) | (g<<8) | b;

            index += 4;
            // printf("%" PRIu32 "\n", _pCurPal[p] );
        }
        // _palIdx = m_uPaletteID;

        // printf("%" PRIu32 "\n", _palIdx);
        //BuildTransTable();
        //if ( m_nBitDepth == 32 )
        // {
            //BuildColorTables_32bpp();
        // }
        // DrawScanline::_pCurPal = _pCurPal;

        // m_bUpdatePal = false;
    }

    RenderOverlays();

    ClearDrawData();
    m_Platform->Present();

}

void Driver3D_OGL::Clear(bool bClearColor)
{
    GLbitfield mask = GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT;
    if ( bClearColor ) mask |= GL_COLOR_BUFFER_BIT;
    glClear( mask );
}

void Driver3D_OGL::SetWorldMatrix(Matrix *pMtx, int32_t worldX, int32_t worldY)
{
    if ( pMtx == nullptr )
    {
        _prevWorldMtxPtr = nullptr;
    }
    else if ( pMtx != _prevWorldMtxPtr )
    {
        m_WorldMtx = *pMtx;
        if ( worldX || worldY )
        {
            m_WorldMtx.m[12] += (worldX - m_pRenderCamera->GetWorldPosX()) * 1024.0f;
            m_WorldMtx.m[13] += (worldY - m_pRenderCamera->GetWorldPosY()) * 1024.0f;
        }
        m_WorldView = m_ViewMtx.MatMul( m_WorldMtx );

        glMatrixMode(GL_MODELVIEW);                         // Select The Modelview Matrix
        glLoadMatrixf( m_WorldView.GetFloatPtr() );

        _prevWorldMtxPtr = pMtx;
    }
}

void Driver3D_OGL::SetViewMatrix(Matrix *pMtx, const Vector3 *pLoc, const Vector3 *pDir)
{
    m_ViewMtx = *pMtx;
    m_Eye = *pLoc;
    m_ViewDir = *pDir;
    _prevWorldMtxPtr = nullptr;    //reset so the world matrix applies the proper transform.
}

void Driver3D_OGL::SetProjMtx(Matrix *pMtx)
{
    glMatrixMode(GL_PROJECTION);                        // Select The Projection Matrix
    glLoadMatrixf( pMtx->GetFloatPtr() );
}

void Driver3D_OGL::SetCamera(Camera *pCamera)
{
    m_pRenderCamera = pCamera;
}

/************** TEXTURE SUPPORT ******************/

//Given color A and background color B, the table contains the closest match for
//A*0.5+B*0.5
void Driver3D_OGL::BuildColorTables_32bpp(int refPalIndex/*=112*/)
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

            // DrawScanline::_colorMap32[0][c + (l<<8)] = Math::clamp(r*c/220, min_r, 255) << 16;
            // DrawScanline::_colorMap32[1][c + (l<<8)] = Math::clamp(g*c/220, min_g, 255) <<  8;
            // DrawScanline::_colorMap32[2][c + (l<<8)] = Math::clamp(b*c/220, min_b, 255);
        }
    }
}

void Driver3D_OGL::SetClearColorFromTex(TextureHandle hTex)
{

    // glClearColor(0.5, 0.7, 0.8, 1.0);
    glClearColor(0.203921, 0.560784, 0.9215686, 1.0);
    // glClearColor(0.41176470588, 0.54509803921, 0.70196078431, 1.0);
    // m_pCurTex = m_Textures[hTex];
    // m_uClearColor = ((uint8_t *)m_pCurTex->m_pData[0])[ (m_pCurTex->m_nHeight-1)*m_pCurTex->m_nWidth ];
}


void Driver3D_OGL::SetTexture(int32_t slot, TextureHandle hTex, uint32_t uFilter, bool bWrap, int32_t frame)
{
    if ( hTex != XL_INVALID_TEXTURE )
    {
        if ( _bTexEnabled == false ) { glEnable(GL_TEXTURE_2D);_bTexEnabled = true; }
        if ( _curTex != hTex )
        {
            glBindTexture(GL_TEXTURE_2D, hTex);
            _curTex = hTex;
        }
        else    //if the texture hasn't changed, return.
        {
            return;
        }
    }
    else
    {
        if ( _curTex != XL_INVALID_TEXTURE ) { glBindTexture(GL_TEXTURE_2D, 0); _curTex = XL_INVALID_TEXTURE; }
        if ( _bTexEnabled ) { glDisable(GL_TEXTURE_2D); _bTexEnabled = false; }

        return;
    }

    //set the filter.
    if ( uFilter == FILTER_POINT )
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    }
    else if ( uFilter == FILTER_NORMAL_NO_MIP )
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, bWrap ? GL_REPEAT : GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, bWrap ? GL_REPEAT : GL_CLAMP);
}

void Driver3D_OGL::SetColor(const Vector4 *pColor)
{
    if ( pColor == nullptr ) pColor = &Vector4::One;

    if ( *pColor != _prevColor )
    {
        glColor4fv(&pColor->x);
        _prevColor = *pColor;
    }
}

TextureHandle Driver3D_OGL::CreateTexture(uint32_t uWidth, uint32_t uHeight, uint32_t uFormat/*=TEX_FORMAT_RGBA8*/, uint8_t *pData/*=nullptr*/, bool bGenMips/*=false*/, int32_t nFrameCnt/*=1*/)
{
    GLint internalFormat=GL_RGBA8;
    GLenum type=GL_UNSIGNED_BYTE;
    GLenum glFormat=GL_RGBA;
    if ( uFormat == TEX_FORMAT_RGBA8 )
    {
        internalFormat = GL_RGBA8;
        type = GL_UNSIGNED_BYTE;
    }
    else if ( uFormat == TEX_FORMAT_RGBA16F )
    {
        internalFormat = GL_RGBA16F;
        type = GL_HALF_FLOAT;
    }
    else if ( uFormat == TEX_FORMAT_RGBA32F )
    {
        internalFormat = GL_RGBA32F;
        type = GL_FLOAT;
    }
    else if ( uFormat == TEX_FORMAT_R32F )
    {
        internalFormat = GL_R32F;
        type = GL_FLOAT;
        glFormat = GL_RED;
    }
    else if ( uFormat == TEX_FORMAT_OTHER)
    {
        internalFormat = GL_RGBA8;
        type = GL_UNSIGNED_BYTE;
    }
    uint32_t uTextureID = m_uTextureCnt;
    m_uTextureCnt++;

    glGenTextures(1, (GLuint *)&m_Textures[uTextureID]);
    glBindTexture(GL_TEXTURE_2D, m_Textures[uTextureID]);

    if ( pData )
    {
        glTexImage2D(GL_TEXTURE_2D, 0, 4, uWidth, uHeight, 0, glFormat, type, pData);
    }

    if ( pData && bGenMips && uFormat == TEX_FORMAT_RGBA8 )
    {
        GenerateMips(uWidth, uHeight, pData);
    }
    else
    {

      for (uint8_t i=0; i<sizeof(pData); i++)
      {
        GenerateMips(uWidth, uHeight, pData);
          // glTexImage2D(GL_TEXTURE_2D, 0, 4, uWidth, uHeight, 0, GL_RGBA8, GL_UNSIGNED_BYTE, &pData[i]);
        // glDisable(GL_COLOR_MATERIAL);
        // glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        // glTexImage2D(GL_TEXTURE_2D, 0, 4, uWidth, uHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, pData);
        // GenerateMips(uWidth, uHeight, pData);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      }

    }

    return m_Textures[uTextureID];
}

void Driver3D_OGL::FreeTexture(TextureHandle hTex)
{
    //remove from the texture list.
    uint32_t uFound = 0xffffffff;
    for (uint32_t i=0; i<m_uTextureCnt; i++)
    {
        if ( m_Textures[i] == hTex )
        {
            uFound = i;
            break;
        }
    }
    if ( uFound < m_uTextureCnt )
    {
        if ( uFound < m_uTextureCnt-1 ) //we need to change the list.
        {
            //just swap the last used texture and the current texture.
            m_Textures[uFound] = m_Textures[m_uTextureCnt-1];
        }

        m_uTextureCnt--;
        //only delete the texture if it's in the list,
        //we don't want to delete it twice by accident.
        glDeleteTextures(1, (GLuint *)&hTex);
    }
}

void Driver3D_OGL::FillTexture(TextureHandle hTex, uint8_t *pData, uint32_t uWidth, uint32_t uHeight, bool bGenMips/*=false*/)
{
    GLint internalFormat=GL_RGBA8;
    GLenum type=GL_UNSIGNED_BYTE;
    GLenum glFormat=GL_RGBA;

    glBindTexture(GL_TEXTURE_2D, hTex);
    glTexImage2D(GL_TEXTURE_2D, 0, 4, uWidth, uHeight, 0, glFormat, type, pData);

    if ( bGenMips )
    {
        GenerateMips(uWidth, uHeight, pData);
    }
}

void Driver3D_OGL::SetExtension_Data(uint32_t uExtension, void *pData0, void *pData1)
{
    if ( uExtension == EXT_TEXTURE_INDEX )
    {
        m_pTexArray = (TextureHandle *)pData0;
        m_pTexIndex = (uint16_t *)pData1;

        // XL_Console::PrintF("^1Error: pData0 to load %s", pData0);
        // XL_Console::PrintF("^1Error: pData1 to load %s", pData1);
        // SetTexture(0, m_pTexArray[m_pTexIndex]);
        // void Driver3D_OGL::SetTexture(int32_t slot, TextureHandle hTex, uint32_t uFilter, bool bWrap, int32_t frame);



    }

    else if ( uExtension == EXT_GOURAUD )
    {
        int32_t enable = *((int32_t *)pData0);
        m_bGouraud = enable ? true : false;
    }
}

void Driver3D_OGL::GenerateMips(uint32_t uWidth, uint32_t uHeight, uint8_t *pData)
{
    int nLevel=0;

    uint8_t *prevLevel = pData;
    uWidth  >>= 1; if ( uWidth < 1 )  uWidth  = 1;
    uHeight >>= 1; if ( uHeight < 1 ) uHeight = 1;
    nLevel++;

    bool b1x2 = false, b2x1 = false;
    while (uWidth >= 1 && uHeight >= 1)
    {
        uint8_t *pNextLevel = (uint8_t *)xlMalloc(uWidth*uHeight*4);

        //something weird is going on if both of these cases are true at the same time.
        assert( b1x2 == false || b2x1 == false );
        for (uint32_t y=0; y<uHeight; y++)
        {
            for (uint32_t x=0; x<uWidth; x++)
            {
                if ( b1x2 )
                {
                    int32_t yy=y*2, xx=x*4;
                    uint8_t r0 = prevLevel[4*yy*(uWidth)+xx+0];
                    uint8_t g0 = prevLevel[4*yy*(uWidth)+xx+1];
                    uint8_t b0 = prevLevel[4*yy*(uWidth)+xx+2];
                    uint8_t a0 = prevLevel[4*yy*(uWidth)+xx+3];

                    uint8_t r2 = prevLevel[4*(yy+1)*(uWidth)+xx+0];
                    uint8_t g2 = prevLevel[4*(yy+1)*(uWidth)+xx+1];
                    uint8_t b2 = prevLevel[4*(yy+1)*(uWidth)+xx+2];
                    uint8_t a2 = prevLevel[4*(yy+1)*(uWidth)+xx+3];

                    pNextLevel[4*y*uWidth+4*x+0] = (r0+r2)>>1;
                    pNextLevel[4*y*uWidth+4*x+1] = (g0+g2)>>1;
                    pNextLevel[4*y*uWidth+4*x+2] = (b0+b2)>>1;
                    pNextLevel[4*y*uWidth+4*x+3] = (a0+a2)>>1;
                }
                else if ( b2x1 )
                {
                    int32_t yy=y, xx=x*2*4;

                    uint8_t r0 = prevLevel[4*yy*(uWidth<<1)+xx+0];
                    uint8_t g0 = prevLevel[4*yy*(uWidth<<1)+xx+1];
                    uint8_t b0 = prevLevel[4*yy*(uWidth<<1)+xx+2];
                    uint8_t a0 = prevLevel[4*yy*(uWidth<<1)+xx+3];

                    uint8_t r1 = prevLevel[4*yy*(uWidth<<1)+xx+4];
                    uint8_t g1 = prevLevel[4*yy*(uWidth<<1)+xx+5];
                    uint8_t b1 = prevLevel[4*yy*(uWidth<<1)+xx+6];
                    uint8_t a1 = prevLevel[4*yy*(uWidth<<1)+xx+7];

                    pNextLevel[4*y*uWidth+4*x+0] = (r0+r1)>>1;
                    pNextLevel[4*y*uWidth+4*x+1] = (g0+g1)>>1;
                    pNextLevel[4*y*uWidth+4*x+2] = (b0+b1)>>1;
                    pNextLevel[4*y*uWidth+4*x+3] = (a0+a1)>>1;
                }
                else
                {
                    int32_t yy=y*2, xx=x*2*4;
                    uint8_t r0 = prevLevel[4*yy*(uWidth<<1)+xx+0];
                    uint8_t g0 = prevLevel[4*yy*(uWidth<<1)+xx+1];
                    uint8_t b0 = prevLevel[4*yy*(uWidth<<1)+xx+2];
                    uint8_t a0 = prevLevel[4*yy*(uWidth<<1)+xx+3];

                    uint8_t r1 = prevLevel[4*yy*(uWidth<<1)+xx+4];
                    uint8_t g1 = prevLevel[4*yy*(uWidth<<1)+xx+5];
                    uint8_t b1 = prevLevel[4*yy*(uWidth<<1)+xx+6];
                    uint8_t a1 = prevLevel[4*yy*(uWidth<<1)+xx+7];

                    uint8_t r2 = prevLevel[4*(yy+1)*(uWidth<<1)+xx+0];
                    uint8_t g2 = prevLevel[4*(yy+1)*(uWidth<<1)+xx+1];
                    uint8_t b2 = prevLevel[4*(yy+1)*(uWidth<<1)+xx+2];
                    uint8_t a2 = prevLevel[4*(yy+1)*(uWidth<<1)+xx+3];

                    uint8_t r3 = prevLevel[4*(yy+1)*(uWidth<<1)+xx+4];
                    uint8_t g3 = prevLevel[4*(yy+1)*(uWidth<<1)+xx+5];
                    uint8_t b3 = prevLevel[4*(yy+1)*(uWidth<<1)+xx+6];
                    uint8_t a3 = prevLevel[4*(yy+1)*(uWidth<<1)+xx+7];

                    pNextLevel[4*y*uWidth+4*x+0] = ( ((r0+r1)>>1) + ((r2+r3)>>1) )>>1;
                    pNextLevel[4*y*uWidth+4*x+1] = ( ((g0+g1)>>1) + ((g2+g3)>>1) )>>1;
                    pNextLevel[4*y*uWidth+4*x+2] = ( ((b0+b1)>>1) + ((b2+b3)>>1) )>>1;
                    pNextLevel[4*y*uWidth+4*x+3] = ( ((a0+a1)>>1) + ((a2+a3)>>1) )>>1;
                }
            }
        }
        glTexImage2D(GL_TEXTURE_2D, nLevel, 4, uWidth, uHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, pNextLevel);

        if ( prevLevel && prevLevel != pData )
        {
            xlFree(prevLevel);
        }
        prevLevel = pNextLevel;

        if ( uWidth <= 1 && uHeight <=1 )
            break;

        uWidth  >>= 1; if ( uWidth < 1 )  { uWidth  = 1; b1x2 = true; }
        uHeight >>= 1; if ( uHeight < 1 ) { uHeight = 1; b2x1 = true; }
        nLevel++;
    };
    if ( prevLevel && prevLevel != pData )
    {
        xlFree(prevLevel);
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

/*************** VBO/IBO Support *****************/
uint32_t Driver3D_OGL::CreateVBO()
{
    uint32_t uVBO_ID;
    glGenBuffers(1, (GLuint *)&uVBO_ID);

    return uVBO_ID;
}

void Driver3D_OGL::AllocVBO_Mem(uint32_t uID, uint32_t uVtxCnt, uint32_t uSize, bool bDynamic)
{
    if ( uID != _uBindBufferVB ) { glBindBuffer(GL_ARRAY_BUFFER, uID); _uBindBufferVB = uID; }
    glBufferData(GL_ARRAY_BUFFER, uSize, nullptr, bDynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    _uBindBufferVB = 0;
}

void Driver3D_OGL::FillVBO(uint32_t uID, void *pData, uint32_t uSize, bool bDynamic)
{
    if ( uID != _uBindBufferVB ) { glBindBuffer(GL_ARRAY_BUFFER, uID); _uBindBufferVB = uID; }

    //cause the "discard" buffer behavior, which is done by first calling glBufferData() with null data.
    glBufferData(GL_ARRAY_BUFFER, uSize, nullptr, bDynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
    //now lock and update the buffer.
    void *pMem = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
    if ( pMem )
    {
        memcpy(pMem, pData, uSize);
    }
    glUnmapBuffer(GL_ARRAY_BUFFER);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    _uBindBufferVB = 0;
}

void Driver3D_OGL::SetVBO(uint32_t uID, uint32_t uStride, uint32_t uVBO_Flags)
{
    if ( _uPrevVBO == uID )
        return;

    bool bNewBindID = false;
    if ( _uBindBufferVB != uID ) { glBindBuffer(GL_ARRAY_BUFFER, uID); _uBindBufferVB = uID; bNewBindID = true; }
    if ( _bVtxArrayEnabled == false ) { glEnableClientState(GL_VERTEX_ARRAY); _bVtxArrayEnabled = true; }
    if ( _uVB_Stride != uStride || bNewBindID )
    {
        glVertexPointer(3, GL_FLOAT, uStride, BUFFER_OFFSET(0));    //The starting point of the VBO, for the vertices
        _uVB_Stride = uStride;
    }

    uint32_t uOffset = 12;
    if ( uVBO_Flags & VBO_HAS_NORMALS )
    {
        if ( _bNrmlArrayEnabled == false ) { glEnableClientState(GL_NORMAL_ARRAY); _bNrmlArrayEnabled = true; }
        glNormalPointer(GL_FLOAT, uStride, BUFFER_OFFSET(uOffset));     //The starting point of normals
        uOffset += 12;
    }
    if ( uVBO_Flags & VBO_HAS_COLORS )
    {
        if ( _bColorArrayEnabled == false ) { glEnableClientState(GL_COLOR_ARRAY); _bColorArrayEnabled = true; }
        glColorPointer(3, GL_FLOAT, uStride, BUFFER_OFFSET(uOffset));    //The starting point of colors
        uOffset += 12;
    }
    if ( uVBO_Flags & VBO_HAS_TEXCOORDS )
    {
        if ( _bTex0ArrayEnabled == false ) { glEnableClientState(GL_TEXTURE_COORD_ARRAY); _bTex0ArrayEnabled = true; }
        if ( _uTexCoordStride != uStride || _uTexCoordOffset != uOffset || bNewBindID )
        {
            glTexCoordPointer(2, GL_FLOAT, uStride, BUFFER_OFFSET(uOffset));    //The starting point of texture coordinates
            _uTexCoordStride = uStride;
            _uTexCoordOffset = uOffset;
        }
        uOffset += 8;
    }
    _uPrevVBO = uID;
}

void Driver3D_OGL::DeleteBuffer(uint32_t uID)
{
    glDeleteBuffers(1, (GLuint *)&uID);
}

uint32_t Driver3D_OGL::CreateIB()
{
    uint32_t uIB_ID;
    glGenBuffers(1, (GLuint *)&uIB_ID);

    return uIB_ID;
}

void Driver3D_OGL::FillIB(uint32_t uID, void *pData, uint32_t uSize, bool bDynamic)
{
    if ( _uBindBufferIB != uID ) { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, uID); _uBindBufferIB = uID; }
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, uSize, pData, bDynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    _uBindBufferIB = 0;
}

void Driver3D_OGL::ClearDrawData()
{
    if ( _uBindBufferVB != 0 ) { glBindBuffer(GL_ARRAY_BUFFER, 0); _uBindBufferVB = 0; }
    if ( _uBindBufferIB != 0 ) { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); _uBindBufferIB = 0; }
    if ( _bVtxArrayEnabled ) { glDisableClientState(GL_VERTEX_ARRAY); _bVtxArrayEnabled = false; }
    if ( _bNrmlArrayEnabled) { glDisableClientState(GL_NORMAL_ARRAY); _bVtxArrayEnabled = false; }
    if ( _bColorArrayEnabled) { glDisableClientState(GL_COLOR_ARRAY); _bColorArrayEnabled = false; }
    if ( _bTex0ArrayEnabled ) { glDisableClientState(GL_TEXTURE_COORD_ARRAY); _bTex0ArrayEnabled = false; }
    SetTexture(0, XL_INVALID_TEXTURE);

    _uPrevVBO = 0xffffffff;
}



void Driver3D_OGL::RenderOverlays()
{
    // for (uint32_t i=0; i<m_uOverlayCount; i++)
    // {
    //     int32_t tw = m_Textures[ m_Overlays[i].hTex ]->m_nWidth;
    //     int32_t th = m_Textures[ m_Overlays[i].hTex ]->m_nHeight;
    //
    //     int32_t tx = m_Overlays[i].x;
    //     int32_t ty = m_Overlays[i].y;
    //     if ( tx >= (int32_t)m_FrameWidth || ty >= (int32_t)m_FrameHeight )
    //         continue;
    //
    //     int32_t tw_Clipped = Math::Min( tx+tw*m_Overlays[i].scale, (int32_t)m_FrameWidth )-tx;
    //     int32_t th_Clipped = Math::Min( ty+th*m_Overlays[i].scale, (int32_t)m_FrameHeight )-ty;
    //     if ( tw_Clipped <= 0 || th_Clipped <= 0 )
    //         continue;
    //
    //     uint32_t *pImage = (uint32_t *)m_Textures[ m_Overlays[i].hTex ]->m_pData[0];
    //     int32_t xOffset = 0;
    //     if ( tx < 0 )
    //          xOffset = -tx;
    //     int32_t yOffset = 0;
    //     if ( ty < 0 )
    //         yOffset = -ty;
    //
    //     int32_t texel_xOffset = xOffset/m_Overlays[i].scale;
    //     int32_t texel_yOffset = yOffset/m_Overlays[i].scale;
    //     int32_t tex_y = texel_yOffset;
    //     int32_t stepsPerTexel = m_Overlays[i].scale;
    //     int32_t stepsY = stepsPerTexel;
    //     for (int32_t y=0; y<th_Clipped-yOffset; y++)
    //     {
    //         uint32_t *pImageV = &pImage[ tex_y*tw + texel_xOffset ];
    //         int32_t fy = m_FrameHeight-(ty+y+yOffset)-1;
    //
    //         // uint32_t *pLine = &m_pFrameBuffer_32bpp[ fy*m_FrameWidth + tx + xOffset ];
    //         int32_t tex_x = 0;
    //         int32_t stepsX = stepsPerTexel;
    //         for (int32_t x=0; x<tw_Clipped-xOffset; x++)
    //         {
    //             // *pLine++ = pImageV[tex_x];
    //
    //             stepsX--;
    //             if ( stepsX == 0 )
    //             {
    //                  tex_x++;
    //                  stepsX = stepsPerTexel;
    //             }
    //         }
    //
    //         stepsY--;
    //         if ( stepsY == 0 )
    //         {
    //              tex_y++;
    //              stepsY = stepsPerTexel;
    //         }
    //     }
    // }
    m_uOverlayCount = 0;
}

//The function assumes a vertex buffer has already been set.
void Driver3D_OGL::RenderIndexedTriangles(IndexBuffer *pIB, int32_t nTriCnt, int32_t startIndex/*=0*/)
{
    uint32_t uIB_ID  = pIB->GetID();
    uint32_t uStride = pIB->GetStride();

    if ( _uBindBufferIB != uIB_ID ) { glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, uIB_ID); _uBindBufferIB = uIB_ID; }
    //To render, we can either use glDrawElements or glDrawRangeElements
    //The is the number of indices. 3 indices needed to make a single triangle
    int idxCnt = nTriCnt*3;

    // for (int t=0, i=startIndex; t<nTriCnt*3; t++, i+=3)
    // {
    //     //EXT_TEXTURE_INDEX
    //     if ( m_pTexArray )
    //     {
    //         // int32_t texIndex = m_pTexArray[m_pTexIndex[t>>1]&0xff];
    //         // assert( (m_pTexIndex[t>>1]&0xff) < (56*4) );
    //
    //         // DrawScanline::_pCurTex = m_Textures[ texIndex ];
    //         // DrawScanline::_texFlip = m_pTexIndex[t>>1]>>8;
    //         // _curTex = texIndex
    //         // SetTexture(0,m_pTexArray[0]);
    //
    //         // int32_t texIndex = m_pTexArray[m_pTexIndex[t]];
    //         // SetTexture(0,texIndex);
    //         // CreateTexture(64, 64, IDriver3D::TEX_FORMAT_RGBA8, m_Textures[texIndex], true, 1);
    //
    //
    //         if(Input::IsKeyDown(XL_I)){
    //
    //           // printf("%s", m_Textures[0]);
    //       }
    //     }
    // }

    glDrawRangeElements(GL_TRIANGLES, 0, idxCnt, idxCnt, (uStride==2)?GL_UNSIGNED_SHORT:GL_UNSIGNED_INT, BUFFER_OFFSET(startIndex*uStride));
}

void Driver3D_OGL::RenderScreenQuad(const Vector4& posScale, const Vector2& uvTop, const Vector2& uvBot, const Vector4& colorTop, const Vector4& colorBot)
{
    glBegin(GL_QUADS);
        if ( _prevColor != colorTop )
        {
            glColor4fv(&colorTop.x);
            _prevColor = colorTop;
        }
        glTexCoord2f(uvTop.x, uvTop.y);
        glVertex3f(posScale.x, posScale.y, -1.0f);
        glTexCoord2f(uvBot.x, uvTop.y);
        glVertex3f(posScale.x+posScale.z, posScale.y, -1.0f);

        if ( _prevColor != colorBot )
        {
            glColor4fv(&colorBot.x);
            _prevColor = colorBot;
        }
        glTexCoord2f(uvBot.x, uvBot.y);
        glVertex3f(posScale.x+posScale.z, posScale.y+posScale.w, -1.0f);
        glTexCoord2f(uvTop.x, uvBot.y);
        glVertex3f(posScale.x, posScale.y+posScale.w, -1.0f);
    glEnd();
}

void Driver3D_OGL::RenderWorldQuad(const Vector3& pos0, const Vector3& pos1, const Vector2& uv0, const Vector2& uv1, const Vector4& color)
{
    if ( _prevColor != color )
    {
        glColor4fv(&color.x);
        _prevColor = color;
    }
    glBegin(GL_QUADS);
        glTexCoord2f(uv0.x, uv0.y);
        glVertex3f(pos0.x, pos0.y, pos0.z);
        glTexCoord2f(uv1.x, uv0.y);
        glVertex3f(pos1.x, pos1.y, pos0.z);

        glTexCoord2f(uv1.x, uv1.y);
        glVertex3f(pos1.x, pos1.y, pos1.z);
        glTexCoord2f(uv0.x, uv1.y);
        glVertex3f(pos0.x, pos0.y, pos1.z);
    glEnd();
}

void Driver3D_OGL::RenderWorldQuad(const Vector3 *posList, const Vector2 *uvList, const Vector4& color, bool bRecieveLighting)
{
    if ( _prevColor != color )
    {
        glColor4fv(&color.x);
        _prevColor = color;
    }
    glBegin(GL_QUADS);
        glTexCoord2f(uvList[0].x, uvList[0].y);
        glVertex3f(posList[0].x, posList[0].y, posList[0].z);
        glTexCoord2f(uvList[1].x, uvList[1].y);
        glVertex3f(posList[1].x, posList[1].y, posList[1].z);

        glTexCoord2f(uvList[2].x, uvList[2].y);
        glVertex3f(posList[2].x, posList[2].y, posList[2].z);
        glTexCoord2f(uvList[3].x, uvList[3].y);
        glVertex3f(posList[3].x, posList[3].y, posList[3].z);
    glEnd();
}

void Driver3D_OGL::RenderWorldQuad(const Vector3 *posList, const Vector2 *uvList, const Vector4 *color, bool bRecieveLighting)
{
    glBegin(GL_QUADS);
        if ( _prevColor != color[0] ) { glColor4fv(&color[0].x); _prevColor = color[0]; }
        glTexCoord2f(uvList[0].x, uvList[0].y);
        glVertex3f(posList[0].x, posList[0].y, posList[0].z);
        if ( _prevColor != color[1] ) { glColor4fv(&color[1].x); _prevColor = color[1]; }
        glTexCoord2f(uvList[1].x, uvList[1].y);
        glVertex3f(posList[1].x, posList[1].y, posList[1].z);

        if ( _prevColor != color[2] ) { glColor4fv(&color[2].x); _prevColor = color[2]; }
        glTexCoord2f(uvList[2].x, uvList[2].y);
        glVertex3f(posList[2].x, posList[2].y, posList[2].z);
        if ( _prevColor != color[3] ) { glColor4fv(&color[3].x); _prevColor = color[3]; }
        glTexCoord2f(uvList[3].x, uvList[3].y);
        glVertex3f(posList[3].x, posList[3].y, posList[3].z);
    glEnd();
}

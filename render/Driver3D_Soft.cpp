#include "Driver3D_Soft.h"
#include <stdlib.h>
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
#include <stdio.h>
#include <malloc.h>
#include <float.h>
#include <cstdint>

#if PLATFORM_WIN	//we have to include Windows.h before gl.h on Windows platforms.
	#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
	// Windows Header Files:
	#include <windows.h>
#endif

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>

#define TEST_COLORMAP 0
#define BUFFER_OFFSET(i) ((char *)NULL + (i))

Texture *Driver3D_Soft::m_pCurTex;
u32 Driver3D_Soft::s_uColormapID;

Matrix *_prevWorldMtxPtr_Soft = NULL;
s32 _prevWorldX = 0;
s32 _prevWorldY = 0;
u32 *m_pFrameBuffer_32bpp;
u8  *m_pFrameBuffer_8bpp;
u16 *m_pDepthBuffer;
u32 m_FrameWidth;
u32 m_FrameHeight;
GLuint m_VideoFrameBuffer;
bool m_bVideoMemAllocated=false;
u8  _fog_table[256];
u8 m_colorRed=0xff, m_colorGrn=0xff, m_colorBlue=0xff;
int m_nMatrixViewKey  = 0;
u32 m_uMatrixWorldKey = 0;
int m_AffineLength;
float m_QuadWidthScale  = 1.0f;
float m_QuadHeightScale = 1.0f;
float m_fTexAnimFrameRate = 8.0f;

s32 _trianglesPerFrame = 0;
static u32 *_pCurPal = NULL;

//#define MAX_FRAMEBUFFER_WIDTH 2048
#define MAX_FRAMEBUFFER_WIDTH 1024

void *aligned_malloc(size_t size, size_t align_size)
{
	char *ptr, *ptr2, *aligned_ptr;
	int align_mask = (int)align_size-1;

	ptr = (char *)malloc( size + align_size + sizeof(int) );
	if ( ptr == NULL)
		return NULL;

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
	m_pCurVBO   = NULL;
	m_pTexArray = NULL;
	m_pTexIndex = NULL;
	m_pRenderCamera = NULL;
	m_pCurPolygonData = NULL;
	m_bBilinear = false;
	m_bGouraud  = false;
	m_uClearColor = 0;
	m_Textures.clear();
	TriangleRasterizer::BuildTables();
	_pCurPal = xlNew u32[256];
	m_fTimer = 0.0f;

	m_uExtensions = EXT_TEXTURE_INDEX | EXT_GOURAUD | EXT_POLYGON_DATA;
}

Driver3D_Soft::~Driver3D_Soft()
{
	//delete textures.
	if ( m_Textures.size() )
	{
		vector<Texture *>::iterator iTex = m_Textures.begin();
		vector<Texture *>::iterator eTex = m_Textures.end();
		for (; iTex != eTex; ++iTex)
		{
			Texture *pTex = *iTex;
			if (!pTex)
				continue;

			for (s32 f=0; f<pTex->m_nFrameCnt; f++)
			{
				free( pTex->m_pData[f] );
			}
			delete pTex;
		}
		m_Textures.clear();
	}

	aligned_free(m_pFrameBuffer_32bpp);
	aligned_free(m_pFrameBuffer_8bpp);
	aligned_free(m_pDepthBuffer);

	xlDelete [] _pCurPal;
}

void Driver3D_Soft::ChangeWindowSize(s32 w, s32 h)
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
	u32 *pImage = (u32 *)malloc(64*64*4);
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

bool Driver3D_Soft::Init(s32 w, s32 h)
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
	if ( EngineSettings::IsFeatureEnabled(EngineSettings::EMULATE_320x200) )
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
		m_pFrameBuffer_32bpp = (u32 *)aligned_malloc( m_FrameWidth*m_FrameHeight*sizeof(u32), 16 );
		m_pFrameBuffer_8bpp  = NULL;
	}
	else
	{
		m_pFrameBuffer_8bpp  = (u8 *)aligned_malloc( m_FrameWidth*m_FrameHeight*sizeof(u8), 16 );
		m_pFrameBuffer_32bpp = (u32 *)aligned_malloc( m_FrameWidth*m_FrameHeight*sizeof(u32), 16 );
	}
	m_pDepthBuffer = (u16 *)aligned_malloc( m_FrameWidth*m_FrameHeight*sizeof(u16), 16 );

	//init fog table
	for (int z=0; z<256; z++)
	{
		float fZ = (float)z / 255.0f;
		float F = 1.0f - fZ;
		if ( F < 0.0f ) F = 0.0f;
		if ( F > 1.0f ) F = 1.0f;
		_fog_table[z] = (u8)(F*255.0f);
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
	DrawScanline::_pCurTex		 = NULL;
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

void Driver3D_Soft::EnableAlphaTest(bool bEnable, u8 uAlphaCutoff)
{
	m_bAlphaTest = bEnable;
}

void Driver3D_Soft::SetBlendMode(u32 uMode)
{
	m_uBlendMode = uMode;
}

void Driver3D_Soft::SetFogDensity(f32 fDensity)
{
}

void Driver3D_Soft::EnableFog(bool bEnable, f32 fEnd)
{
	DrawScanline::_useFog = bEnable;
}

void Driver3D_Soft::EnableStencilWriting(bool bEnable, u32 uValue)
{
}

void Driver3D_Soft::EnableStencilTesting(bool bEnable)
{
}

void Driver3D_Soft::SetBitDepth(s32 bitDepth) 
{ 
	m_nBitDepth = bitDepth; 
	TextureLoader::SetTextureColorDepth(8);// m_nBitDepth );
}

void Driver3D_Soft::SetExtension_Data(u32 uExtension, void *pData0, void *pData1)
{
	if ( uExtension == EXT_TEXTURE_INDEX )
	{
		m_pTexArray = (TextureHandle *)pData0;
		m_pTexIndex = (u16 *)pData1;
	}
	else if ( uExtension == EXT_GOURAUD )
	{
		s32 enable = *((s32 *)pData0);
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
	m_uClearColor = ((u8 *)m_pCurTex->m_pData[0])[ (m_pCurTex->m_nHeight-1)*m_pCurTex->m_nWidth ];
}

void Driver3D_Soft::ConvertFrameBufferTo32bpp(u32 *pal)
{
	u8 *pSource = m_pFrameBuffer_8bpp;
	u32 *pDest  = m_pFrameBuffer_32bpp;

	u32 uPixelCount = m_FrameWidth*m_FrameHeight;
	for (u32 p=0; p<uPixelCount; p++)
	{
		*pDest++ = pal[ *pSource++ ];
	}
}

//Given color A and background color B, the table contains the closest match for
//A*0.5+B*0.5
void Driver3D_Soft::BuildColorTables_32bpp(int refPalIndex/*=112*/)
{
	u8 *pColormap = TextureLoader::GetColormapData(s_uColormapID);

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
				u64 uMinDist2 = 0xffffffffffffffff;
				int blendIndex = -1;
				for (int cBld=0; cBld<256; cBld++)
				{
					s32 rChk = (_pCurPal[cBld]>>16)&0xff;
					s32 gChk = (_pCurPal[cBld]>> 8)&0xff;
					s32 bChk = (_pCurPal[cBld]    )&0xff;

					s32 rDiff = rChk - rBld;
					s32 gDiff = gChk - gBld;
					s32 bDiff = bChk - bBld;

					u64 rDist2 = (u64)(rDiff*rDiff) + (u64)(gDiff*gDiff) + (u64)(bDiff*bDiff);
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
					s32 rChk = (_pCurPal[cBld]>>16)&0xff;
					s32 gChk = (_pCurPal[cBld]>> 8)&0xff;
					s32 bChk = (_pCurPal[cBld]    )&0xff;

					s32 rDiff = rChk - rAdd;
					s32 gDiff = gChk - gAdd;
					s32 bDiff = bChk - bAdd;

					u64 rDist2 = (u64)(rDiff*rDiff) + (u64)(gDiff*gDiff) + (u64)(bDiff*bDiff);
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
			u8 *pColormap  = TextureLoader::GetColormapData(4);
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
		static u32 _palIdx = 0xffffffff;
		if ( _pCurPal == NULL || _palIdx != m_uPaletteID || m_bUpdatePal )
		{
			u8 *pal = TextureLoader::GetPaletteData(m_uPaletteID);
			int index = 0;
			for (u32 p=0; p<256; p++)
			{
				u8 r = pal[ index+0 ];
				u8 g = pal[ index+1 ];
				u8 b = pal[ index+2 ];
				u8 a = pal[ index+3 ];

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
		u8 *pColormap = TextureLoader::GetColormapData(m_uColormapID);
		u32 uColor = _pCurPal ? _pCurPal[ pColormap[0] ] : 0;
		for (u32 i=0; i<m_FrameWidth*m_FrameHeight; i++) { m_pFrameBuffer_32bpp[i] = uColor; }
		//memset(m_pFrameBuffer_32bpp, _pCurPal ? _pCurPal[ pColormap[0] ] : 0, m_FrameWidth*m_FrameHeight*4);
	}
	else if ( bClearColor && m_nBitDepth == 8 )
	{
		u8 *pColormap = TextureLoader::GetColormapData(m_uColormapID);
		u8 uColor = m_uClearColor;
		if ( uColor == 0 )
		{
			uColor = pColormap[0];
		}
		memset(m_pFrameBuffer_8bpp, uColor, m_FrameWidth*m_FrameHeight);
	}

	memset(m_pDepthBuffer, 0xff, m_FrameWidth*m_FrameHeight*2);
}

void Driver3D_Soft::SetWorldMatrix(Matrix *pMtx, s32 worldX, s32 worldY)
{
	if ( pMtx == NULL )
	{
		_prevWorldMtxPtr_Soft = NULL;
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
	_prevWorldMtxPtr_Soft = NULL;	//reset so the world matrix applies the proper transform.

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
void Driver3D_Soft::SetTexture(s32 slot, TextureHandle hTex, u32 uFilter, bool bWrap, s32 frame)
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
		DrawScanline::_uCurFrame = (u32)(m_fTimer * m_fTexAnimFrameRate) % m_pCurTex->m_nFrameCnt;
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

TextureHandle Driver3D_Soft::CreateTexture(u32 uWidth, u32 uHeight, u32 uFormat/*=TEX_FORMAT_RGBA8*/, u8 *pData/*=NULL*/, bool bGenMips/*=false*/, s32 nFrameCnt/*=1*/)
{
	Texture *pTex = new Texture();

	pTex->m_nWidth  = uWidth;
	pTex->m_nHeight = uHeight;
	pTex->m_nMipCnt = 1;
	pTex->m_bIsPow2 = ( Math::RoundNextPow2(uWidth) == uWidth && Math::RoundNextPow2(uHeight) == uHeight ) ? true : false;

	assert( nFrameCnt < 32 );
	for (s32 f=0; f<32; f++)
	{
		pTex->m_pData[f] = NULL;
	}

	s32 nBytesPerPixel = (uFormat!=TEX_FORMAT_FORCE_32bpp) ? (m_nBitDepth>>3) : 4;
	if ( pTex->m_bIsPow2 == false )
	{
		u32 dataSize = (uWidth+2)*(uHeight+2)*nBytesPerPixel;
		m_uDbg_AllocSize = dataSize;
		pTex->m_nFrameCnt = nFrameCnt;
		for (s32 f=0; f<nFrameCnt; f++)
		{
			pTex->m_pData[f] = (u32 *)malloc(dataSize);
			memset(pTex->m_pData[f], 0, dataSize);
		}
	}
	else
	{
		u32 dataSize = uWidth*uHeight*nBytesPerPixel;
		if ( bGenMips )
		{
			u32 w=uWidth>>1, h=uHeight>>1;
			while (w >= 1 && h >= 1)
			{
				dataSize += (w*h);
				w >>= 1;
				h >>= 1;
			};
		}
		m_uDbg_AllocSize = dataSize;
		pTex->m_nFrameCnt = nFrameCnt;
		for (s32 f=0; f<nFrameCnt; f++)
		{
			pTex->m_pData[f] = (u32 *)malloc(dataSize);
		}
	}

	if ( m_nBitDepth == 32 || uFormat==TEX_FORMAT_FORCE_32bpp )
	{
		u32 uFrameOffset = 0;
		for (s32 f=0; f<nFrameCnt; f++)
		{
			s32 yOffs = 0;
			u32 *pDestData = pTex->m_pData[f];
			for (u32 y=0; y<uHeight; y++)
			{
				for (u32 x=0; x<uWidth; x++)
				{
					u8 r = pData[uFrameOffset + ((yOffs+x)<<2) + 0];
					u8 g = pData[uFrameOffset + ((yOffs+x)<<2) + 1];
					u8 b = pData[uFrameOffset + ((yOffs+x)<<2) + 2];
					u8 a = pData[uFrameOffset + ((yOffs+x)<<2) + 3];

					pDestData[yOffs+x] = (a<<24) | (r<<16) | (g<<8) | b;
				}
				yOffs += uWidth;
			}

			uFrameOffset += uWidth*uHeight*4;
		}
	}
	else
	{	
		u32 uFrameOffset = 0;
		for (s32 f=0; f<nFrameCnt; f++)
		{
			memcpy(pTex->m_pData[f], &pData[uFrameOffset], uWidth*uHeight);
			if ( pTex->m_bIsPow2 && bGenMips )
			{
				GenerateMips(uWidth, uHeight, (u8 *)pTex->m_pData[f]);
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

void Driver3D_Soft::FillTexture(TextureHandle hTex, u8 *pData, u32 uWidth, u32 uHeight, bool bGenMips/*=false*/)
{
	if ( hTex < m_Textures.size() )
	{
		s32 nBytesPerPixel = m_nBitDepth>>3;

		Texture *pTex = m_Textures[hTex];
		memcpy(pTex->m_pData[0], pData, uWidth*uHeight*nBytesPerPixel);
	}
}

void Driver3D_Soft::GenerateMips(u32 uWidth, u32 uHeight, u8 *pData)
{
	//assume 8 bit for now, for testing.
	u32 w  = uWidth>>1, h = uHeight>>1;
	u32 pW = uWidth,   pH = uHeight;
	u32 uIndex = uWidth*uHeight;
	u8 *pDstData = &pData[uIndex];
	u8 *pSrcData = pData;
	u32 level = 1;

	u8 *pal = TextureLoader::GetPaletteData(m_uPaletteID);

	while (w > 1 || h > 1)
	{
		for (u32 y=0; y<h; y++)
		{
			for (u32 x=0; x<w; x++)
			{
				u8 r, g, b;
				s32 I[4];
				u8 index[4];

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

				s32 ave = (I[0] + I[1] + I[2] + I[3])>>2;
				I[0] = abs(I[0] - ave);
				I[1] = abs(I[1] - ave);
				I[2] = abs(I[2] - ave);
				I[3] = abs(I[3] - ave);

				u8 finalIdx = 3;
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
u32 Driver3D_Soft::CreateVBO()
{
	u32 uVBO_ID = (u32)m_VBO.size();

	VBO *pVBO = new VBO;
	pVBO->nMatrixViewKey = -1;
	pVBO->uMatrixWorldKey = -1;
	m_VBO.push_back( pVBO );

	return uVBO_ID;
}

void Driver3D_Soft::AllocVBO_Mem(u32 uID, u32 uVtxCnt, u32 uSize, bool bDynamic)
{
	VBO *pVBO = m_VBO[uID];
	if ( pVBO )
	{
		pVBO->nVtxCnt = (int)uVtxCnt;
		pVBO->pSrcVtx = (float *)malloc(uSize);
		pVBO->pVtx	  = (VFmt_Pos_UV *)malloc(uVtxCnt*sizeof(VFmt_Pos_UV));
		pVBO->pVtx_Clipped = (VFmt_Pos_UV_Clip *)malloc(uVtxCnt*sizeof(VFmt_Pos_UV_Clip));
	}
}

void Driver3D_Soft::FillVBO(u32 uID, void *pData, u32 uSize, bool bDynamic)
{
	VBO *pVBO = m_VBO[uID];
	if ( pVBO )
	{
		memcpy(pVBO->pSrcVtx, pData, uSize);
	}
}

void Driver3D_Soft::SetVBO(u32 uID, u32 uStride, u32 uVBO_Flags)
{
	assert(uID < m_VBO.size());
	m_pCurVBO   = m_VBO[uID];
	m_PosStream = m_pCurVBO->pSrcVtx;

	u32 uOffset = 12;
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

void Driver3D_Soft::DeleteBuffer(u32 uID)
{
}

u32 Driver3D_Soft::CreateIB()
{
	IBO *ibo    = new IBO;
	u32 uIBO_ID = (u32)m_IBO.size();
	ibo->uFlags = 0;
	ibo->pRendererData = NULL;
	m_IBO.push_back(ibo);

	return uIBO_ID;
}

void Driver3D_Soft::FillIB(u32 uID, void *pData, u32 uSize, bool bDynamic)
{
	IBO *ibo = m_IBO[uID];
	if ( ibo )
	{
		ibo->pIndices = new u16[uSize>>1];
		memcpy(ibo->pIndices, pData, uSize);
	}
}

void Driver3D_Soft::ResetIBFlags(u32 uID)
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
	m_pCurVBO = NULL;
	m_pCurPolygonData = NULL;
	m_uBlendMode = BLEND_NONE;
	m_bAlphaTest = false;
}

void Driver3D_Soft::LocalToWorld(VBO *pVBO)
{
	VFmt_Pos_UV *pWorld = pVBO->pVtx;

	float *posStream  = m_PosStream;
	float *uvStream   = m_TCoordStream;
	float *nrmlStream = m_NrmlStream;
	u32 offset = 0;
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
void Driver3D_Soft::RenderIndexedTriangles(IndexBuffer *pIB, s32 nTriCnt, s32 startIndex/*=0*/)
{
	if ( m_pCurVBO == NULL )
		return;

	u32 uIB_ID = pIB->GetID();
	IBO *pIbo  = m_IBO[uIB_ID];
	u16 *pIndices = pIbo->pIndices;

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
			u16 *pIdx = &pIbo->pIndices[i];
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

	PolygonData *polyData = NULL;//(PolygonData *)pIbo->pRendererData;
	for (int t=0, i=startIndex; t<nTriCnt; t++, i+=3)
	{
		//EXT_TEXTURE_INDEX
		if ( m_pTexArray )
		{
			s32 texIndex = m_pTexArray[m_pTexIndex[t>>1]&0xff];
			assert( (m_pTexIndex[t>>1]&0xff) < (56*4) );
			DrawScanline::_pCurTex = m_Textures[ texIndex ];
			DrawScanline::_texFlip = m_pTexIndex[t>>1]>>8;
		}

		TriangleRasterizer::DrawClippedNGon_Indexed(this, m_pCurVBO, 3, &pIndices[i], s_uColormapID == 0 ? true : false, alphaMode, NULL);//polyData?&polyData[t]:NULL);
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
	for (u32 i=0; i<m_uOverlayCount; i++)
	{
		s32 tw = m_Textures[ m_Overlays[i].hTex ]->m_nWidth;
		s32 th = m_Textures[ m_Overlays[i].hTex ]->m_nHeight;

		s32 tx = m_Overlays[i].x;
		s32 ty = m_Overlays[i].y;
		if ( tx >= (s32)m_FrameWidth || ty >= (s32)m_FrameHeight )
			continue;

		s32 tw_Clipped = Math::Min( tx+tw*m_Overlays[i].scale, (s32)m_FrameWidth )-tx;
		s32 th_Clipped = Math::Min( ty+th*m_Overlays[i].scale, (s32)m_FrameHeight )-ty;
		if ( tw_Clipped <= 0 || th_Clipped <= 0 )
			continue;

		u32 *pImage = (u32 *)m_Textures[ m_Overlays[i].hTex ]->m_pData[0];
		s32 xOffset = 0;
		if ( tx < 0 )
			 xOffset = -tx;
		s32 yOffset = 0;
		if ( ty < 0 )
			yOffset = -ty;

		s32 texel_xOffset = xOffset/m_Overlays[i].scale;
		s32 texel_yOffset = yOffset/m_Overlays[i].scale;
		s32 tex_y = texel_yOffset;
		s32 stepsPerTexel = m_Overlays[i].scale;
		s32 stepsY = stepsPerTexel;
		for (s32 y=0; y<th_Clipped-yOffset; y++)
		{
			u32 *pImageV = &pImage[ tex_y*tw + texel_xOffset ];
			s32 fy = m_FrameHeight-(ty+y+yOffset)-1;

			u32 *pLine = &m_pFrameBuffer_32bpp[ fy*m_FrameWidth + tx + xOffset ];
			s32 tex_x = 0;
			s32 stepsX = stepsPerTexel;
			for (s32 x=0; x<tw_Clipped-xOffset; x++)
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
	u32 *pImage = (u32 *)m_pCurTex->m_pData[0];
	u32 *pImageV;
	s32 tw = m_pCurTex->m_nWidth;
	s32 th = m_pCurTex->m_nHeight;

	s32 w = (s32)(posScale.z*m_QuadWidthScale);
	s32 h = (s32)(posScale.w*m_QuadHeightScale);
	if ( w > (s32)m_FrameWidth )  w = (s32)m_FrameWidth;
	if ( h > (s32)m_FrameHeight ) h = (s32)m_FrameHeight;

	s32 x = (s32)(posScale.x*m_QuadWidthScale);
	s32 y = (s32)(m_FrameHeight-posScale.y*m_QuadHeightScale-h);
	if ( y < 0 ) y = 0;
	
	s32 dudx = Fixed16_16Math::FloatToFixed( (float)tw / (float)w );
	s32 dvdy = Fixed16_16Math::FloatToFixed( (float)th / (float)h );

	s32 v = Fixed16_16Math::IntToFixed(th)-1;
	if ( m_uBlendMode == BLEND_NONE && m_bAlphaTest == false )
	{
		for (s32 yy=y; yy<y+h; yy++)
		{
			s32 u = 0;
			pImageV = &pImage[ (v>>16)*tw ];
			u32 *pLine = &m_pFrameBuffer_32bpp[ yy*m_FrameWidth + x ];
			for (s32 xx=0; xx<w; xx++)
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
		for (s32 yy=y; yy<y+h; yy++)
		{
			s32 u = 0;
			pImageV = &pImage[ (v>>16)*tw ];
			u32 *pLine = &m_pFrameBuffer_32bpp[ yy*m_FrameWidth + x ];
			for (s32 xx=0; xx<w; xx++)
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
	u8 *pImage = (u8 *)m_pCurTex->m_pData[0];
	u8 *pImageV;
	s32 tw = m_pCurTex->m_nWidth;
	s32 th = m_pCurTex->m_nHeight;

	s32 w = (s32)(posScale.z*m_QuadWidthScale);
	s32 h = (s32)(posScale.w*m_QuadHeightScale);
	if ( w > (s32)m_FrameWidth )  w = (s32)m_FrameWidth;
	if ( h > (s32)m_FrameHeight ) h = (s32)m_FrameHeight;

	s32 x = (s32)(posScale.x*m_QuadWidthScale);
	s32 y = (s32)(m_FrameHeight-posScale.y*m_QuadHeightScale-h);
	if ( y < 0 ) y = 0;
	
	s32 dudx = Fixed16_16Math::FloatToFixed( (float)tw / (float)w );
	s32 dvdy = Fixed16_16Math::FloatToFixed( (float)th / (float)h );

	s32 v = Fixed16_16Math::IntToFixed(th)-1;
	if ( m_uBlendMode == BLEND_NONE && m_bAlphaTest == false )
	{
		for (s32 yy=y, yOffs=yy*m_FrameWidth; yy<y+h; yy++, yOffs+=m_FrameWidth)
		{
			s32 u = 0;
			pImageV = &pImage[ (v>>16)*tw ];
			u8 *pLine = &m_pFrameBuffer_8bpp[ yOffs + x ];
			for (s32 xx=0; xx<w; xx++)
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
		for (s32 yy=y, yOffs=yy*m_FrameWidth; yy<y+h; yy++, yOffs+=m_FrameWidth)
		{
			s32 u = 0;
			pImageV = &pImage[ (v>>16)*tw ];
			u8 *pLine = &m_pFrameBuffer_8bpp[ yOffs + x ];
			for (s32 xx=0; xx<w; xx++)
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

		ibo.pIndices = xlNew u16[4];
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

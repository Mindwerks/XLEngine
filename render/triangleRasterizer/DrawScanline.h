#ifndef DRAW_SCANLINE_H
#define DRAW_SCANLINE_H

#include "VertexFormats.h"
#include "TriangleRasterizer.h"
#include "../../math/Matrix.h"
#include "../../math/Vector2.h"
#include "../../math/Vector4.h"
#include "../../math/FixedPoint.h"
#include <assert.h>

#define DRAW_FUNC(bpp8, pow2, bilinear, alpha, gouraud, persp)	void Draw_##bpp8##_##pow2##_##bilinear##_##alpha##_##gouraud##_##persp##_(const TriGradients& Gradients, TriEdge *pLeft, TriEdge *pRight)

#define GET_DRAW_FUNC(bpp8, pow2, bilinear, alpha, gouraud, persp) Draw_##bpp8##_##pow2##_##bilinear##_##alpha##_##gouraud##_##persp##_

typedef void (*DrawScanlineFunc)(const TriGradients&, TriEdge *, TriEdge *);

namespace DrawScanline
{
	const float _fLightRadius = 64.0f;
	const float _fLightScale = 1.0f / _fLightRadius;
	const float _fLightScale2 = _fLightScale*_fLightScale;
	//Variables are filled in by the software renderer.
	//These are stored locally, rather then going through a pointer,
	//for speed. Scanline drawing is the most intensive part of the rasterization
	//loop, so this code is very... C-like.
	extern int _nAffineLength;
	extern int _nFrameWidth;
	extern int _nFrameHeight;
	extern int _Intensity;
	extern u32 *_pFrameBuffer_32;
	extern u8  *_pFrameBuffer_8;
	extern u8 _uColormapID;
	extern u16 *_pDepthBuffer;
	extern Texture *_pCurTex;
	extern u32 _uCurFrame;
	extern int _texFlip;
	extern int _nLightCnt;
	extern int  _nMip;
	extern bool _useFog;
	extern Vector3 _avLightPos[4];
	extern float _afIntens[4];
	extern Vector3 _N;
	extern float _sqrtTable[65537];
	extern u32 *_pCurPal;
	extern u8 _aTransTable_Blend[256*256];
	extern u8 _aTransTable_Add[256*256];
	extern u32 _colorMap32[3][256*256];
	
	//Get the proper draw function that is specialized to handle the features
	//requested.
	//bpp8 = 1 if 8 bit color.
	//pow2 = 1 if the current texture is a power of 2 in width and height.
	//bilinear = 1 if using the "fast bilinear" mode.
	//alpha = 0 - opaque, 1 - cutout, 2 - alpha blend (50% blend).
	DrawScanlineFunc GetDrawFunc(int bpp8, int pow2, int bilinear, int alpha, int gouraud, int perspCorrect);
};

#endif // DRAW_SCANLINE_H

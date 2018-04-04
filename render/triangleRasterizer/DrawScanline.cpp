#include "../Driver3D_Soft.h"
#include "TriangleRasterizer.h"
#include "DrawScanline.h"
#include "../../Engine.h"
#include "../../math/Math.h"
#include "../../math/FixedPoint.h"
#include "../../fileformats/TextureLoader.h"
#include <stdio.h>
#include <malloc.h>
#include <float.h>

namespace DrawScanline
{
	int _nAffineLength;
	int _nFrameWidth;
	int _nFrameHeight;
	int _Intensity;
	u32 *_pFrameBuffer_32;
	u8  *_pFrameBuffer_8;
	u8 _uColormapID;
	u16 *_pDepthBuffer;
	int  _nMip=0;
	Texture *_pCurTex;
	u32 _uCurFrame = 0;
	int _texFlip = 0;
	int _nLightCnt;
	bool _useFog = true;
	Vector3 _avLightPos[4];
	float _afIntens[4];
	Vector3 _N;
	u32 *_pCurPal;

	float _sqrtTable[65537];
	u8    _aTransTable_Blend[256*256];
	u8	  _aTransTable_Add[256*256];
	u32   _colorMap32[3][256*256];

	static int _ditherTable[]=
	{
		16384,     0, 32768, 49152,
		49152, 32768,     0, 16384
	};

	/********************************************************************************
	 Scanline draw function permutations based on desired features.
	 This allows for the same code to be shared between all scanline drawing functions
	 but specialized for the feature set using defines. This is similar to using defines
	 for shader permutations.
	 "DrawScanline_Base.h" contains the drawing function body.
	 ********************************************************************************/
	//PERSPECTSIVE CORRECT
	#define affine 0

	#define bpp8 0
	#define pow2 0
	#define bilinear 0
	#define alpha 0
	#define gouraud 0
		DRAW_FUNC(0, 0, 0, 0, 0, 0)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 0
	#define pow2 0
	#define bilinear 0
	#define alpha 1
	#define gouraud 0
		DRAW_FUNC(0, 0, 0, 1, 0, 0)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 0
	#define pow2 0
	#define bilinear 0
	#define alpha 2
	#define gouraud 0
		DRAW_FUNC(0, 0, 0, 2, 0, 0)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 0
	#define pow2 0
	#define bilinear 1
	#define alpha 0
	#define gouraud 0
		DRAW_FUNC(0, 0, 1, 0, 0, 0)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 0
	#define pow2 0
	#define bilinear 1
	#define alpha 1
	#define gouraud 0
		DRAW_FUNC(0, 0, 1, 1, 0, 0)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 0
	#define pow2 0
	#define bilinear 1
	#define alpha 2
	#define gouraud 0
		DRAW_FUNC(0, 0, 1, 2, 0, 0)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 0
	#define pow2 1
	#define bilinear 0
	#define alpha 0
	#define gouraud 0
		DRAW_FUNC(0, 1, 0, 0, 0, 0)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 0
	#define pow2 1
	#define bilinear 0
	#define alpha 1
	#define gouraud 0
		DRAW_FUNC(0, 1, 0, 1, 0, 0)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 0
	#define pow2 1
	#define bilinear 0
	#define alpha 2
	#define gouraud 0
		DRAW_FUNC(0, 1, 0, 2, 0, 0)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 0
	#define pow2 1
	#define bilinear 1
	#define alpha 0
	#define gouraud 0
		DRAW_FUNC(0, 1, 1, 0, 0, 0)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 0
	#define pow2 1
	#define bilinear 1
	#define alpha 1
	#define gouraud 0
		DRAW_FUNC(0, 1, 1, 1, 0, 0)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 0
	#define pow2 1
	#define bilinear 1
	#define alpha 2
	#define gouraud 0
		DRAW_FUNC(0, 1, 1, 2, 0, 0)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 1
	#define pow2 0
	#define bilinear 0
	#define alpha 0
	#define gouraud 0
		DRAW_FUNC(1, 0, 0, 0, 0, 0)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 1
	#define pow2 0
	#define bilinear 0
	#define alpha 1
	#define gouraud 0
		DRAW_FUNC(1, 0, 0, 1, 0, 0)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 1
	#define pow2 0
	#define bilinear 0
	#define alpha 2
	#define gouraud 0
		DRAW_FUNC(1, 0, 0, 2, 0, 0)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 1
	#define pow2 0
	#define bilinear 1
	#define alpha 0
	#define gouraud 0
		DRAW_FUNC(1, 0, 1, 0, 0, 0)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 1
	#define pow2 0
	#define bilinear 1
	#define alpha 1
	#define gouraud 0
		DRAW_FUNC(1, 0, 1, 1, 0, 0)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 1
	#define pow2 0
	#define bilinear 1
	#define alpha 2
	#define gouraud 0
		DRAW_FUNC(1, 0, 1, 2, 0, 0)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 1
	#define pow2 1
	#define bilinear 0
	#define alpha 0
	#define gouraud 0
		DRAW_FUNC(1, 1, 0, 0, 0, 0)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 1
	#define pow2 1
	#define bilinear 0
	#define alpha 1
	#define gouraud 0
		DRAW_FUNC(1, 1, 0, 1, 0, 0)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 1
	#define pow2 1
	#define bilinear 0
	#define alpha 2
	#define gouraud 0
		DRAW_FUNC(1, 1, 0, 2, 0, 0)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 1
	#define pow2 1
	#define bilinear 1
	#define alpha 0
	#define gouraud 0
		DRAW_FUNC(1, 1, 1, 0, 0, 0)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 1
	#define pow2 1
	#define bilinear 1
	#define alpha 1
	#define gouraud 0
		DRAW_FUNC(1, 1, 1, 1, 0, 0)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 1
	#define pow2 1
	#define bilinear 1
	#define alpha 2
	#define gouraud 0
		DRAW_FUNC(1, 1, 1, 2, 0, 0)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	//Gouraud
	#define bpp8 0
	#define pow2 0
	#define bilinear 0
	#define alpha 0
	#define gouraud 1
		DRAW_FUNC(0, 0, 0, 0, 1, 0)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 0
	#define pow2 0
	#define bilinear 0
	#define alpha 1
	#define gouraud 1
		DRAW_FUNC(0, 0, 0, 1, 1, 0)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 0
	#define pow2 0
	#define bilinear 0
	#define alpha 2
	#define gouraud 1
		DRAW_FUNC(0, 0, 0, 2, 1, 0)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 0
	#define pow2 0
	#define bilinear 1
	#define alpha 0
	#define gouraud 1
		DRAW_FUNC(0, 0, 1, 0, 1, 0)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 0
	#define pow2 0
	#define bilinear 1
	#define alpha 1
	#define gouraud 1
		DRAW_FUNC(0, 0, 1, 1, 1, 0)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 0
	#define pow2 0
	#define bilinear 1
	#define alpha 2
	#define gouraud 1
		DRAW_FUNC(0, 0, 1, 2, 1, 0)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 0
	#define pow2 1
	#define bilinear 0
	#define alpha 0
	#define gouraud 1
		DRAW_FUNC(0, 1, 0, 0, 1, 0)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 0
	#define pow2 1
	#define bilinear 0
	#define alpha 1
	#define gouraud 1
		DRAW_FUNC(0, 1, 0, 1, 1, 0)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 0
	#define pow2 1
	#define bilinear 0
	#define alpha 2
	#define gouraud 1
		DRAW_FUNC(0, 1, 0, 2, 1, 0)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 0
	#define pow2 1
	#define bilinear 1
	#define alpha 0
	#define gouraud 1
		DRAW_FUNC(0, 1, 1, 0, 1, 0)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 0
	#define pow2 1
	#define bilinear 1
	#define alpha 1
	#define gouraud 1
		DRAW_FUNC(0, 1, 1, 1, 1, 0)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 0
	#define pow2 1
	#define bilinear 1
	#define alpha 2
	#define gouraud 1
		DRAW_FUNC(0, 1, 1, 2, 1, 0)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 1
	#define pow2 0
	#define bilinear 0
	#define alpha 0
	#define gouraud 1
		DRAW_FUNC(1, 0, 0, 0, 1, 0)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 1
	#define pow2 0
	#define bilinear 0
	#define alpha 1
	#define gouraud 1
		DRAW_FUNC(1, 0, 0, 1, 1, 0)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 1
	#define pow2 0
	#define bilinear 0
	#define alpha 2
	#define gouraud 1
		DRAW_FUNC(1, 0, 0, 2, 1, 0)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 1
	#define pow2 0
	#define bilinear 1
	#define alpha 0
	#define gouraud 1
		DRAW_FUNC(1, 0, 1, 0, 1, 0)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 1
	#define pow2 0
	#define bilinear 1
	#define alpha 1
	#define gouraud 1
		DRAW_FUNC(1, 0, 1, 1, 1, 0)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 1
	#define pow2 0
	#define bilinear 1
	#define alpha 2
	#define gouraud 1
		DRAW_FUNC(1, 0, 1, 2, 1, 0)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 1
	#define pow2 1
	#define bilinear 0
	#define alpha 0
	#define gouraud 1
		DRAW_FUNC(1, 1, 0, 0, 1, 0)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 1
	#define pow2 1
	#define bilinear 0
	#define alpha 1
	#define gouraud 1
		DRAW_FUNC(1, 1, 0, 1, 1, 0)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 1
	#define pow2 1
	#define bilinear 0
	#define alpha 2
	#define gouraud 1
		DRAW_FUNC(1, 1, 0, 2, 1, 0)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 1
	#define pow2 1
	#define bilinear 1
	#define alpha 0
	#define gouraud 1
		DRAW_FUNC(1, 1, 1, 0, 1, 0)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 1
	#define pow2 1
	#define bilinear 1
	#define alpha 1
	#define gouraud 1
		DRAW_FUNC(1, 1, 1, 1, 1, 0)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 1
	#define pow2 1
	#define bilinear 1
	#define alpha 2
	#define gouraud 1
		DRAW_FUNC(1, 1, 1, 2, 1, 0)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#undef  affine
	#define affine 1

	#define bpp8 0
	#define pow2 0
	#define bilinear 0
	#define alpha 0
	#define gouraud 0
		DRAW_FUNC(0, 0, 0, 0, 0, 1)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 0
	#define pow2 0
	#define bilinear 0
	#define alpha 1
	#define gouraud 0
		DRAW_FUNC(0, 0, 0, 1, 0, 1)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 0
	#define pow2 0
	#define bilinear 0
	#define alpha 2
	#define gouraud 0
		DRAW_FUNC(0, 0, 0, 2, 0, 1)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 0
	#define pow2 0
	#define bilinear 1
	#define alpha 0
	#define gouraud 0
		DRAW_FUNC(0, 0, 1, 0, 0, 1)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 0
	#define pow2 0
	#define bilinear 1
	#define alpha 1
	#define gouraud 0
		DRAW_FUNC(0, 0, 1, 1, 0, 1)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 0
	#define pow2 0
	#define bilinear 1
	#define alpha 2
	#define gouraud 0
		DRAW_FUNC(0, 0, 1, 2, 0, 1)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 0
	#define pow2 1
	#define bilinear 0
	#define alpha 0
	#define gouraud 0
		DRAW_FUNC(0, 1, 0, 0, 0, 1)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 0
	#define pow2 1
	#define bilinear 0
	#define alpha 1
	#define gouraud 0
		DRAW_FUNC(0, 1, 0, 1, 0, 1)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 0
	#define pow2 1
	#define bilinear 0
	#define alpha 2
	#define gouraud 0
		DRAW_FUNC(0, 1, 0, 2, 0, 1)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 0
	#define pow2 1
	#define bilinear 1
	#define alpha 0
	#define gouraud 0
		DRAW_FUNC(0, 1, 1, 0, 0, 1)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 0
	#define pow2 1
	#define bilinear 1
	#define alpha 1
	#define gouraud 0
		DRAW_FUNC(0, 1, 1, 1, 0, 1)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 0
	#define pow2 1
	#define bilinear 1
	#define alpha 2
	#define gouraud 0
		DRAW_FUNC(0, 1, 1, 2, 0, 1)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 1
	#define pow2 0
	#define bilinear 0
	#define alpha 0
	#define gouraud 0
		DRAW_FUNC(1, 0, 0, 0, 0, 1)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 1
	#define pow2 0
	#define bilinear 0
	#define alpha 1
	#define gouraud 0
		DRAW_FUNC(1, 0, 0, 1, 0, 1)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 1
	#define pow2 0
	#define bilinear 0
	#define alpha 2
	#define gouraud 0
		DRAW_FUNC(1, 0, 0, 2, 0, 1)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 1
	#define pow2 0
	#define bilinear 1
	#define alpha 0
	#define gouraud 0
		DRAW_FUNC(1, 0, 1, 0, 0, 1)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 1
	#define pow2 0
	#define bilinear 1
	#define alpha 1
	#define gouraud 0
		DRAW_FUNC(1, 0, 1, 1, 0, 1)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 1
	#define pow2 0
	#define bilinear 1
	#define alpha 2
	#define gouraud 0
		DRAW_FUNC(1, 0, 1, 2, 0, 1)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 1
	#define pow2 1
	#define bilinear 0
	#define alpha 0
	#define gouraud 0
		DRAW_FUNC(1, 1, 0, 0, 0, 1)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 1
	#define pow2 1
	#define bilinear 0
	#define alpha 1
	#define gouraud 0
		DRAW_FUNC(1, 1, 0, 1, 0, 1)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 1
	#define pow2 1
	#define bilinear 0
	#define alpha 2
	#define gouraud 0
		DRAW_FUNC(1, 1, 0, 2, 0, 1)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 1
	#define pow2 1
	#define bilinear 1
	#define alpha 0
	#define gouraud 0
		DRAW_FUNC(1, 1, 1, 0, 0, 1)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 1
	#define pow2 1
	#define bilinear 1
	#define alpha 1
	#define gouraud 0
		DRAW_FUNC(1, 1, 1, 1, 0, 1)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 1
	#define pow2 1
	#define bilinear 1
	#define alpha 2
	#define gouraud 0
		DRAW_FUNC(1, 1, 1, 2, 0, 1)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	//Gouraud
	#define bpp8 0
	#define pow2 0
	#define bilinear 0
	#define alpha 0
	#define gouraud 1
		DRAW_FUNC(0, 0, 0, 0, 1, 1)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 0
	#define pow2 0
	#define bilinear 0
	#define alpha 1
	#define gouraud 1
		DRAW_FUNC(0, 0, 0, 1, 1, 1)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 0
	#define pow2 0
	#define bilinear 0
	#define alpha 2
	#define gouraud 1
		DRAW_FUNC(0, 0, 0, 2, 1, 1)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 0
	#define pow2 0
	#define bilinear 1
	#define alpha 0
	#define gouraud 1
		DRAW_FUNC(0, 0, 1, 0, 1, 1)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 0
	#define pow2 0
	#define bilinear 1
	#define alpha 1
	#define gouraud 1
		DRAW_FUNC(0, 0, 1, 1, 1, 1)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 0
	#define pow2 0
	#define bilinear 1
	#define alpha 2
	#define gouraud 1
		DRAW_FUNC(0, 0, 1, 2, 1, 1)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 0
	#define pow2 1
	#define bilinear 0
	#define alpha 0
	#define gouraud 1
		DRAW_FUNC(0, 1, 0, 0, 1, 1)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 0
	#define pow2 1
	#define bilinear 0
	#define alpha 1
	#define gouraud 1
		DRAW_FUNC(0, 1, 0, 1, 1, 1)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 0
	#define pow2 1
	#define bilinear 0
	#define alpha 2
	#define gouraud 1
		DRAW_FUNC(0, 1, 0, 2, 1, 1)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 0
	#define pow2 1
	#define bilinear 1
	#define alpha 0
	#define gouraud 1
		DRAW_FUNC(0, 1, 1, 0, 1, 1)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 0
	#define pow2 1
	#define bilinear 1
	#define alpha 1
	#define gouraud 1
		DRAW_FUNC(0, 1, 1, 1, 1, 1)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 0
	#define pow2 1
	#define bilinear 1
	#define alpha 2
	#define gouraud 1
		DRAW_FUNC(0, 1, 1, 2, 1, 1)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 1
	#define pow2 0
	#define bilinear 0
	#define alpha 0
	#define gouraud 1
		DRAW_FUNC(1, 0, 0, 0, 1, 1)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 1
	#define pow2 0
	#define bilinear 0
	#define alpha 1
	#define gouraud 1
		DRAW_FUNC(1, 0, 0, 1, 1, 1)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 1
	#define pow2 0
	#define bilinear 0
	#define alpha 2
	#define gouraud 1
		DRAW_FUNC(1, 0, 0, 2, 1, 1)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 1
	#define pow2 0
	#define bilinear 1
	#define alpha 0
	#define gouraud 1
		DRAW_FUNC(1, 0, 1, 0, 1, 1)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 1
	#define pow2 0
	#define bilinear 1
	#define alpha 1
	#define gouraud 1
		DRAW_FUNC(1, 0, 1, 1, 1, 1)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 1
	#define pow2 0
	#define bilinear 1
	#define alpha 2
	#define gouraud 1
		DRAW_FUNC(1, 0, 1, 2, 1, 1)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 1
	#define pow2 1
	#define bilinear 0
	#define alpha 0
	#define gouraud 1
		DRAW_FUNC(1, 1, 0, 0, 1, 1)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 1
	#define pow2 1
	#define bilinear 0
	#define alpha 1
	#define gouraud 1
		DRAW_FUNC(1, 1, 0, 1, 1, 1)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 1
	#define pow2 1
	#define bilinear 0
	#define alpha 2
	#define gouraud 1
		DRAW_FUNC(1, 1, 0, 2, 1, 1)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 1
	#define pow2 1
	#define bilinear 1
	#define alpha 0
	#define gouraud 1
		DRAW_FUNC(1, 1, 1, 0, 1, 1)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 1
	#define pow2 1
	#define bilinear 1
	#define alpha 1
	#define gouraud 1
		DRAW_FUNC(1, 1, 1, 1, 1, 1)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#define bpp8 1
	#define pow2 1
	#define bilinear 1
	#define alpha 2
	#define gouraud 1
		DRAW_FUNC(1, 1, 1, 2, 1, 1)
		#include "DrawScanline_Base.h"
	#undef bpp8
	#undef pow2
	#undef bilinear
	#undef alpha
	#undef gouraud

	#undef  affine

	/********************************************************************************
	 Given a set of features, return the proper function pointer using the table.
	 ********************************************************************************/
	DrawScanlineFunc drawTable[]=
	{
		GET_DRAW_FUNC(0, 0, 0, 0, 0, 0),
		GET_DRAW_FUNC(0, 0, 0, 1, 0, 0),
		GET_DRAW_FUNC(0, 0, 0, 2, 0, 0),
		GET_DRAW_FUNC(0, 0, 1, 0, 0, 0),
		GET_DRAW_FUNC(0, 0, 1, 1, 0, 0),
		GET_DRAW_FUNC(0, 0, 1, 2, 0, 0),
		GET_DRAW_FUNC(0, 1, 0, 0, 0, 0),
		GET_DRAW_FUNC(0, 1, 0, 1, 0, 0),
		GET_DRAW_FUNC(0, 1, 0, 2, 0, 0),
		GET_DRAW_FUNC(0, 1, 1, 0, 0, 0),
		GET_DRAW_FUNC(0, 1, 1, 1, 0, 0),
		GET_DRAW_FUNC(0, 1, 1, 2, 0, 0),
		GET_DRAW_FUNC(1, 0, 0, 0, 0, 0),
		GET_DRAW_FUNC(1, 0, 0, 1, 0, 0),
		GET_DRAW_FUNC(1, 0, 0, 2, 0, 0),
		GET_DRAW_FUNC(1, 0, 1, 0, 0, 0),
		GET_DRAW_FUNC(1, 0, 1, 1, 0, 0),
		GET_DRAW_FUNC(1, 0, 1, 2, 0, 0),
		GET_DRAW_FUNC(1, 1, 0, 0, 0, 0),
		GET_DRAW_FUNC(1, 1, 0, 1, 0, 0),
		GET_DRAW_FUNC(1, 1, 0, 2, 0, 0),
		GET_DRAW_FUNC(1, 1, 1, 0, 0, 0),
		GET_DRAW_FUNC(1, 1, 1, 1, 0, 0),
		GET_DRAW_FUNC(1, 1, 1, 2, 0, 0),
		GET_DRAW_FUNC(0, 0, 0, 0, 1, 0),
		GET_DRAW_FUNC(0, 0, 0, 1, 1, 0),
		GET_DRAW_FUNC(0, 0, 0, 2, 1, 0),
		GET_DRAW_FUNC(0, 0, 1, 0, 1, 0),
		GET_DRAW_FUNC(0, 0, 1, 1, 1, 0),
		GET_DRAW_FUNC(0, 0, 1, 2, 1, 0),
		GET_DRAW_FUNC(0, 1, 0, 0, 1, 0),
		GET_DRAW_FUNC(0, 1, 0, 1, 1, 0),
		GET_DRAW_FUNC(0, 1, 0, 2, 1, 0),
		GET_DRAW_FUNC(0, 1, 1, 0, 1, 0),
		GET_DRAW_FUNC(0, 1, 1, 1, 1, 0),
		GET_DRAW_FUNC(0, 1, 1, 2, 1, 0),
		GET_DRAW_FUNC(1, 0, 0, 0, 1, 0),
		GET_DRAW_FUNC(1, 0, 0, 1, 1, 0),
		GET_DRAW_FUNC(1, 0, 0, 2, 1, 0),
		GET_DRAW_FUNC(1, 0, 1, 0, 1, 0),
		GET_DRAW_FUNC(1, 0, 1, 1, 1, 0),
		GET_DRAW_FUNC(1, 0, 1, 2, 1, 0),
		GET_DRAW_FUNC(1, 1, 0, 0, 1, 0),
		GET_DRAW_FUNC(1, 1, 0, 1, 1, 0),
		GET_DRAW_FUNC(1, 1, 0, 2, 1, 0),
		GET_DRAW_FUNC(1, 1, 1, 0, 1, 0),
		GET_DRAW_FUNC(1, 1, 1, 1, 1, 0),
		GET_DRAW_FUNC(1, 1, 1, 2, 1, 0),

		GET_DRAW_FUNC(0, 0, 0, 0, 0, 1),
		GET_DRAW_FUNC(0, 0, 0, 1, 0, 1),
		GET_DRAW_FUNC(0, 0, 0, 2, 0, 1),
		GET_DRAW_FUNC(0, 0, 1, 0, 0, 1),
		GET_DRAW_FUNC(0, 0, 1, 1, 0, 1),
		GET_DRAW_FUNC(0, 0, 1, 2, 0, 1),
		GET_DRAW_FUNC(0, 1, 0, 0, 0, 1),
		GET_DRAW_FUNC(0, 1, 0, 1, 0, 1),
		GET_DRAW_FUNC(0, 1, 0, 2, 0, 1),
		GET_DRAW_FUNC(0, 1, 1, 0, 0, 1),
		GET_DRAW_FUNC(0, 1, 1, 1, 0, 1),
		GET_DRAW_FUNC(0, 1, 1, 2, 0, 1),
		GET_DRAW_FUNC(1, 0, 0, 0, 0, 1),
		GET_DRAW_FUNC(1, 0, 0, 1, 0, 1),
		GET_DRAW_FUNC(1, 0, 0, 2, 0, 1),
		GET_DRAW_FUNC(1, 0, 1, 0, 0, 1),
		GET_DRAW_FUNC(1, 0, 1, 1, 0, 1),
		GET_DRAW_FUNC(1, 0, 1, 2, 0, 1),
		GET_DRAW_FUNC(1, 1, 0, 0, 0, 1),
		GET_DRAW_FUNC(1, 1, 0, 1, 0, 1),
		GET_DRAW_FUNC(1, 1, 0, 2, 0, 1),
		GET_DRAW_FUNC(1, 1, 1, 0, 0, 1),
		GET_DRAW_FUNC(1, 1, 1, 1, 0, 1),
		GET_DRAW_FUNC(1, 1, 1, 2, 0, 1),
		GET_DRAW_FUNC(0, 0, 0, 0, 1, 1),
		GET_DRAW_FUNC(0, 0, 0, 1, 1, 1),
		GET_DRAW_FUNC(0, 0, 0, 2, 1, 1),
		GET_DRAW_FUNC(0, 0, 1, 0, 1, 1),
		GET_DRAW_FUNC(0, 0, 1, 1, 1, 1),
		GET_DRAW_FUNC(0, 0, 1, 2, 1, 1),
		GET_DRAW_FUNC(0, 1, 0, 0, 1, 1),
		GET_DRAW_FUNC(0, 1, 0, 1, 1, 1),
		GET_DRAW_FUNC(0, 1, 0, 2, 1, 1),
		GET_DRAW_FUNC(0, 1, 1, 0, 1, 1),
		GET_DRAW_FUNC(0, 1, 1, 1, 1, 1),
		GET_DRAW_FUNC(0, 1, 1, 2, 1, 1),
		GET_DRAW_FUNC(1, 0, 0, 0, 1, 1),
		GET_DRAW_FUNC(1, 0, 0, 1, 1, 1),
		GET_DRAW_FUNC(1, 0, 0, 2, 1, 1),
		GET_DRAW_FUNC(1, 0, 1, 0, 1, 1),
		GET_DRAW_FUNC(1, 0, 1, 1, 1, 1),
		GET_DRAW_FUNC(1, 0, 1, 2, 1, 1),
		GET_DRAW_FUNC(1, 1, 0, 0, 1, 1),
		GET_DRAW_FUNC(1, 1, 0, 1, 1, 1),
		GET_DRAW_FUNC(1, 1, 0, 2, 1, 1),
		GET_DRAW_FUNC(1, 1, 1, 0, 1, 1),
		GET_DRAW_FUNC(1, 1, 1, 1, 1, 1),
		GET_DRAW_FUNC(1, 1, 1, 2, 1, 1)
	};

	DrawScanlineFunc GetDrawFunc(int bpp8, int pow2, int bilinear, int alpha, int gouraud, int perspCorrect)
	{
		int index = alpha + bilinear*3 + pow2*6 + bpp8*12 + gouraud*24 + (!perspCorrect)*48;
		return drawTable[index];
	}
}
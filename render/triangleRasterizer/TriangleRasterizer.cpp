#include "../Driver3D_Soft.h"
#include "TriangleRasterizer.h"
#include "DrawScanline.h"
#include "../../Engine.h"
#include "../../math/Math.h"
#include "../../math/FixedPoint.h"
#include "../../render/Camera.h"
#include <stdio.h>
#include <malloc.h>
#include <float.h>

#if PLATFORM_WIN	//we have to include Windows.h before gl.h on Windows platforms.
	#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
	// Windows Header Files:
	#include <windows.h>
#endif

//defines and static variables.
#define CLAMP_X(x) if (x<0) x=(fixed28_4)0; else if (x>=(fixed28_4)(DrawScanline::_nFrameWidth <<4)) x = (fixed28_4)(DrawScanline::_nFrameWidth <<4)-1
#define CLAMP_Y(y) if (y<0) y=(fixed28_4)0; else if (y>=(fixed28_4)(DrawScanline::_nFrameHeight<<4)) y = (fixed28_4)(DrawScanline::_nFrameHeight<<4)-1

//static float m_fZRange = 8000.0f;	//400
//static float m_fZRange = 128.0f*8000.0f;	//400
//float m_fZRange = 8000.0f;	//400
float m_fZRange = 400.0f;

static const float _AffineSize = 32.0f;

/*********************************************************
 ** Triangle rasterization
 *********************************************************/
void TriGradients::init( const VFmt_Pos_UV_Screen *pVertices, const Texture *pTex )
{
	fixed28_4 x1y0 = Fixed28_4_Math::Mul( pVertices[1].x - pVertices[2].x, pVertices[0].y - pVertices[2].y );
	fixed28_4 x0y1 = Fixed28_4_Math::Mul( pVertices[0].x - pVertices[2].x, pVertices[1].y - pVertices[2].y );
	float OneOverdX =  1.0f / Fixed28_4_Math::FixedToFloat( x1y0 - x0y1 );
	float OneOverdY = -OneOverdX;

	int Counter;
	for (Counter=0; Counter<3; Counter++)
	{
		assert( pVertices[Counter].z >= -m_fZRange && pVertices[Counter].z <= m_fZRange );

		const float OneOverZ = 1.0f / pVertices[Counter].z;
		aOneOverZ[Counter] = OneOverZ;
		aUOverZ[Counter] = pVertices[Counter].u * OneOverZ * (float)(pTex->m_nWidth >>DrawScanline::_nMip);
		aVOverZ[Counter] = pVertices[Counter].v * OneOverZ * (float)(pTex->m_nHeight>>DrawScanline::_nMip);

		//Lighting...
		aLxOverZ[Counter] = pVertices[Counter].lx * OneOverZ;
		aLyOverZ[Counter] = pVertices[Counter].ly * OneOverZ;
		aLzOverZ[Counter] = pVertices[Counter].lz * OneOverZ;

		agOverZ[Counter]  = pVertices[Counter].g  * OneOverZ;
	}

	dOneOverZdX = OneOverdX * ( ((aOneOverZ[1] - aOneOverZ[2]) * Fixed28_4_Math::FixedToFloat(pVertices[0].y - pVertices[2].y)) -
		((aOneOverZ[0] - aOneOverZ[2]) * Fixed28_4_Math::FixedToFloat(pVertices[1].y - pVertices[2].y)) );
	dOneOverZdY = OneOverdY * ( ((aOneOverZ[1] - aOneOverZ[2]) * Fixed28_4_Math::FixedToFloat(pVertices[0].x - pVertices[2].x)) -
		((aOneOverZ[0] - aOneOverZ[2]) * Fixed28_4_Math::FixedToFloat(pVertices[1].x - pVertices[2].x)) );

	dUOverZdX = OneOverdX * ( ((aUOverZ[1] - aUOverZ[2]) * Fixed28_4_Math::FixedToFloat(pVertices[0].y - pVertices[2].y)) -
		((aUOverZ[0] - aUOverZ[2]) * Fixed28_4_Math::FixedToFloat(pVertices[1].y - pVertices[2].y)) );
	dUOverZdY = OneOverdY * ( ((aUOverZ[1] - aUOverZ[2]) * Fixed28_4_Math::FixedToFloat(pVertices[0].x - pVertices[2].x)) -
		((aUOverZ[0] - aUOverZ[2]) * Fixed28_4_Math::FixedToFloat(pVertices[1].x - pVertices[2].x)) );

	dVOverZdX = OneOverdX * ( ((aVOverZ[1] - aVOverZ[2]) * Fixed28_4_Math::FixedToFloat(pVertices[0].y - pVertices[2].y)) -
		((aVOverZ[0] - aVOverZ[2]) * Fixed28_4_Math::FixedToFloat(pVertices[1].y - pVertices[2].y)) );
	dVOverZdY = OneOverdY * ( ((aVOverZ[1] - aVOverZ[2]) * Fixed28_4_Math::FixedToFloat(pVertices[0].x - pVertices[2].x)) -
		((aVOverZ[0] - aVOverZ[2]) * Fixed28_4_Math::FixedToFloat(pVertices[1].x - pVertices[2].x)) );

	dOneOverZdX8 = dOneOverZdX * 8.0f;
	dUOverZdX8   = dUOverZdX   * 8.0f;
	dVOverZdX8   = dVOverZdX   * 8.0f;

	//rounding modifiers.
	const fixed16_16 Half        = 0x8000;
	const fixed16_16 PosModifier = Half;
	const fixed16_16 NegModifier = Half - 1;

	float dUdXIndicator = dUOverZdX*aOneOverZ[0] - aUOverZ[0]*dOneOverZdX;
	if ( dUdXIndicator > 0.0f )
	{
		dUdXModifier = PosModifier;
	}
	else if ( dUdXIndicator < 0.0f )
	{
		dUdXModifier = NegModifier;
	}
	else	//exactly 0
	{
		float dUdYIndicator = dUOverZdY*aOneOverZ[0] - aUOverZ[0]*dOneOverZdY;

		if ( dUdYIndicator >= 0.0f )
		{
			dUdXModifier = PosModifier;
		}
		else
		{
			dUdXModifier = NegModifier;
		}
	}

	float dVdXIndicator = dVOverZdX*aOneOverZ[0] - aVOverZ[0]*dOneOverZdX;
	if ( dVdXIndicator > 0.0f )
	{
		dVdXModifier = PosModifier;
	}
	else if ( dVdXIndicator < 0.0f )
	{
		dVdXModifier = NegModifier;
	}
	else	//exactly 0
	{
		float dVdYIndicator = dVOverZdY*aOneOverZ[0] - aVOverZ[0]*dOneOverZdY;

		if ( dVdYIndicator >= 0.0f )
		{
			dVdXModifier = PosModifier;
		}
		else
		{
			dVdXModifier = NegModifier;
		}
	}

	//Lighting.
	dLxOverZdX = OneOverdX * ( ((aLxOverZ[1] - aLxOverZ[2]) * Fixed28_4_Math::FixedToFloat(pVertices[0].y - pVertices[2].y)) -
		((aLxOverZ[0] - aLxOverZ[2]) * Fixed28_4_Math::FixedToFloat(pVertices[1].y - pVertices[2].y)) );
	dLxOverZdY = OneOverdY * ( ((aLxOverZ[1] - aLxOverZ[2]) * Fixed28_4_Math::FixedToFloat(pVertices[0].x - pVertices[2].x)) -
		((aLxOverZ[0] - aLxOverZ[2]) * Fixed28_4_Math::FixedToFloat(pVertices[1].x - pVertices[2].x)) );

	dLyOverZdX = OneOverdX * ( ((aLyOverZ[1] - aLyOverZ[2]) * Fixed28_4_Math::FixedToFloat(pVertices[0].y - pVertices[2].y)) -
		((aLyOverZ[0] - aLyOverZ[2]) * Fixed28_4_Math::FixedToFloat(pVertices[1].y - pVertices[2].y)) );
	dLyOverZdY = OneOverdY * ( ((aLyOverZ[1] - aLyOverZ[2]) * Fixed28_4_Math::FixedToFloat(pVertices[0].x - pVertices[2].x)) -
		((aLyOverZ[0] - aLyOverZ[2]) * Fixed28_4_Math::FixedToFloat(pVertices[1].x - pVertices[2].x)) );

	dLzOverZdX = OneOverdX * ( ((aLzOverZ[1] - aLzOverZ[2]) * Fixed28_4_Math::FixedToFloat(pVertices[0].y - pVertices[2].y)) -
		((aLzOverZ[0] - aLzOverZ[2]) * Fixed28_4_Math::FixedToFloat(pVertices[1].y - pVertices[2].y)) );
	dLzOverZdY = OneOverdY * ( ((aLzOverZ[1] - aLzOverZ[2]) * Fixed28_4_Math::FixedToFloat(pVertices[0].x - pVertices[2].x)) -
		((aLzOverZ[0] - aLzOverZ[2]) * Fixed28_4_Math::FixedToFloat(pVertices[1].x - pVertices[2].x)) );

	dgOverZdX = OneOverdX * ( ((agOverZ[1] - agOverZ[2]) * Fixed28_4_Math::FixedToFloat(pVertices[0].y - pVertices[2].y)) -
		((agOverZ[0] - agOverZ[2]) * Fixed28_4_Math::FixedToFloat(pVertices[1].y - pVertices[2].y)) );
	dgOverZdY = OneOverdY * ( ((agOverZ[1] - agOverZ[2]) * Fixed28_4_Math::FixedToFloat(pVertices[0].x - pVertices[2].x)) -
		((agOverZ[0] - agOverZ[2]) * Fixed28_4_Math::FixedToFloat(pVertices[1].x - pVertices[2].x)) );

	dLxOverZdX8 = dLxOverZdX * 8.0f;
	dLyOverZdX8 = dLyOverZdX * 8.0f;
	dLzOverZdX8 = dLzOverZdX * 8.0f;
	dgOverZdX8  = dgOverZdX  * 8.0f;

	//rounding modifiers.
	float dLxdXIndicator = dLxOverZdX*aOneOverZ[0] - aLxOverZ[0]*dOneOverZdX;
	if ( dLxdXIndicator > 0.0f )
	{
		dLxdXModifier = PosModifier;
	}
	else if ( dLxdXIndicator < 0.0f )
	{
		dLxdXModifier = NegModifier;
	}
	else	//exactly 0
	{
		float dLxdYIndicator = dLxOverZdY*aOneOverZ[0] - aLxOverZ[0]*dOneOverZdY;

		if ( dLxdYIndicator >= 0.0f )
		{
			dLxdXModifier = PosModifier;
		}
		else
		{
			dLxdXModifier = NegModifier;
		}
	}

	float dLydXIndicator = dLyOverZdX*aOneOverZ[0] - aLyOverZ[0]*dOneOverZdX;
	if ( dLydXIndicator > 0.0f )
	{
		dLydXModifier = PosModifier;
	}
	else if ( dLydXIndicator < 0.0f )
	{
		dLydXModifier = NegModifier;
	}
	else	//exactly 0
	{
		float dLydYIndicator = dLyOverZdY*aOneOverZ[0] - aLyOverZ[0]*dOneOverZdY;

		if ( dLydYIndicator >= 0.0f )
		{
			dLydXModifier = PosModifier;
		}
		else
		{
			dLydXModifier = NegModifier;
		}
	}

	float dLzdXIndicator = dLzOverZdX*aOneOverZ[0] - aLzOverZ[0]*dOneOverZdX;
	if ( dLzdXIndicator > 0.0f )
	{
		dLzdXModifier = PosModifier;
	}
	else if ( dLzdXIndicator < 0.0f )
	{
		dLzdXModifier = NegModifier;
	}
	else	//exactLz 0
	{
		float dLzdYIndicator = dLzOverZdY*aOneOverZ[0] - aLzOverZ[0]*dOneOverZdY;

		if ( dLzdYIndicator >= 0.0f )
		{
			dLzdXModifier = PosModifier;
		}
		else
		{
			dLzdXModifier = NegModifier;
		}
	}

	float dgdXIndicator = dgOverZdX*aOneOverZ[0] - agOverZ[0]*dOneOverZdX;
	if ( dgdXIndicator > 0.0f )
	{
		dgdXModifier = PosModifier;
	}
	else if ( dgdXIndicator < 0.0f )
	{
		dgdXModifier = NegModifier;
	}
	else	//exactLz 0
	{
		float dgdYIndicator = dgOverZdY*aOneOverZ[0] - agOverZ[0]*dOneOverZdY;

		if ( dgdYIndicator >= 0.0f )
		{
			dgdXModifier = PosModifier;
		}
		else
		{
			dgdXModifier = NegModifier;
		}
	}

}

void TriEdge::init(const TriGradients& Gradients, const VFmt_Pos_UV_Screen *pVertices, int Top, int Bottom)
{
	Y = Fixed28_4_Math::Ceil( pVertices[Top].y );
	int YEnd = Fixed28_4_Math::Ceil( pVertices[Bottom].y );
	Height = YEnd - Y;

	if ( Height )
	{
		s32 dN = pVertices[Bottom].y - pVertices[Top].y;
		s32 dM = pVertices[Bottom].x - pVertices[Top].x;

		Denominator = dN*16;
		s32 InitialNumerator = dM*16*Y - dM*pVertices[Top].y + dN*pVertices[Top].x - 1 + Denominator;
		Fixed28_4_Math::FloorDivMod( InitialNumerator, Denominator, X, ErrorTerm);
		Fixed28_4_Math::FloorDivMod( dM*16, Denominator, XStep, Numerator );
		
		float YPreStep = Fixed28_4_Math::FixedToFloat( Y*16 - pVertices[Top].y );
		float XPreStep = Fixed28_4_Math::FixedToFloat( X*16 - pVertices[Top].x );

		OneOverZ = Gradients.aOneOverZ[Top] + YPreStep*Gradients.dOneOverZdY + XPreStep*Gradients.dOneOverZdX;
		OneOverZStep = XStep * Gradients.dOneOverZdX + Gradients.dOneOverZdY;
		OneOverZStepExtra = Gradients.dOneOverZdX;

		UOverZ = Gradients.aUOverZ[Top] + YPreStep*Gradients.dUOverZdY + XPreStep*Gradients.dUOverZdX;
		UOverZStep = XStep * Gradients.dUOverZdX + Gradients.dUOverZdY;
		UOverZStepExtra = Gradients.dUOverZdX;

		VOverZ = Gradients.aVOverZ[Top] + YPreStep*Gradients.dVOverZdY + XPreStep*Gradients.dVOverZdX;
		VOverZStep = XStep * Gradients.dVOverZdX + Gradients.dVOverZdY;
		VOverZStepExtra = Gradients.dVOverZdX;

		//Lighting
		LxOverZ = Gradients.aLxOverZ[Top] + YPreStep*Gradients.dLxOverZdY + XPreStep*Gradients.dLxOverZdX;
		LxOverZStep = XStep * Gradients.dLxOverZdX + Gradients.dLxOverZdY;
		LxOverZStepExtra = Gradients.dLxOverZdX;

		LyOverZ = Gradients.aLyOverZ[Top] + YPreStep*Gradients.dLyOverZdY + XPreStep*Gradients.dLyOverZdX;
		LyOverZStep = XStep * Gradients.dLyOverZdX + Gradients.dLyOverZdY;
		LyOverZStepExtra = Gradients.dLyOverZdX;

		LzOverZ = Gradients.aLzOverZ[Top] + YPreStep*Gradients.dLzOverZdY + XPreStep*Gradients.dLzOverZdX;
		LzOverZStep = XStep * Gradients.dLzOverZdX + Gradients.dLzOverZdY;
		LzOverZStepExtra = Gradients.dLzOverZdX;

		gOverZ = Gradients.agOverZ[Top] + YPreStep*Gradients.dgOverZdY + XPreStep*Gradients.dgOverZdX;
		gOverZStep = XStep * Gradients.dgOverZdX + Gradients.dgOverZdY;
		gOverZStepExtra = Gradients.dgOverZdX;
	}
}

inline int TriEdge::Step()
{
	X += XStep;
	Y++;
	Height--;

	UOverZ += UOverZStep;
	VOverZ += VOverZStep;
	OneOverZ += OneOverZStep;
	//Lighitng
	LxOverZ += LxOverZStep;
	LyOverZ += LyOverZStep;
	LzOverZ += LzOverZStep;
	gOverZ  += gOverZStep;

	ErrorTerm += Numerator;
	if ( ErrorTerm >= Denominator )
	{
		X++;
		ErrorTerm -= Denominator;
		OneOverZ += OneOverZStepExtra;
		UOverZ += UOverZStepExtra;
		VOverZ += VOverZStepExtra;
		//Lighting
		LxOverZ += LxOverZStepExtra;
		LyOverZ += LyOverZStepExtra;
		LzOverZ += LzOverZStepExtra;
		gOverZ  += gOverZStepExtra;
	}

	return Height;
}

namespace TriangleRasterizer
{
	//forward declaration
	void TextureMapTriangle(Driver3D_Soft *pDriver, VFmt_Pos_UV_Screen *pScrVert, int alphaMode, bool bPerspCorrect=true);

	//Implementation
	void BuildTables()
	{
		for (fixed16_16 v=0; v<=65536; v++)
		{
			float f = Fixed16_16Math::FixedToFloat(v);
			f = f > 0 ? sqrtf(f) : 0;
			DrawScanline::_sqrtTable[v] = f;
		}
	}

	void InterpVertex( VFmt_Pos_UV_Clip& dst, float s, const VFmt_Pos_UV_Clip& A, const VFmt_Pos_UV_Clip& B )
	{
		dst.x = A.x + s*(B.x - A.x);
		dst.y = A.y + s*(B.y - A.y);
		dst.z = A.z + s*(B.z - A.z);
		dst.w = A.w + s*(B.w - A.w);

		dst.u = A.u + s*(B.u - A.u);
		dst.v = A.v + s*(B.v - A.v);

		dst.lx = A.lx + s*(B.lx - A.lx);
		dst.ly = A.ly + s*(B.ly - A.ly);
		dst.lz = A.lz + s*(B.lz - A.lz);

		dst.g  = A.g  + s*(B.g - A.g);
	}

	/*
	Store per triangle:
	cenWS
	radius2
	nrmlWS
	*/

	bool ClipPolygon(PolyClipspace& dstA, PolyClipspace& dstB, bool& bPolyClipped)
	{
		bPolyClipped = false;

		//Clip in homogenous clip space.
		//+x
		dstA.vtxCnt = 0;
		for (int e=0; e<dstB.vtxCnt; e++)
		{
			VFmt_Pos_UV_Clip& A = dstB.vtx[e];
			VFmt_Pos_UV_Clip& B = dstB.vtx[(e+1)%dstB.vtxCnt];

			if ( A.w + A.x > 0.0f )
			{
				//add vertex.
				dstA.vtx[ dstA.vtxCnt++ ] = A;
			}
			if ( (A.w + A.x > 0.0f && B.w + B.x <= 0.0f) || (B.w + B.x > 0.0f && A.w + A.x <= 0.0f) )
			{
				//add clipped vertex.
				float s = (A.w + A.x) / ( (A.w + A.x) - (B.w + B.x) );
				InterpVertex( dstA.vtx[ dstA.vtxCnt ], s, A, B );
				dstA.vtxCnt++;
				bPolyClipped = true;
			}
		}
		if ( dstA.vtxCnt < 3 ) return false;

		//-x
		dstB.vtxCnt = 0;
		for (int e=0; e<dstA.vtxCnt; e++)
		{
			VFmt_Pos_UV_Clip& A = dstA.vtx[e];
			VFmt_Pos_UV_Clip& B = dstA.vtx[(e+1)%dstA.vtxCnt];

			if ( A.w - A.x > 0.0f )
			{
				//add vertex.
				dstB.vtx[ dstB.vtxCnt++ ] = A;
			}
			if ( (A.w - A.x > 0.0f && B.w - B.x <= 0.0f) || (B.w - B.x > 0.0f && A.w - A.x <= 0.0f) )
			{
				//add clipped vertex.
				float s = (A.w - A.x) / ( (A.w - A.x) - (B.w - B.x) );
				InterpVertex( dstB.vtx[ dstB.vtxCnt ], s, A, B );
				dstB.vtxCnt++;
				bPolyClipped = true;
			}
		}
		if ( dstB.vtxCnt < 3 ) return false;

		//+y
		dstA.vtxCnt = 0;
		for (int e=0; e<dstB.vtxCnt; e++)
		{
			VFmt_Pos_UV_Clip& A = dstB.vtx[e];
			VFmt_Pos_UV_Clip& B = dstB.vtx[(e+1)%dstB.vtxCnt];

			if ( A.w + A.y > 0.0f )
			{
				//add vertex.
				dstA.vtx[ dstA.vtxCnt++ ] = A;
			}
			if ( (A.w + A.y > 0.0f && B.w + B.y <= 0.0f) || (B.w + B.y > 0.0f && A.w + A.y <= 0.0f) )
			{
				//add clipped vertex.
				float s = (A.w + A.y) / ( (A.w + A.y) - (B.w + B.y) );
				InterpVertex( dstA.vtx[ dstA.vtxCnt ], s, A, B );
				dstA.vtxCnt++;
				bPolyClipped = true;
			}
		}
		if ( dstA.vtxCnt < 3 ) return false;

		//-y
		dstB.vtxCnt = 0;
		for (int e=0; e<dstA.vtxCnt; e++)
		{
			VFmt_Pos_UV_Clip& A = dstA.vtx[e];
			VFmt_Pos_UV_Clip& B = dstA.vtx[(e+1)%dstA.vtxCnt];

			if ( A.w - A.y > 0.0f )
			{
				//add vertex.
				dstB.vtx[ dstB.vtxCnt++ ] = A;
			}
			if ( (A.w - A.y > 0.0f && B.w - B.y <= 0.0f) || (B.w - B.y > 0.0f && A.w - A.y <= 0.0f) )
			{
				//add clipped vertex.
				float s = (A.w - A.y) / ( (A.w - A.y) - (B.w - B.y) );
				InterpVertex( dstB.vtx[ dstB.vtxCnt ], s, A, B );
				dstB.vtxCnt++;
				bPolyClipped = true;
			}
		}
		if ( dstB.vtxCnt < 3 ) return false;

		//near plane.
		float wNear = 0.1f;
		dstA.vtxCnt = 0;
		for (int e=0; e<dstB.vtxCnt; e++)
		{
			VFmt_Pos_UV_Clip& A = dstB.vtx[e];
			VFmt_Pos_UV_Clip& B = dstB.vtx[(e+1)%dstB.vtxCnt];

			if ( A.w > wNear )
			{
				//add vertex.
				dstA.vtx[ dstA.vtxCnt++ ] = A;
			}
			if ( (A.w > wNear && B.w <= wNear) || (B.w > wNear && A.w <= wNear) )
			{
				//add clipped vertex.
				float s = (A.w - wNear) / ( A.w - B.w );
				InterpVertex( dstA.vtx[ dstA.vtxCnt ], s, A, B );
				dstA.vtxCnt++;
				bPolyClipped = true;
			}
		}
		if ( dstA.vtxCnt < 3 ) return false;

		//far plane.
		float wFar = m_fZRange;
		dstB.vtxCnt = 0;
		for (int e=0; e<dstA.vtxCnt; e++)
		{
			VFmt_Pos_UV_Clip& A = dstA.vtx[e];
			VFmt_Pos_UV_Clip& B = dstA.vtx[(e+1)%dstA.vtxCnt];

			if ( A.w < wFar )
			{
				//add vertex.
				dstB.vtx[ dstB.vtxCnt++ ] = A;
			}
			if ( (A.w < wFar && B.w >= wFar) || (B.w < wFar && A.w >= wFar) )
			{
				//add clipped vertex.
				float s = (A.w - wFar) / ( A.w - B.w );
				InterpVertex( dstB.vtx[ dstB.vtxCnt ], s, A, B );
				dstB.vtxCnt++;
			}
		}
		if ( dstB.vtxCnt < 3 ) return false;

		return true;
	}

	float ComputePolygonData(const VFmt_Pos_UV *pWorld, const u16 *pIdx, const PolygonData *polygonData, s32 vCnt, Vector3& cen, Vector3& N)
	{
		float r2;
		if ( polygonData )
		{
			cen = polygonData->cenWS;
			N   = polygonData->nrmlWS;
			r2  = polygonData->radius2_WS;
		}
		else
		{
			cen.Set(0,0,0);
			for (int v=0; v<vCnt; v++)
			{
				Vector3 pos(pWorld[pIdx[v]].pos[0], pWorld[pIdx[v]].pos[1], pWorld[pIdx[v]].pos[2]);
				cen = cen+pos;
			}
			float fOOCnt = 1.0f/(float)vCnt;
			cen.x *= fOOCnt;
			cen.y *= fOOCnt;
			cen.z *= fOOCnt;

			r2 = 0;
			for (int v=0; v<vCnt; v++)
			{
				Vector3 pos(pWorld[pIdx[v]].pos[0], pWorld[pIdx[v]].pos[1], pWorld[pIdx[v]].pos[2]);
				Vector3 offs = pos - cen;
				float d2 = offs.Dot(offs);
				if ( d2 > r2 )
					 r2 = d2;
			}
			
			//light and backface cull the polygon while still in world space.
			//1. Compute the world space normal.
			Vector3 U( pWorld[pIdx[2]].pos[0] - pWorld[pIdx[0]].pos[0], pWorld[pIdx[2]].pos[1] - pWorld[pIdx[0]].pos[1], 
				pWorld[pIdx[2]].pos[2] - pWorld[pIdx[0]].pos[2] );
			Vector3 V( pWorld[pIdx[1]].pos[0] - pWorld[pIdx[0]].pos[0], pWorld[pIdx[1]].pos[1] - pWorld[pIdx[0]].pos[1], 
				pWorld[pIdx[1]].pos[2] - pWorld[pIdx[0]].pos[2] );
			N.Cross(U, V);
			N.Normalize();
		}

		return r2;
	}

	void ClipToScreen(const PolyClipspace& dstB, VFmt_Pos_UV_Screen *aScrVtx, float *xRange, float *yRange, float *uRange, float *vRange)
	{
		float fw = (float)DrawScanline::_nFrameWidth;
		float fh = (float)DrawScanline::_nFrameHeight;
		float oozr = 1.0f / (m_fZRange - 0.1f);

		for (int v=0; v<dstB.vtxCnt; v++)
		{
			assert( fabsf(dstB.vtx[v].w) > 0.0001f );
			float oow = 1.0f / dstB.vtx[v].w;

			float x = fw*( dstB.vtx[v].x*oow*0.5f+0.5f);
			float y = fh*(-dstB.vtx[v].y*oow*0.5f+0.5f);

			//polygon size is screenspace.
			xRange[0] = Math::Min( x, xRange[0] );
			xRange[1] = Math::Max( x, xRange[1] );
			yRange[0] = Math::Min( y, yRange[0] );
			yRange[1] = Math::Max( y, yRange[1] );

			aScrVtx[v].x = Fixed28_4_Math::FloatToFixed( x );
			aScrVtx[v].y = Fixed28_4_Math::FloatToFixed( y );
			aScrVtx[v].z = dstB.vtx[v].z*oozr;// -- why doesn't z/w work?

			float tu = dstB.vtx[v].u;
			float tv = dstB.vtx[v].v;
			if ( DrawScanline::_texFlip&1 )
			{
				tu = -tu;
			}
			if ( DrawScanline::_texFlip&2 )
			{
				tv = -tv;
			}
			if ( DrawScanline::_texFlip&4 )
			{
				float tmp = tu;
				tu = tv;
				tv = tmp;
			}

			uRange[0] = Math::Min( tu, uRange[0] );
			uRange[1] = Math::Max( tu, uRange[1] );
			vRange[0] = Math::Min( tv, vRange[0] );
			vRange[1] = Math::Max( tv, vRange[1] );

			aScrVtx[v].u = tu;
			aScrVtx[v].v = tv;

			aScrVtx[v].lx = dstB.vtx[v].lx;
			aScrVtx[v].ly = dstB.vtx[v].ly;
			aScrVtx[v].lz = dstB.vtx[v].lz;

			aScrVtx[v].g  = dstB.vtx[v].g;
		}
	}

	s32 ComputeMipLevel(float dx, float dy, float du, float dv)
	{
		static bool _bGenLogTable = true;
		static s32 _anLogTable[33*256]; 
		if ( _bGenLogTable )
		{
			for (s32 x=0; x<33*256; x++)
			{
				float f = (float)x/256.0f;
				float l = Math::log2(f);

				_anLogTable[x] = Math::clamp( (s32)l, 0, 5 );
			}

			_bGenLogTable = false;
		}

		float dt = Math::clamp( Math::Max(du, dv), 0.0f, 1.0f );
		//texels per pixel.
		float texelsPerPixel = (float)DrawScanline::_pCurTex->m_nWidth*dt / Math::Min( dx, dy );
		s32 nLogInt = Math::FloatToInt( texelsPerPixel*256.0f );

		return _anLogTable[ Math::clamp(nLogInt, 0, 8192) ];
	}

	void DrawClippedNGon_Indexed(Driver3D_Soft *pDriver, VBO *pVertices, int vCnt, const u16 *pIdx, bool bRecLighting, int alphaMode, PolygonData *polygonData)
	{
		//mipmapping only if forced, off by default.
		bool bComputeMips = pDriver->GetForceMipmapping() && DrawScanline::_pCurTex->m_bIsPow2;

		PolyClipspace dstA, dstB;
		const VFmt_Pos_UV_Clip *pClip = pVertices->pVtx_Clipped;
		const VFmt_Pos_UV *pWorld = pVertices->pVtx;
		Vector3 cen, N;
		float r2 = ComputePolygonData(pWorld, pIdx, polygonData, vCnt, cen, N);

		//2. Backface culling.
		const Vector3 eye = pDriver->GetEyePos();
		const float *pos0 = pWorld[pIdx[0]].pos;
		Vector3 D( eye.x-pos0[0], eye.y-pos0[1], eye.z-pos0[2] );
		if ( D.Dot(N) > 0.0f )
			return;

		//Is the triangle completely outside the frustum?
		s32 frustumTest = pDriver->GetCamera()->SphereInsideFrustum(cen, sqrtf(r2));
		if ( frustumTest == Camera::FRUSTUM_OUT )
			return;

		//copy data for clipping.
		dstB.vtxCnt = vCnt;
		for (int v=0; v<vCnt; v++)
		{
			dstB.vtx[v] = pClip[ pIdx[v] ];
		}

		//3. Now do camera based lighting.
		DrawScanline::_nLightCnt = 0;
		DrawScanline::_N = N;
		if ( bRecLighting )
		{
			const Vector3 viewDir = pDriver->GetViewDir();

			if ( pDriver->GetGouraud() )
			{
				DrawScanline::_Intensity = 196;
			}
			else
			{
				//first check the camera light.
				Vector3 lVec = cen - eye;
				float d2 = lVec.Dot(lVec);
				if ( lVec.Dot(N) > 0 && lVec.Dot(N) < DrawScanline::_fLightRadius && d2 < r2*1.5f + DrawScanline::_fLightRadius*DrawScanline::_fLightRadius )
				{
					DrawScanline::_avLightPos[DrawScanline::_nLightCnt] = eye;
					DrawScanline::_afIntens[DrawScanline::_nLightCnt] = 1.0f;
					DrawScanline::_nLightCnt++;
				}
				//check other lights currently assigned.
				for (int l=0; l<pDriver->GetLightCount() && DrawScanline::_nLightCnt < 4; l++)
				{
					//now check another, fixed light (for now).
					const LightObject **apLights = pDriver->GetLights();
					lVec = cen - apLights[l]->m_vLoc;
					d2 = lVec.Dot(lVec);
					if ( lVec.Dot(N) > 0 && lVec.Dot(N) < DrawScanline::_fLightRadius && d2 < r2*1.5f + DrawScanline::_fLightRadius*DrawScanline::_fLightRadius )
					{
						DrawScanline::_avLightPos[DrawScanline::_nLightCnt] = apLights[l]->m_vLoc;
						DrawScanline::_afIntens[DrawScanline::_nLightCnt] = apLights[l]->m_fIntensity;
						DrawScanline::_nLightCnt++;
					}
				}

				DrawScanline::_Intensity = 64;
			}
		}
		else
		{
			//DrawScanline::_Intensity = 255;
			const Vector3 sunlight(-0.5773502692f, 0.5773502692f, -0.5773502692f);
			float SunI = Math::clamp( N.Dot(sunlight), 0.0f, 1.0f );
			DrawScanline::_Intensity = Math::clamp( (s32)( 255.0f*(pDriver->GetAmbient() + SunI) ), 0, 255 );
		}

		bool bPolyClipped=false;
		if ( frustumTest == Camera::FRUSTUM_INTERSECT )
		{
			if ( !ClipPolygon(dstA, dstB, bPolyClipped) )
				return;
		}

		//dstB is the final polygon...
		//Final clipspace to screenspace conversion...
		VFmt_Pos_UV_Screen aScrVtx[32];
		float xRange[2] = { 4096, 0 };
		float yRange[2] = { 4096, 0 };
		float uRange[2] = { 4096, 0 };
		float vRange[2] = { 4096, 0 };
		ClipToScreen(dstB, aScrVtx, xRange, yRange, uRange, vRange);

		float dx = xRange[1] - xRange[0];
		float dy = yRange[1] - yRange[0];
		if ( dx < 0.00001f && dy < 0.00001f )
			return;

		//compute the proper mip level.
		bool bPerspCorrect = true;
		if ( bComputeMips )
		{
			float du = uRange[1] - uRange[0];
			float dv = vRange[1] - vRange[0];
			DrawScanline::_nMip = ComputeMipLevel( dx, dy, du, dv );

			//distant or small polygons are rendered as affine.
			if ( dx <=_AffineSize && dy <=_AffineSize )
			{
				bPerspCorrect = false;
			}
		}
		else
		{
			DrawScanline::_nMip = 0;
		}

		//now split into triangles and render.
		for (int t=0; t<dstB.vtxCnt-2; t++)
		{
			VFmt_Pos_UV_Screen& A = aScrVtx[0];
			VFmt_Pos_UV_Screen& B = aScrVtx[t+1];
			VFmt_Pos_UV_Screen& C = aScrVtx[t+2];

			assert(A.x>>4 >= 0 && A.x>>4 <= DrawScanline::_nFrameWidth && A.y>>4 >= 0 && A.y>>4 <= DrawScanline::_nFrameHeight);
			assert(B.x>>4 >= 0 && B.x>>4 <= DrawScanline::_nFrameWidth && B.y>>4 >= 0 && B.y>>4 <= DrawScanline::_nFrameHeight);
			assert(C.x>>4 >= 0 && C.x>>4 <= DrawScanline::_nFrameWidth && C.y>>4 >= 0 && C.y>>4 <= DrawScanline::_nFrameHeight);

			VFmt_Pos_UV_Screen vtx[] = { A, B, C };
			//if ( m_bWireFrame )
			//	WireframeTriangle(vtx);
			//else
				TextureMapTriangle(pDriver, vtx, alphaMode, bPerspCorrect);
		}
	}

	void DrawClippedNGon(Driver3D_Soft *pDriver, VBO *pVertices, int vCnt, int offs, int alphaMode)
	{
		PolyClipspace dstA, dstB;
		VFmt_Pos_UV_Clip *pClip = &pVertices->pVtx_Clipped[offs];
		dstB.vtxCnt = vCnt;
		for (int v=0; v<vCnt; v++)
		{
			dstB.vtx[v] = pClip[v];
		}

		//+x
		dstA.vtxCnt = 0;
		for (int e=0; e<dstB.vtxCnt; e++)
		{
			VFmt_Pos_UV_Clip& A = dstB.vtx[e];
			VFmt_Pos_UV_Clip& B = dstB.vtx[(e+1)%dstB.vtxCnt];

			if ( A.w + A.x > 0.0f )
			{
				//add vertex.
				dstA.vtx[ dstA.vtxCnt++ ] = A;
			}
			if ( (A.w + A.x > 0.0f && B.w + B.x <= 0.0f) || (B.w + B.x > 0.0f && A.w + A.x <= 0.0f) )
			{
				//add clipped vertex.
				float s = (A.w + A.x) / ( (A.w + A.x) - (B.w + B.x) );
				InterpVertex( dstA.vtx[ dstA.vtxCnt ], s, A, B );
				dstA.vtxCnt++;
			}
		}

		//-x
		dstB.vtxCnt = 0;
		for (int e=0; e<dstA.vtxCnt; e++)
		{
			VFmt_Pos_UV_Clip& A = dstA.vtx[e];
			VFmt_Pos_UV_Clip& B = dstA.vtx[(e+1)%dstA.vtxCnt];

			if ( A.w - A.x > 0.0f )
			{
				//add vertex.
				dstB.vtx[ dstB.vtxCnt++ ] = A;
			}
			if ( (A.w - A.x > 0.0f && B.w - B.x <= 0.0f) || (B.w - B.x > 0.0f && A.w - A.x <= 0.0f) )
			{
				//add clipped vertex.
				float s = (A.w - A.x) / ( (A.w - A.x) - (B.w - B.x) );
				InterpVertex( dstB.vtx[ dstB.vtxCnt ], s, A, B );
				dstB.vtxCnt++;
			}
		}

		//+y
		dstA.vtxCnt = 0;
		for (int e=0; e<dstB.vtxCnt; e++)
		{
			VFmt_Pos_UV_Clip& A = dstB.vtx[e];
			VFmt_Pos_UV_Clip& B = dstB.vtx[(e+1)%dstB.vtxCnt];

			if ( A.w + A.y > 0.0f )
			{
				//add vertex.
				dstA.vtx[ dstA.vtxCnt++ ] = A;
			}
			if ( (A.w + A.y > 0.0f && B.w + B.y <= 0.0f) || (B.w + B.y > 0.0f && A.w + A.y <= 0.0f) )
			{
				//add clipped vertex.
				float s = (A.w + A.y) / ( (A.w + A.y) - (B.w + B.y) );
				InterpVertex( dstA.vtx[ dstA.vtxCnt ], s, A, B );
				dstA.vtxCnt++;
			}
		}

		//-y
		dstB.vtxCnt = 0;
		for (int e=0; e<dstA.vtxCnt; e++)
		{
			VFmt_Pos_UV_Clip& A = dstA.vtx[e];
			VFmt_Pos_UV_Clip& B = dstA.vtx[(e+1)%dstA.vtxCnt];

			if ( A.w - A.y > 0.0f )
			{
				//add vertex.
				dstB.vtx[ dstB.vtxCnt++ ] = A;
			}
			if ( (A.w - A.y > 0.0f && B.w - B.y <= 0.0f) || (B.w - B.y > 0.0f && A.w - A.y <= 0.0f) )
			{
				//add clipped vertex.
				float s = (A.w - A.y) / ( (A.w - A.y) - (B.w - B.y) );
				InterpVertex( dstB.vtx[ dstB.vtxCnt ], s, A, B );
				dstB.vtxCnt++;
			}
		}

		//near plane.
		float wNear = 0.1f;
		dstA.vtxCnt = 0;
		for (int e=0; e<dstB.vtxCnt; e++)
		{
			VFmt_Pos_UV_Clip& A = dstB.vtx[e];
			VFmt_Pos_UV_Clip& B = dstB.vtx[(e+1)%dstB.vtxCnt];

			if ( A.w > wNear )
			{
				//add vertex.
				dstA.vtx[ dstA.vtxCnt++ ] = A;
			}
			if ( (A.w > wNear && B.w <= wNear) || (B.w > wNear && A.w <= wNear) )
			{
				//add clipped vertex.
				float s = (A.w - wNear) / ( A.w - B.w );
				InterpVertex( dstA.vtx[ dstA.vtxCnt ], s, A, B );
				dstA.vtxCnt++;
			}
		}

		//dstA is the final polygon...
		//Final clipspace to screenspace conversion...
		VFmt_Pos_UV_Screen aScrVtx[32];
		float fw = (float)DrawScanline::_nFrameWidth;
		float fh = (float)DrawScanline::_nFrameHeight;
		float oozr = 1.0f / (m_fZRange - 0.1f);
		for (int v=0; v<dstB.vtxCnt; v++)
		{
			float oow = 1.0f / dstA.vtx[v].w;
			aScrVtx[v].x = Fixed28_4_Math::FloatToFixed( fw*( dstA.vtx[v].x*oow*0.5f+0.5f) );
			aScrVtx[v].y = Fixed28_4_Math::FloatToFixed( fh*(-dstA.vtx[v].y*oow*0.5f+0.5f) );
			aScrVtx[v].z = dstB.vtx[v].z*oozr;// -- why doesn't z/w work?

			aScrVtx[v].u = dstA.vtx[v].u;
			aScrVtx[v].v = dstA.vtx[v].v;
		}

		//now split into triangles and render.
		for (int t=0; t<dstA.vtxCnt-2; t++)
		{
			VFmt_Pos_UV_Screen& A = aScrVtx[0];
			VFmt_Pos_UV_Screen& B = aScrVtx[t+1];
			VFmt_Pos_UV_Screen& C = aScrVtx[t+2];

			VFmt_Pos_UV_Screen vtx[] = { A, B, C };
			//if ( m_bWireFrame )
			//	WireframeTriangle(vtx);
			//else
				TextureMapTriangle(pDriver, vtx, alphaMode);
		}
	}

	void TextureMapTriangle(Driver3D_Soft *pDriver, VFmt_Pos_UV_Screen *pScrVert, int alphaMode, bool bPerspCorrect)
	{
		int Top, Middle, Bottom, MiddleForCompare, BottomForCompare;

		//hackity hack... clamp x & y to avoid crashes...
		CLAMP_X(pScrVert[0].x); CLAMP_Y(pScrVert[0].y);
		CLAMP_X(pScrVert[1].x); CLAMP_Y(pScrVert[1].y);
		CLAMP_X(pScrVert[2].x); CLAMP_Y(pScrVert[2].y);
		fixed28_4 y0 = pScrVert[0].y, y1 = pScrVert[1].y, y2 = pScrVert[2].y;

		//sort vertices by y.
		if ( y0 < y1 )
		{
			if ( y2 < y0 )
			{
				Top = 2; Middle = 0; Bottom = 1;
				MiddleForCompare = 0; BottomForCompare = 1;
			}
			else
			{
				Top = 0;
				if ( y1 < y2 )
				{
					Middle = 1; Bottom = 2;
					MiddleForCompare = 1; BottomForCompare = 2;
				}
				else
				{
					Middle = 2; Bottom = 1;
					MiddleForCompare = 2; BottomForCompare = 1;
				}
			}
		}
		else
		{
			if ( y2 < y1 )
			{
				Top = 2; Middle = 1; Bottom = 0;
				MiddleForCompare = 1; BottomForCompare = 0;
			}
			else
			{
				Top = 1;
				if ( y0 < y2 )
				{
					Middle = 0; Bottom = 2;
					MiddleForCompare = 3; BottomForCompare = 2;
				}
				else
				{
					Middle = 2; Bottom = 0;
					MiddleForCompare = 2; BottomForCompare = 3;
				}
			}
		}

		DrawScanlineFunc pDrawScanline = DrawScanline::GetDrawFunc(pDriver->GetBitDepth()==8?1:0, pDriver->GetCurTex()->m_bIsPow2?1:0, pDriver->GetBilinear()?1:0, alphaMode, pDriver->GetGouraud()?1:0, bPerspCorrect?1:0);

		//setup the triangle gradients.
		TriGradients Gradients;
		Gradients.init( pScrVert, pDriver->GetCurTex() );
		
		TriEdge TopToBottom, TopToMiddle, MiddleToBottom;
		TriEdge *pLeft, *pRight;
		int MiddleIsLeft;

		TopToBottom.init(Gradients, pScrVert, Top, Bottom);
		TopToMiddle.init(Gradients, pScrVert, Top, Middle);
		MiddleToBottom.init(Gradients, pScrVert, Middle, Bottom);

		if ( BottomForCompare > MiddleForCompare )
		{
			MiddleIsLeft = 0;
			pLeft  = &TopToBottom;
			pRight = &TopToMiddle;
		}
		else
		{
			MiddleIsLeft = 1;
			pLeft  = &TopToMiddle;
			pRight = &TopToBottom;
		}

		int Height = TopToMiddle.Height;
		while (Height--)
		{
			pDrawScanline( Gradients, pLeft, pRight );
			TopToMiddle.Step();
			TopToBottom.Step();
		};

		if ( MiddleIsLeft )
		{
			pLeft  = &MiddleToBottom;
			pRight = &TopToBottom;
		}
		else
		{
			pLeft  = &TopToBottom;
			pRight = &MiddleToBottom;
		}
		Height = MiddleToBottom.Height;
		while (Height--)
		{
			pDrawScanline( Gradients, pLeft, pRight );
			MiddleToBottom.Step();
			TopToBottom.Step();
		};
	}
};
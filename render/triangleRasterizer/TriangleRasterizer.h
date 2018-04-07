#ifndef TRIANGLE_RASTERIZER_H
#define TRIANGLE_RASTERIZER_H

#include "VertexFormats.h"
#include "../../math/Matrix.h"
#include "../../math/Vector2.h"
#include "../../math/Vector4.h"
#include "../../math/FixedPoint.h"
#include <assert.h>

struct Texture;
struct PolygonData;

struct PolyClipspace
{
	int16_t vtxCnt;
	VFmt_Pos_UV_Clip vtx[32];
};

struct TriGradients
{
	void init( const VFmt_Pos_UV_Screen *pVertices, const Texture *pTex );
	float aOneOverZ[3];
	float aUOverZ[3];
	float aVOverZ[3];

	float dOneOverZdX, dOneOverZdX8, dOneOverZdY;
	float dUOverZdX, dUOverZdX8, dUOverZdY;
	float dVOverZdX, dVOverZdX8, dVOverZdY;
	fixed16_16 dUdXModifier;
	fixed16_16 dVdXModifier;

	//Lighting
	float aLxOverZ[3];
	float aLyOverZ[3];
	float aLzOverZ[3];
	float agOverZ[3];

	float dLxOverZdX, dLxOverZdX8, dLxOverZdY;
	float dLyOverZdX, dLyOverZdX8, dLyOverZdY;
	float dLzOverZdX, dLzOverZdX8, dLzOverZdY;
	float dgOverZdX,  dgOverZdX8,  dgOverZdY;
	fixed16_16 dLxdXModifier;
	fixed16_16 dLydXModifier;
	fixed16_16 dLzdXModifier;
	fixed16_16 dgdXModifier;
};

struct TriEdge
{
	void init(const TriGradients& Gradients, const VFmt_Pos_UV_Screen *pVertices, int Top, int Bottom);
	inline int Step();

	int32_t X, XStep, Numerator, Denominator;	//DDA info for X.
	int32_t ErrorTerm;
	int Y, Height;	//Current Y and vertical count.
	float OneOverZ, OneOverZStep, OneOverZStepExtra;	// 1/z and step.
	float UOverZ, UOverZStep, UOverZStepExtra;			// u/z and step.
	float VOverZ, VOverZStep, VOverZStepExtra;			// v/z and step.

	//lighting
	float LxOverZ, LxOverZStep, LxOverZStepExtra;			// u/z and step.
	float LyOverZ, LyOverZStep, LyOverZStepExtra;			// v/z and step.
	float LzOverZ, LzOverZStep, LzOverZStepExtra;			// v/z and step.
	float gOverZ, gOverZStep, gOverZStepExtra;			// v/z and step.
};

namespace TriangleRasterizer
{
	void BuildTables();
	void DrawClippedNGon_Indexed(Driver3D_Soft *pDriver, VBO *pVertices, int vCnt, const uint16_t *pIdx, bool bRecLighting, int alphaMode, PolygonData *polygonData=NULL);
	void DrawClippedNGon(Driver3D_Soft *pDriver, VBO *pVertices, int vCnt, int offs, int alphaMode);
};

#endif // TRIANGLE_RASTERIZER_H

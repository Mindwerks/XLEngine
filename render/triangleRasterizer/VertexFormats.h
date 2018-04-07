#ifndef VERTEX_FORMATS_H
#define VERTEX_FORMATS_H

#include "../../math/FixedPoint.h"

#define VFMT_POS_UV      0
#define VFMT_POS_NRML_UV 1
#define VFMT_POS_CLR_UV  2

struct VFmt_Pos_UV
{
	float pos[3];
	float uv[2];
	float g;
};

struct VFmt_Pos_UV_Clip
{
	float x, y, z, w;
	float u, v;
	float lx, ly, lz;
	float g;
};

struct VFmt_Pos_UV_Screen
{
	fixed28_4 x, y;
	float z, u, v;
	float lx, ly, lz;
	float g;
};

struct VBO
{
	int nVtxCnt;
	float *pSrcVtx;
	VFmt_Pos_UV *pVtx;
	VFmt_Pos_UV_Clip *pVtx_Clipped;

	int nMatrixViewKey;
	uint32_t uMatrixWorldKey;
	uint32_t uFlags;
};

struct IBO
{
	uint16_t *pIndices;
	void *pRendererData;
	uint32_t uFlags;
};

#endif // VERTEX_FORMATS_H

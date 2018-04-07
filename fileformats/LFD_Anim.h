#ifndef LFD_ANIM_H
#define LFD_ANIM_H

#include <math.h>
#include "../CommonTypes.h"

class IDriver3D;

struct RGB_Color
{
	uint8_t r;
	uint8_t g;
	uint8_t b;
};

struct PLTT_File
{
	uint8_t First;
	uint8_t Last;
	RGB_Color colors[256];
	int32_t num_colors;
};

class LFD_Anim
{
public:
	LFD_Anim(IDriver3D *pDriver);
	~LFD_Anim(void);
	void Destroy();

	static bool LoadPLTT(char *pData, int32_t len);
	static bool SetPLTT(PLTT_File *pal, bool bCopyFullPal=true);
	static PLTT_File *GetPLTT();

	bool Load(char *pData, int32_t len, bool bUseProperOffs=false);
	bool LoadDELT(char *pData, int32_t len, bool bUseProperOffs=false);
	void SetScale(f32 sx=1.0f, f32 sy=1.0f) { m_fScaleX = sx; m_fScaleY = sy; }
	void Render(int32_t frame, f32 x=0.0f, f32 y=0.0f, f32 maxX=1.0f, f32 minY=0.0f, f32 dU=0.0f, f32 dV=0.0f, bool bDistort=false);
	void GetFrameExtents(int32_t frame, f32 x, f32 y, int32_t& frameX0, int32_t& frameY0, int32_t& frameWidth, int32_t& frameHeight);

	void SetOffsScale(float sx, float sy);
	int32_t GetFrameCount() { return m_nNumDelts; }
private:
	TextureHandle m_hTex[256];
	f32 m_Width[256];
	f32 m_Height[256];
	f32 m_OffsX[256];
	f32 m_OffsY[256];

	f32 m_u1[256];
	f32 m_v1[256];

	f32 m_fScaleX, m_fScaleY;
	int32_t m_nNumDelts;

	IDriver3D *m_pDriver;

	static PLTT_File m_PalFile;
};

#endif //LFD_ANIM_H
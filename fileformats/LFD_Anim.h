#ifndef LFD_ANIM_H
#define LFD_ANIM_H

#include <math.h>
#include "../CommonTypes.h"

class IDriver3D;

struct RGB_Color
{
	u8 r;
	u8 g;
	u8 b;
};

struct PLTT_File
{
	u8 First;
	u8 Last;
	RGB_Color colors[256];
	s32 num_colors;
};

class LFD_Anim
{
public:
	LFD_Anim(IDriver3D *pDriver);
	~LFD_Anim(void);
	void Destroy();

	static bool LoadPLTT(char *pData, s32 len);
	static bool SetPLTT(PLTT_File *pal, bool bCopyFullPal=true);
	static PLTT_File *GetPLTT();

	bool Load(char *pData, s32 len, bool bUseProperOffs=false);
	bool LoadDELT(char *pData, s32 len, bool bUseProperOffs=false);
	void SetScale(f32 sx=1.0f, f32 sy=1.0f) { m_fScaleX = sx; m_fScaleY = sy; }
	void Render(s32 frame, f32 x=0.0f, f32 y=0.0f, f32 maxX=1.0f, f32 minY=0.0f, f32 dU=0.0f, f32 dV=0.0f, bool bDistort=false);
	void GetFrameExtents(s32 frame, f32 x, f32 y, s32& frameX0, s32& frameY0, s32& frameWidth, s32& frameHeight);

	void SetOffsScale(float sx, float sy);
	s32 GetFrameCount() { return m_nNumDelts; }
private:
	TextureHandle m_hTex[256];
	f32 m_Width[256];
	f32 m_Height[256];
	f32 m_OffsX[256];
	f32 m_OffsY[256];

	f32 m_u1[256];
	f32 m_v1[256];

	f32 m_fScaleX, m_fScaleY;
	s32 m_nNumDelts;

	IDriver3D *m_pDriver;

	static PLTT_File m_PalFile;
};

#endif //LFD_ANIM_H
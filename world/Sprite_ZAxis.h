#ifndef SPRITE_ZAXIS_H
#define SPRITE_ZAXIS_H

#include "../CommonTypes.h"
#include "RenderComponent.h"
#include <vector>

using namespace std;

class IDriver3D;
class Object;

class Sprite_ZAxis : public RenderComponent
{
public:
	Sprite_ZAxis();
	virtual ~Sprite_ZAxis(){};

	void Render(Object *pObj, IDriver3D *pDriver, f32 fIntensity, const Vector3& vOffset);
	void SetUV_Flip(bool bFlipX, bool bFlipY, bool bFlipAxis=false) { m_aFlip[0] = bFlipX?1:0; m_aFlip[1] = bFlipY?1:0; m_aFlip[2] = bFlipAxis?1:0; }
	void SetAlpha(f32 fAlpha=1.0f) { m_fAlpha = fAlpha; }
	void AddFX_Frame(TextureHandle frameTex, u32 uWidth, u32 uHeight)
	{
		FX_Frame frame;
		frame.hTex    = frameTex;
		frame.uWidth  = uWidth;
		frame.uHeight = uHeight;
		m_fxFrames.push_back(frame);
	}

	//Oriented Sprite specific functions.
	void SetTextureHandle(TextureHandle hTex) { m_hTex = hTex; }
	void SetBaseIntensity(f32 fBaseItens) { m_fBaseItens = fBaseItens; }

	void SetFlag(u32 uFlag) { m_uFlags |= uFlag; }
	bool IsFlagSet(u32 uFlag) { return (m_uFlags&uFlag)!=0; }
public:
	enum
	{
		FLAG_EMISSIVE = (1<<0),
	};

private:
	struct FX_Frame
	{
		TextureHandle hTex;
		u32 uWidth;
		u32 uHeight;
	};

	TextureHandle m_hTex;
	vector<FX_Frame> m_fxFrames;
	u32 m_uFlags;
	u32 m_uCurFrame;
	s32 m_nFrameDelay;
	u8 m_aFlip[3];
	u8 m_uPad;
	f32 m_fBaseItens;
	f32 m_fAlpha;
};

#endif //SPRITE_ZAXIS_H

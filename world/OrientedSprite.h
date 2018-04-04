#ifndef ORIENTEDSPRITE_H
#define ORIENTEDSPRITE_H

#include "../CommonTypes.h"
#include "RenderComponent.h"

class IDriver3D;
class Object;

class OrientedSprite : public RenderComponent
{
public:
	OrientedSprite();
	virtual ~OrientedSprite();

	void Render(Object *pObj, IDriver3D *pDriver, f32 fIntensity, const Vector3& vOffset);
	void SetUV_Flip(bool bFlipX, bool bFlipY, bool bFlipAxis=false) { m_aFlip[0] = bFlipX?1:0; m_aFlip[1] = bFlipY?1:0; m_aFlip[2] = bFlipAxis?1:0; }
	void SetAlpha(f32 fAlpha=1.0f) { m_fAlpha = fAlpha; }

	//Oriented Sprite specific functions.
	void SetTextureHandle(TextureHandle hTex) { m_hTex = hTex; }
	void SetBaseIntensity(f32 fBaseItens) { m_fBaseItens = fBaseItens; }

private:
	TextureHandle m_hTex;
	u8 m_aFlip[3];
	u8 m_uPad;
	f32 m_fBaseItens;
	f32 m_fAlpha;
};

#endif //ORIENTEDSPRITE_H

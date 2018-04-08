#ifndef ORIENTEDSPRITE_H
#define ORIENTEDSPRITE_H

#include "../CommonTypes.h"
#include "RenderComponent.h"

class IDriver3D;

class Object;

class OrientedSprite : public RenderComponent {
public:
    OrientedSprite();

    virtual ~OrientedSprite() {};

    void Render(Object *pObj, IDriver3D *pDriver, float fIntensity, const Vector3 &vOffset);

    void SetUV_Flip(bool bFlipX, bool bFlipY, bool bFlipAxis = false) {
        m_aFlip[0] = bFlipX ? 1 : 0;
        m_aFlip[1] = bFlipY ? 1 : 0;
        m_aFlip[2] = bFlipAxis ? 1 : 0;
    }

    void SetAlpha(float fAlpha = 1.0f) { m_fAlpha = fAlpha; }

    //Oriented Sprite specific functions.
    void SetTextureHandle(TextureHandle hTex) { m_hTex = hTex; }

    void SetBaseIntensity(float fBaseItens) { m_fBaseItens = fBaseItens; }

private:
    TextureHandle m_hTex;
    uint8_t m_aFlip[3];
    uint8_t m_uPad;
    float m_fBaseItens;
    float m_fAlpha;
};

#endif //ORIENTEDSPRITE_H

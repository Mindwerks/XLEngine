#ifndef SPRITE_ZAXIS_H
#define SPRITE_ZAXIS_H

#include "../CommonTypes.h"
#include "RenderComponent.h"

#include <vector>

class IDriver3D;
class Object;

class Sprite_ZAxis : public RenderComponent
{
public:
    Sprite_ZAxis();
    virtual ~Sprite_ZAxis() = default;

    virtual void Render(Object *pObj, IDriver3D *pDriver, float fIntensity, const Vector3& vOffset) override;
    virtual void SetUV_Flip(bool bFlipX, bool bFlipY, bool bFlipAxis=false) override { m_aFlip[0] = bFlipX?1:0; m_aFlip[1] = bFlipY?1:0; m_aFlip[2] = bFlipAxis?1:0; }
    void SetAlpha(float fAlpha=1.0f) { m_fAlpha = fAlpha; }
    void AddFX_Frame(TextureHandle frameTex, uint32_t uWidth, uint32_t uHeight)
    {
        FX_Frame frame;
        frame.hTex    = frameTex;
        frame.uWidth  = uWidth;
        frame.uHeight = uHeight;
        m_fxFrames.push_back(frame);
    }

    //Oriented Sprite specific functions.
    virtual void SetTextureHandle(TextureHandle hTex) override { m_hTex = hTex; }
    void SetBaseIntensity(float fBaseItens) { m_fBaseItens = fBaseItens; }

    void SetFlag(uint32_t uFlag) { m_uFlags |= uFlag; }
    bool IsFlagSet(uint32_t uFlag) { return (m_uFlags&uFlag)!=0; }
public:
    enum
    {
        FLAG_EMISSIVE = (1<<0),
    };

private:
    struct FX_Frame
    {
        TextureHandle hTex;
        uint32_t uWidth;
        uint32_t uHeight;
    };

    TextureHandle m_hTex;
    std::vector<FX_Frame> m_fxFrames;
    uint32_t m_uFlags;
    uint32_t m_uCurFrame;
    int32_t m_nFrameDelay;
    uint8_t m_aFlip[3];
    uint8_t m_uPad;
    float m_fBaseItens;
    float m_fAlpha;
};

#endif //SPRITE_ZAXIS_H

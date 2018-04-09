#ifndef LFD_ANIM_H
#define LFD_ANIM_H

#include <cmath>
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
    void SetScale(float sx=1.0f, float sy=1.0f) { m_fScaleX = sx; m_fScaleY = sy; }
    void Render(int32_t frame, float x=0.0f, float y=0.0f, float maxX=1.0f, float minY=0.0f, float dU=0.0f, float dV=0.0f, bool bDistort=false);
    void GetFrameExtents(int32_t frame, float x, float y, int32_t& frameX0, int32_t& frameY0, int32_t& frameWidth, int32_t& frameHeight);

    void SetOffsScale(float sx, float sy);
    int32_t GetFrameCount() { return m_nNumDelts; }
private:
    TextureHandle m_hTex[256];
    float m_Width[256];
    float m_Height[256];
    float m_OffsX[256];
    float m_OffsY[256];

    float m_u1[256];
    float m_v1[256];

    float m_fScaleX, m_fScaleY;
    int32_t m_nNumDelts;

    IDriver3D *m_pDriver;

    static PLTT_File m_PalFile;
};

#endif //LFD_ANIM_H
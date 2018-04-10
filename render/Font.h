#ifndef FONT_H
#define FONT_H

#include "../CommonTypes.h"
#include "../math/Vector2.h"
#include "../math/Vector3.h"
#include <string>

class IDriver3D;

struct FontVertex
{
    Vector3 pos;
    Vector2 uv;
};

struct CharDescriptor
{
    //clean 16 bytes
    uint16_t x, y;
    uint16_t Width, Height;
    int16_t XOffset, YOffset;
    int16_t XAdvance;
    uint16_t Page;

    CharDescriptor() : x( 0 ), y( 0 ), Width( 0 ), Height( 0 ), XOffset( 0 ), YOffset( 0 ),
        XAdvance( 0 ), Page( 0 )
    { }
};

struct Charset
{
    uint16_t LineHeight;
    uint16_t Base;
    uint16_t Width, Height;
    uint16_t Pages;
    CharDescriptor Chars[256];
};

class XLFont
{
public:
    XLFont();
    ~XLFont();

    bool Load( const std::string& szFile, IDriver3D *pDriver );
    TextureHandle GetTexture() { return m_hTex; }
    int32_t FillVB(int32_t x, int32_t y, const std::string& szString, FontVertex *pVB_Data);

    uint32_t ComputePixelPos(const std::string& szString, uint32_t uPos);

private:
    TextureHandle m_hTex;
    Charset m_CharSet;
};

#endif //FONT_H

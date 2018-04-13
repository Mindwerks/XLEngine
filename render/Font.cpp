#include "Font.h"
#include "FontManager.h"
#include "TextureCache.h"
#include "IDriver3D.h"
#include "../fileformats/Vfs.h"

#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <memory.h>

XLFont::XLFont()
{
    m_hTex = XL_INVALID_TEXTURE;
}

XLFont::~XLFont()
{
}

bool XLFont::Load( const std::string& szFile, IDriver3D */*pDriver*/ )
{
    bool bResult = false;

    auto fntFile = Vfs::get().openInput(szFile);
    if(!fntFile) return false;

    uint8_t version[4] = {0};
    fntFile->read(reinterpret_cast<char*>(version), 4);
    if ( version[0] != 'B' || version[1] != 'M' || version[2] != 'F' || version[3] != 0x03 )
        return false;

    //final texture name.
    char szTextureName[64];

    //zero out all the characters.
    memset(m_CharSet.Chars, 0, sizeof(m_CharSet.Chars));

    bool bDone = false;
    while(!fntFile->eof() && fntFile->good() && bDone == false)
    {
        //read the block type and size.
        uint8_t  uBlockType;
        uint32_t uBlockSize;

        uBlockType = fntFile->get();
        uBlockSize = read_le<uint32_t>(*fntFile);

        switch(uBlockType)
        {
            case 1: //INFO block
            {
                uint16_t fontSize, stretchH;
                uint8_t  flags, charSet, aa, spacingHoriz, spacingVert, outline;
                uint8_t  padding[4];
                char szName[64];
                fontSize     = read_le<uint16_t>(*fntFile);
                flags        = fntFile->get();
                charSet      = fntFile->get();
                stretchH     = read_le<uint16_t>(*fntFile);
                aa           = fntFile->get();
                padding[0]   = fntFile->get();
                padding[1]   = fntFile->get();
                padding[2]   = fntFile->get();
                padding[3]   = fntFile->get();
                spacingHoriz = fntFile->get();
                spacingVert  = fntFile->get();
                outline      = fntFile->get();
                fntFile->read(szName, uBlockSize-14);
            }
            break;

            case 2:
            {
                uint8_t flags;
                uint8_t channels[4];
                m_CharSet.LineHeight = read_le<uint16_t>(*fntFile);
                m_CharSet.Base       = read_le<uint16_t>(*fntFile);
                m_CharSet.Width      = read_le<uint16_t>(*fntFile);
                m_CharSet.Height     = read_le<uint16_t>(*fntFile);
                m_CharSet.Pages      = read_le<uint16_t>(*fntFile);
                flags       = fntFile->get();
                channels[0] = fntFile->get();
                channels[1] = fntFile->get();
                channels[2] = fntFile->get();
                channels[3] = fntFile->get();
            }
            break;

            case 3:
            {
                fntFile->read(szTextureName, uBlockSize);
            }
            break;

            case 4:
            {
                uint32_t ID;
                uint8_t page, channel;
                int32_t nBlockSize = (int32_t)uBlockSize;
                while(nBlockSize >= 20)
                {
                    ID = read_le<uint32_t>(*fntFile);
                    if(ID > 255) ID = 255;

                    m_CharSet.Chars[ID].x        = read_le<uint16_t>(*fntFile);
                    m_CharSet.Chars[ID].y        = read_le<uint16_t>(*fntFile);
                    m_CharSet.Chars[ID].Width    = read_le<uint16_t>(*fntFile);
                    m_CharSet.Chars[ID].Height   = read_le<uint16_t>(*fntFile);
                    m_CharSet.Chars[ID].XOffset  = read_le<int16_t>(*fntFile);
                    m_CharSet.Chars[ID].YOffset  = read_le<int16_t>(*fntFile);
                    m_CharSet.Chars[ID].XAdvance = read_le<int16_t>(*fntFile);
                    page    = fntFile->get();
                    channel = fntFile->get();
                    m_CharSet.Chars[ID].Page = page;

                    nBlockSize -= 20;
                };
            }
            break;

            case 5: //Kerning pairs - currently unused.
            {
                uint32_t first, second;
                uint16_t amount;
                first  = read_le<uint32_t>(*fntFile);
                second = read_le<uint32_t>(*fntFile);
                amount = read_le<uint16_t>(*fntFile);
            }
            break;

            default:
                bDone = true;
        }
    }

    fntFile = nullptr;
    bResult = true;

    //now load the texture.
    m_hTex = TextureCache::LoadTexture(szTextureName, false);

    return bResult;
}

uint32_t XLFont::ComputePixelPos(const std::string& szString, uint32_t uPos)
{
    if(uPos > szString.size())
        uPos = szString.size();
    int32_t nCurX = 0;
    for(uint32_t i = 0; i < uPos;i++)
        nCurX += m_CharSet.Chars[(uint8_t)szString[i]].XAdvance;
    return (uint32_t)nCurX;
}

int32_t XLFont::FillVB(int32_t x, int32_t y, const std::string& szString, FontVertex *pVB_Data)
{
    int32_t nCurX = x;
    int32_t nCurY = y;

    float texelX  = 1.0f / (float)m_CharSet.Width;
    float texelY  = 1.0f / (float)m_CharSet.Height;

    size_t l = szString.size();

    assert(l < MAX_STRING_COUNT);

    const char *pszText = szString.c_str();
    for (size_t i=0, i4=0; i<l; i++, i4+=4)
    {
        uint16_t CharX   = m_CharSet.Chars[(uint8_t)pszText[i]].x;
        uint16_t CharY   = m_CharSet.Chars[(uint8_t)pszText[i]].y;
        uint16_t Width   = m_CharSet.Chars[(uint8_t)pszText[i]].Width;
        uint16_t Height  = m_CharSet.Chars[(uint8_t)pszText[i]].Height;
        int16_t OffsetX = m_CharSet.Chars[(uint8_t)pszText[i]].XOffset;
        int16_t OffsetY = m_CharSet.Chars[(uint8_t)pszText[i]].YOffset;

        //upper left
        pVB_Data[i4+0].uv.x  = (float)CharX * texelX;
        pVB_Data[i4+0].uv.y  = (float)CharY * texelY;
        pVB_Data[i4+0].pos.x = (float)(nCurX + OffsetX);
        pVB_Data[i4+0].pos.y = (float)(nCurY + OffsetY);
        pVB_Data[i4+0].pos.z = -1.0f;

        //upper right
        pVB_Data[i4+1].uv.x  = (float)(CharX+Width) * texelX;
        pVB_Data[i4+1].uv.y  = (float)CharY * texelY;
        pVB_Data[i4+1].pos.x = (float)(Width + nCurX + OffsetX);
        pVB_Data[i4+1].pos.y = (float)(nCurY + OffsetY);
        pVB_Data[i4+1].pos.z = -1.0f;

        //lower right
        pVB_Data[i4+2].uv.x  = (float)(CharX+Width) * texelX;
        pVB_Data[i4+2].uv.y  = (float)(CharY+Height) * texelY;
        pVB_Data[i4+2].pos.x = (float)(Width + nCurX + OffsetX);
        pVB_Data[i4+2].pos.y = (float)(nCurY + Height + OffsetY);
        pVB_Data[i4+2].pos.z = -1.0f;

        //lower left
        pVB_Data[i4+3].uv.x  = (float)CharX * texelX;
        pVB_Data[i4+3].uv.y  = (float)(CharY+Height) * texelY;
        pVB_Data[i4+3].pos.x = (float)(nCurX + OffsetX);
        pVB_Data[i4+3].pos.y = (float)(nCurY + Height + OffsetY);
        pVB_Data[i4+3].pos.z = -1.0f;

        nCurX += m_CharSet.Chars[(uint8_t)pszText[i]].XAdvance;
    }

    return (int32_t)l;
}

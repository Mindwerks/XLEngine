#include "Font.h"
#include "FontManager.h"
#include "TextureCache.h"
#include "IDriver3D.h"
#include <cassert>

XLFont::XLFont(void) {
    m_hTex = XL_INVALID_TEXTURE;
}

XLFont::~XLFont(void) {
}

bool XLFont::Load(const string &szFile, IDriver3D *pDriver) {
    bool bResult = false;

    FILE *fntFile = fopen(szFile.c_str(), "rb");
    if (fntFile)
    {
        uint8_t version[4];
        fread(version, 1, 4, fntFile);
        if (version[0] != 'B' || version[1] != 'M' || version[2] != 'F' || version[3] != 0x03)
        {
            fclose(fntFile);
            return false;
        }

        //final texture name.
        char szTextureName[64];

        //zero out all the characters.
        memset(m_CharSet.Chars, 0, sizeof(CharDescriptor) * 256);

        bool bDone = false;
        while (feof(fntFile) == 0 && bDone == false)
        {
            //read the block type and size.
            uint8_t uBlockType;
            uint32_t uBlockSize;

            fread(&uBlockType, 1, 1, fntFile);
            fread(&uBlockSize, 4, 1, fntFile);

            switch (uBlockType)
            {
                case 1:    //INFO block
                {
                    uint16_t fontSize, stretchH;
                    uint8_t flags, charSet, aa, spacingHoriz, spacingVert, outline;
                    uint8_t padding[4];
                    char szName[64];
                    fread(&fontSize, 2, 1, fntFile);
                    fread(&flags, 1, 1, fntFile);
                    fread(&charSet, 1, 1, fntFile);
                    fread(&stretchH, 2, 1, fntFile);
                    fread(&aa, 1, 1, fntFile);
                    fread(&padding[0], 1, 1, fntFile);
                    fread(&padding[1], 1, 1, fntFile);
                    fread(&padding[2], 1, 1, fntFile);
                    fread(&padding[3], 1, 1, fntFile);
                    fread(&spacingHoriz, 1, 1, fntFile);
                    fread(&spacingVert, 1, 1, fntFile);
                    fread(&outline, 1, 1, fntFile);
                    fread(szName, 1, uBlockSize - 14, fntFile);
                }
                    break;
                case 2:
                {
                    uint8_t flags;
                    uint8_t channels[4];
                    fread(&m_CharSet.LineHeight, 2, 1, fntFile);
                    fread(&m_CharSet.Base, 2, 1, fntFile);
                    fread(&m_CharSet.Width, 2, 1, fntFile);
                    fread(&m_CharSet.Height, 2, 1, fntFile);
                    fread(&m_CharSet.Pages, 2, 1, fntFile);
                    fread(&flags, 1, 1, fntFile);
                    fread(&channels[0], 1, 1, fntFile);
                    fread(&channels[1], 1, 1, fntFile);
                    fread(&channels[2], 1, 1, fntFile);
                    fread(&channels[3], 1, 1, fntFile);
                }
                    break;
                case 3:
                {
                    fread(szTextureName, 1, uBlockSize, fntFile);
                }
                    break;
                case 4:
                {
                    uint32_t ID;
                    uint8_t page, channel;
                    int32_t nBlockSize = (int32_t) uBlockSize;
                    while (nBlockSize > 0)
                    {
                        fread(&ID, 4, 1, fntFile);

                        if (ID > 255)
                        {
                            ID = 255;
                        }
                        fread(&m_CharSet.Chars[ID].x, 2, 1, fntFile);
                        fread(&m_CharSet.Chars[ID].y, 2, 1, fntFile);
                        fread(&m_CharSet.Chars[ID].Width, 2, 1, fntFile);
                        fread(&m_CharSet.Chars[ID].Height, 2, 1, fntFile);
                        fread(&m_CharSet.Chars[ID].XOffset, 2, 1, fntFile);
                        fread(&m_CharSet.Chars[ID].YOffset, 2, 1, fntFile);
                        fread(&m_CharSet.Chars[ID].XAdvance, 2, 1, fntFile);
                        fread(&page, 1, 1, fntFile);
                        fread(&channel, 1, 1, fntFile);
                        m_CharSet.Chars[ID].Page = page;

                        nBlockSize -= 20;
                    };
                }
                    break;
                case 5:    //Kerning pairs - currently unused.
                {
                    uint32_t first, second;
                    uint16_t amount;
                    fread(&first, 4, 1, fntFile);
                    fread(&second, 4, 1, fntFile);
                    fread(&amount, 2, 1, fntFile);
                }
                    break;
                default:
                {
                    bDone = true;
                }
            };
        };

        fclose(fntFile);
        bResult = true;

        //now load the texture.
        m_hTex = TextureCache::LoadTexture(szTextureName, false);
    }

    return bResult;
}

uint32_t XLFont::ComputePixelPos(const string &szString, uint32_t uPos) {
    const char *pszText = szString.c_str();
    int32_t nCurX = 0;
    for (uint32_t i = 0; i < uPos; i++)
    {
        nCurX += m_CharSet.Chars[pszText[i]].XAdvance;
    }
    return (uint32_t) nCurX;
}

int32_t XLFont::FillVB(int32_t x, int32_t y, const string &szString, FontVertex *pVB_Data) {
    int32_t nCurX = x;
    int32_t nCurY = y;

    float texelX = 1.0f / (float) m_CharSet.Width;
    float texelY = 1.0f / (float) m_CharSet.Height;

    size_t l = szString.size();

    assert(l < MAX_STRING_COUNT);

    const char *pszText = szString.c_str();
    for (size_t i = 0, i4 = 0; i < l; i++, i4 += 4)
    {
        uint16_t CharX = m_CharSet.Chars[pszText[i]].x;
        uint16_t CharY = m_CharSet.Chars[pszText[i]].y;
        uint16_t Width = m_CharSet.Chars[pszText[i]].Width;
        uint16_t Height = m_CharSet.Chars[pszText[i]].Height;
        int16_t OffsetX = m_CharSet.Chars[pszText[i]].XOffset;
        int16_t OffsetY = m_CharSet.Chars[pszText[i]].YOffset;

        //upper left
        pVB_Data[i4 + 0].uv.x = (float) CharX * texelX;
        pVB_Data[i4 + 0].uv.y = (float) CharY * texelY;
        pVB_Data[i4 + 0].pos.x = (float) (nCurX + OffsetX);
        pVB_Data[i4 + 0].pos.y = (float) (nCurY + OffsetY);
        pVB_Data[i4 + 0].pos.z = -1.0f;

        //upper right
        pVB_Data[i4 + 1].uv.x = (float) (CharX + Width) * texelX;
        pVB_Data[i4 + 1].uv.y = (float) CharY * texelY;
        pVB_Data[i4 + 1].pos.x = (float) (Width + nCurX + OffsetX);
        pVB_Data[i4 + 1].pos.y = (float) (nCurY + OffsetY);
        pVB_Data[i4 + 1].pos.z = -1.0f;

        //lower right
        pVB_Data[i4 + 2].uv.x = (float) (CharX + Width) * texelX;
        pVB_Data[i4 + 2].uv.y = (float) (CharY + Height) * texelY;
        pVB_Data[i4 + 2].pos.x = (float) (Width + nCurX + OffsetX);
        pVB_Data[i4 + 2].pos.y = (float) (nCurY + Height + OffsetY);
        pVB_Data[i4 + 2].pos.z = -1.0f;

        //lower left
        pVB_Data[i4 + 3].uv.x = (float) CharX * texelX;
        pVB_Data[i4 + 3].uv.y = (float) (CharY + Height) * texelY;
        pVB_Data[i4 + 3].pos.x = (float) (nCurX + OffsetX);
        pVB_Data[i4 + 3].pos.y = (float) (nCurY + Height + OffsetY);
        pVB_Data[i4 + 3].pos.z = -1.0f;

        nCurX += m_CharSet.Chars[pszText[i]].XAdvance;
    }

    return (int32_t) l;
}

#ifndef TEXTURECONVERTER_IMG_H
#define TEXTURECONVERTER_IMG_H

#include "TextureConverter.h"

class TextureConv_IMG : public TextureConverter {
public:
    TextureConv_IMG();

    ~TextureConv_IMG();

    uint32_t GetHackID(const char *pszImage);

    bool
    ConvertTexture_Pal8(uint8_t *pConvertedData, int32_t &nOffsX, int32_t &nOffsY, uint32_t &uWidth, uint32_t &uHeight,
                        const uint8_t *pSourceData, uint32_t uLen, const uint8_t *pPalette, bool bCopyPal,
                        uint32_t uHackID = 0);

    bool ConvertTexture_Pal8_TexList(uint8_t *pConvertedData, int32_t &nOffsX, int32_t &nOffsY, uint32_t &uWidth,
                                     uint32_t &uHeight, const uint8_t *pSourceData, uint32_t uLen,
                                     const uint8_t *pPalette, int nRecord, uint32_t uHackID = 0);

    bool
    ConvertTexture_8bpp(uint8_t *pConvertedData, int32_t &nOffsX, int32_t &nOffsY, uint32_t &uWidth, uint32_t &uHeight,
                        const uint8_t *pSourceData, uint32_t uLen, uint32_t uHackID = 0);

    uint32_t ConvertTexture_8bpp_TexList(uint8_t *pConvertedData, int32_t &nOffsX, int32_t &nOffsY, uint32_t &uWidth,
                                         uint32_t &uHeight, const uint8_t *pSourceData, uint32_t uLen, int nRecord,
                                         uint32_t uHackID = 0);

    //Get extra game/format specific data.
    virtual void *GetExtraTexData(uint32_t &uDataSize) {
        uDataSize = 4;
        return m_aExtraData;
    }

private:
    int16_t m_aExtraData[2];
};

#endif //TEXTURECONVERTER_IMG_H
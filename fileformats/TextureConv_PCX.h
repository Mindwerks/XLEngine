#ifndef TEXTURECONVERTER_PCX_H
#define TEXTURECONVERTER_PCX_H

#include "TextureConverter.h"

class TextureConv_PCX : public TextureConverter {
public:
    TextureConv_PCX();

    ~TextureConv_PCX();

    bool
    ConvertTexture_Pal8(uint8_t *pConvertedData, int32_t &nOffsX, int32_t &nOffsY, uint32_t &uWidth, uint32_t &uHeight,
                        const uint8_t *pSourceData, uint32_t uLen, const uint8_t *pPalette, bool bCopyPal,
                        uint32_t uHackID = 0);
};

#endif //TEXTURECONVERTER_PCX_H
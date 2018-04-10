#ifndef TEXTURECONVERTER_H
#define TEXTURECONVERTER_H

#include "../CommonTypes.h"

class TextureConverter
{
public:
    TextureConverter() = default;
    virtual ~TextureConverter() = default;

    //Some games have weird format deviations that must be discovered based on the file name.
    //These are "data hacks" that cannot be fixed without changing the data.
    //So we just live with them, if a format doesn't need this support then do nothing ... the default is 0 which means no hack required.
    virtual uint32_t  GetHackID(const char *pszFileName) { return 0; }  //<- 0 = no hack required, the default state.
    virtual bool ConvertTexture_Pal8(uint8_t *pConvertedData, int32_t& nOffsX, int32_t& nOffsY, uint32_t& uWidth, uint32_t& uHeight, const uint8_t *pSourceData, uint32_t uLen, const uint8_t *pPalette, bool bCopyPal, uint32_t uHackID=0) {return false;}
    virtual bool ConvertTexture_Pal8_TexList(uint8_t *pConvertedData, int32_t& nOffsX, int32_t& nOffsY, uint32_t& uWidth, uint32_t& uHeight, const uint8_t *pSourceData, uint32_t uLen, const uint8_t *pPalette, int nRecord, uint32_t uHackID=0) {return false;}

    virtual bool ConvertTexture_8bpp(uint8_t *pConvertedData, int32_t& nOffsX, int32_t& nOffsY, uint32_t& uWidth, uint32_t& uHeight, const uint8_t *pSourceData, uint32_t uLen, uint32_t uHackID=0) {return false;}
    virtual uint32_t  ConvertTexture_8bpp_TexList(uint8_t *pConvertedData, int32_t& nOffsX, int32_t& nOffsY, uint32_t& uWidth, uint32_t& uHeight, const uint8_t *pSourceData, uint32_t uLen, int nRecord, uint32_t uHackID=0) {return false;}
    //Get extra game/format specific data.
    virtual void *GetExtraTexData(uint32_t& uDataSize) {uDataSize=0; return 0;}
};

#endif //TEXTURECONVERTER_H
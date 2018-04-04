#ifndef TEXTURECONVERTER_PCX_H
#define TEXTURECONVERTER_PCX_H

#include "TextureConverter.h"

class TextureConv_PCX : public TextureConverter
{
public:
	TextureConv_PCX();
	~TextureConv_PCX();

	bool ConvertTexture_Pal8(u8 *pConvertedData, s32& nOffsX, s32& nOffsY, u32& uWidth, u32& uHeight, const u8 *pSourceData, u32 uLen, const u8 *pPalette, bool bCopyPal, u32 uHackID=0);
};

#endif //TEXTURECONVERTER_PCX_H
#ifndef TEXTURECONVERTER_IMG_H
#define TEXTURECONVERTER_IMG_H

#include "TextureConverter.h"

class TextureConv_IMG : public TextureConverter
{
public:
	TextureConv_IMG();
	~TextureConv_IMG();

	u32 GetHackID(const char *pszImage);
	bool ConvertTexture_Pal8(u8 *pConvertedData, s32& nOffsX, s32& nOffsY, u32& uWidth, u32& uHeight, const u8 *pSourceData, u32 uLen, const u8 *pPalette, bool bCopyPal, u32 uHackID=0);
	bool ConvertTexture_Pal8_TexList(u8 *pConvertedData, s32& nOffsX, s32& nOffsY, u32& uWidth, u32& uHeight, const u8 *pSourceData, u32 uLen, const u8 *pPalette, int nRecord, u32 uHackID=0);

	bool ConvertTexture_8bpp(u8 *pConvertedData, s32& nOffsX, s32& nOffsY, u32& uWidth, u32& uHeight, const u8 *pSourceData, u32 uLen, u32 uHackID=0);
	u32  ConvertTexture_8bpp_TexList(u8 *pConvertedData, s32& nOffsX, s32& nOffsY, u32& uWidth, u32& uHeight, const u8 *pSourceData, u32 uLen, int nRecord, u32 uHackID=0);
	//Get extra game/format specific data.
	virtual void *GetExtraTexData(u32& uDataSize) { uDataSize=4; return m_aExtraData; }
private:
	s16 m_aExtraData[2];
};

#endif //TEXTURECONVERTER_IMG_H
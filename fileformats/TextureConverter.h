#ifndef TEXTURECONVERTER_H
#define TEXTURECONVERTER_H

#include "../CommonTypes.h"

class TextureConverter
{
public:
	TextureConverter()  {};
	virtual ~TextureConverter() {};

	//Some games have weird format deviations that must be discovered based on the file name.
	//These are "data hacks" that cannot be fixed without changing the data.
	//So we just live with them, if a format doesn't need this support then do nothing ... the default is 0 which means no hack required.
	virtual u32  GetHackID(const char *pszFileName) { return 0; }	//<- 0 = no hack required, the default state.
	virtual bool ConvertTexture_Pal8(u8 *pConvertedData, s32& nOffsX, s32& nOffsY, u32& uWidth, u32& uHeight, const u8 *pSourceData, u32 uLen, const u8 *pPalette, bool bCopyPal, u32 uHackID=0) {return false;}
	virtual bool ConvertTexture_Pal8_TexList(u8 *pConvertedData, s32& nOffsX, s32& nOffsY, u32& uWidth, u32& uHeight, const u8 *pSourceData, u32 uLen, const u8 *pPalette, int nRecord, u32 uHackID=0) {return false;}

	virtual bool ConvertTexture_8bpp(u8 *pConvertedData, s32& nOffsX, s32& nOffsY, u32& uWidth, u32& uHeight, const u8 *pSourceData, u32 uLen, u32 uHackID=0) {return false;}
	virtual u32  ConvertTexture_8bpp_TexList(u8 *pConvertedData, s32& nOffsX, s32& nOffsY, u32& uWidth, u32& uHeight, const u8 *pSourceData, u32 uLen, int nRecord, u32 uHackID=0) {return false;}
	//Get extra game/format specific data.
	virtual void *GetExtraTexData(u32& uDataSize) {uDataSize=0; return 0;}
};

#endif //TEXTURECONVERTER_H
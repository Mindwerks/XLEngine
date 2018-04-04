#ifndef TEXTURELOADER_H
#define TEXTURELOADER_H

#include "../CommonTypes.h"
#include "TextureTypes.h"
#include <string>
#include <vector>
#include <map>

using namespace std;
class TextureConverter;
class Archive;

#define MAX_LIGHT_LEVELS 64

struct Palette
{
	u8 colors[256*4];	
};

struct Palette_24
{
	u8 colors[256*3];
};

struct Colormap
{
	u8 data[256*MAX_LIGHT_LEVELS];
	int lightLevels;
};

class TextureLoader
{
public:
	static void Init();
	static void Destroy();

	static bool LoadTexture(u32 uTextureType, u32 uPalIndex, Archive *pTexArchive, const string& sFile, bool bCopyPal);
	static u32  LoadTexture_TexList(u32 uTextureType, u32 uPalIndex, Archive *pTexArchive, const string& sFile, int nRecord);
	static bool LoadTexture_NoArchive(u32 uTextureType, u32 uPalIndex, const string& sFile, bool bCopyPal);
	static u32  LoadTexture_NoArchive_TexList(u32 uTextureType, u32 uPalIndex, const string& sFile, int nRecord);
	static bool LoadTexture_Mem( const u8 *pImgBuffer, u32 uPalIndex, u32 width, u32 height );
	static void GetTextureSize(s32& nOffsX, s32& nOffsY, u32& uWidth, u32& uHeight);
	static void *GetTexExtraData(u32& uDataSize) { uDataSize = m_uExtraDataSize; return m_pTexExtraData; }
	static u8  *GetData_RGBA8();
	static void FreeData();

	static void SetPalette(u8 uPalIndex, u8 *pData, u32 uSize, u32 uTransparentIndex);
	static void SetColormap(u8 uColIndex, u8 *pData, int nLightLevels);
	static u8  *GetPaletteData(u8 uPalIndex) { return m_CurPal[uPalIndex].colors; }
	static u8  *GetColormapData(u8 uColormapIndex) { return m_ColorMap[uColormapIndex].data; }
	static int  GetNumLightLevels(u8 uColormapIndex) { return m_ColorMap[uColormapIndex].lightLevels; }

	//Set the color depth that textures are stored in.
	//32bpp is the default (and should always be used with OpenGL).
	static void SetTextureColorDepth(u32 uColorDepth=32) { m_uTexColorDepth = uColorDepth; }

private:
	static TextureConverter *m_TextureConverters[];
	static TextureConverter *m_pCurConverter;
	static u8     *m_pConvertedData;
	static u32     m_uWidth;
	static u32     m_uHeight;
	static s32	   m_nOffsX;
	static s32	   m_nOffsY;
	static u32     m_uExtraDataSize;
	static u32	   m_uTexColorDepth;
	static void   *m_pTexExtraData;
	static Palette m_CurPal[];
	static Colormap m_ColorMap[];
};

#endif //ARCHIVEMANAGER_H
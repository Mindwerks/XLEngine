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
	uint8_t colors[256*4];
};

struct Palette_24
{
	uint8_t colors[256*3];
};

struct Colormap
{
	uint8_t data[256*MAX_LIGHT_LEVELS];
	int lightLevels;
};

class TextureLoader
{
public:
	static void Init();
	static void Destroy();

	static bool LoadTexture(uint32_t uTextureType, uint32_t uPalIndex, Archive *pTexArchive, const string& sFile, bool bCopyPal);
	static uint32_t  LoadTexture_TexList(uint32_t uTextureType, uint32_t uPalIndex, Archive *pTexArchive, const string& sFile, int nRecord);
	static bool LoadTexture_NoArchive(uint32_t uTextureType, uint32_t uPalIndex, const string& sFile, bool bCopyPal);
	static uint32_t  LoadTexture_NoArchive_TexList(uint32_t uTextureType, uint32_t uPalIndex, const string& sFile, int nRecord);
	static bool LoadTexture_Mem( const uint8_t *pImgBuffer, uint32_t uPalIndex, uint32_t width, uint32_t height );
	static void GetTextureSize(int32_t& nOffsX, int32_t& nOffsY, uint32_t& uWidth, uint32_t& uHeight);
	static void *GetTexExtraData(uint32_t& uDataSize) { uDataSize = m_uExtraDataSize; return m_pTexExtraData; }
	static uint8_t  *GetData_RGBA8();
	static void FreeData();

	static void SetPalette(uint8_t uPalIndex, uint8_t *pData, uint32_t uSize, uint32_t uTransparentIndex);
	static void SetColormap(uint8_t uColIndex, uint8_t *pData, int nLightLevels);
	static uint8_t  *GetPaletteData(uint8_t uPalIndex) { return m_CurPal[uPalIndex].colors; }
	static uint8_t  *GetColormapData(uint8_t uColormapIndex) { return m_ColorMap[uColormapIndex].data; }
	static int  GetNumLightLevels(uint8_t uColormapIndex) { return m_ColorMap[uColormapIndex].lightLevels; }

	//Set the color depth that textures are stored in.
	//32bpp is the default (and should always be used with OpenGL).
	static void SetTextureColorDepth(uint32_t uColorDepth=32) { m_uTexColorDepth = uColorDepth; }

private:
	static TextureConverter *m_TextureConverters[];
	static TextureConverter *m_pCurConverter;
	static uint8_t     *m_pConvertedData;
	static uint32_t     m_uWidth;
	static uint32_t     m_uHeight;
	static int32_t	   m_nOffsX;
	static int32_t	   m_nOffsY;
	static uint32_t     m_uExtraDataSize;
	static uint32_t	   m_uTexColorDepth;
	static void   *m_pTexExtraData;
	static Palette m_CurPal[];
	static Colormap m_ColorMap[];
};

#endif //ARCHIVEMANAGER_H
#include "TextureLoader.h"
#include "TextureConverter.h"
#include "ArchiveManager.h"
#include "TextureConv_ART.h"
#include "TextureConv_PCX.h"
#include "TextureConv_IMG.h"
#include "../memory/ScratchPad.h"
#include "../ui/XL_Console.h"
#include <assert.h>
#include <cstring>

#define MAX_PAL_COUNT 32
#define MAX_COLORMAP_COUNT 32

TextureConverter *TextureLoader::m_TextureConverters[TEXTURETYPE_COUNT];
TextureConverter *TextureLoader::m_pCurConverter;
uint8_t *TextureLoader::m_pConvertedData;
int32_t	TextureLoader::m_nOffsX;
int32_t	TextureLoader::m_nOffsY;
uint32_t TextureLoader::m_uWidth;
uint32_t TextureLoader::m_uHeight;
uint32_t TextureLoader::m_uExtraDataSize=0;
uint32_t TextureLoader::m_uTexColorDepth=32;
void *TextureLoader::m_pTexExtraData=NULL;
Palette TextureLoader::m_CurPal[MAX_PAL_COUNT];
Colormap TextureLoader::m_ColorMap[MAX_COLORMAP_COUNT];

void TextureLoader::Init()
{
	m_pCurConverter=NULL;
	m_pConvertedData=NULL;

	//create the texture converter list here.
	m_TextureConverters[TEXTURETYPE_ART] = xlNew TextureConv_ART();
	m_TextureConverters[TEXTURETYPE_PCX] = xlNew TextureConv_PCX();
	m_TextureConverters[TEXTURETYPE_IMG] = xlNew TextureConv_IMG();
}

void TextureLoader::Destroy()
{
	for (uint32_t c=0; c<TEXTURETYPE_COUNT; c++)
	{
		if ( m_TextureConverters[c] )
		{
			xlDelete m_TextureConverters[c];
		}
		m_TextureConverters[c] = NULL;
	}
	m_pCurConverter=NULL;
}

bool TextureLoader::LoadTexture(uint32_t uTextureType, uint32_t uPalIndex, Archive *pTexArchive, const string& sFile, bool bCopyPal)
{
	TextureConverter *pConvert = m_TextureConverters[uTextureType];
	m_pCurConverter = pConvert;

	if ( ArchiveManager::GameFile_Open(pTexArchive, sFile.c_str()) )
	{
		ScratchPad::StartFrame();

		uint32_t uLen  = ArchiveManager::GameFile_GetLength();
		uint8_t *pData = (uint8_t *)ScratchPad::AllocMem( uLen+1 );

		if ( pData )
		{
			ArchiveManager::GameFile_Read(pData, uLen);

			//Run through the texture converter.
			m_pConvertedData = (uint8_t *)ScratchPad::AllocMem( (uLen+1)*4 + 2048*2048 );
			uint32_t uHackID = m_pCurConverter->GetHackID( sFile.c_str() );

			if ( m_uTexColorDepth == 32 )
			{
				m_pCurConverter->ConvertTexture_Pal8(m_pConvertedData, m_nOffsX, m_nOffsY, m_uWidth, m_uHeight, pData, uLen, m_CurPal[uPalIndex].colors, bCopyPal, uHackID);
			}
			else if ( m_uTexColorDepth == 8 )
			{
				m_pCurConverter->ConvertTexture_8bpp(m_pConvertedData, m_nOffsX, m_nOffsY, m_uWidth, m_uHeight, pData, uLen, uHackID);
			}
			else
			{
				assert(0);
				return false;
			}

			m_pTexExtraData = m_pCurConverter->GetExtraTexData(m_uExtraDataSize);
		}
		ArchiveManager::GameFile_Close();
		return true;
	}

	return false;
}

uint32_t TextureLoader::LoadTexture_TexList(uint32_t uTextureType, uint32_t uPalIndex, Archive *pTexArchive, const string& sFile, int nRecord)
{
	uint32_t uFrameCnt = 1;

	TextureConverter *pConvert = m_TextureConverters[uTextureType];
	m_pCurConverter = pConvert;

	if ( ArchiveManager::GameFile_Open(pTexArchive, sFile.c_str()) )
	{
		ScratchPad::StartFrame();

		uint32_t uLen  = ArchiveManager::GameFile_GetLength();
		uint8_t *pData = (uint8_t *)ScratchPad::AllocMem( uLen+1 );

		if ( pData )
		{
			ArchiveManager::GameFile_Read(pData, uLen);

			//Run through the texture converter.
			m_pConvertedData = (uint8_t *)ScratchPad::AllocMem( (uLen+1)*4 + 2048*2048 );
			uint32_t uHackID = m_pCurConverter->GetHackID( sFile.c_str() );

			if ( m_uTexColorDepth == 32 )
			{
				m_pCurConverter->ConvertTexture_Pal8_TexList(m_pConvertedData, m_nOffsX, m_nOffsY, m_uWidth, m_uHeight, pData, uLen, m_CurPal[uPalIndex].colors, nRecord, uHackID);
			}
			else if ( m_uTexColorDepth == 8 )
			{
				uFrameCnt = m_pCurConverter->ConvertTexture_8bpp_TexList(m_pConvertedData, m_nOffsX, m_nOffsY, m_uWidth, m_uHeight, pData, uLen, nRecord, uHackID);
			}
			else
			{
				assert(0);
				return 0;
			}
			m_pTexExtraData = m_pCurConverter->GetExtraTexData(m_uExtraDataSize);
		}
		ArchiveManager::GameFile_Close();
		return uFrameCnt;
	}

	return 0;
}

bool TextureLoader::LoadTexture_NoArchive(uint32_t uTextureType, uint32_t uPalIndex, const string& sFile, bool bCopyPal)
{
	TextureConverter *pConvert = m_TextureConverters[uTextureType];
	m_pCurConverter = pConvert;

	if ( ArchiveManager::File_Open(sFile.c_str()) )
	{
		ScratchPad::StartFrame();

		uint32_t uLen  = ArchiveManager::File_GetLength();
		uint8_t *pData = (uint8_t *)ScratchPad::AllocMem( uLen+1 );

		if ( pData )
		{
			ArchiveManager::File_Read(pData, 0, uLen);

			//Run through the texture converter.
			m_pConvertedData = (uint8_t *)ScratchPad::AllocMem( (uLen+1)*4 + 2048*2048 );
			uint32_t uHackID = m_pCurConverter->GetHackID( sFile.c_str() );
			if ( m_uTexColorDepth == 32 )
			{
				m_pCurConverter->ConvertTexture_Pal8(m_pConvertedData, m_nOffsX, m_nOffsY, m_uWidth, m_uHeight, pData, uLen, m_CurPal[uPalIndex].colors, bCopyPal, uHackID);
			}
			else if ( m_uTexColorDepth == 8 )
			{
				m_pCurConverter->ConvertTexture_8bpp(m_pConvertedData, m_nOffsX, m_nOffsY, m_uWidth, m_uHeight, pData, uLen, uHackID);
			}
			else
			{
				assert(0);
				return false;
			}
			m_pTexExtraData = m_pCurConverter->GetExtraTexData(m_uExtraDataSize);
		}
		ArchiveManager::File_Close();
		return true;
	}
	return false;
}

uint32_t TextureLoader::LoadTexture_NoArchive_TexList(uint32_t uTextureType, uint32_t uPalIndex, const string& sFile, int nRecord)
{
	uint32_t uFrameCnt = 1;

	TextureConverter *pConvert = m_TextureConverters[uTextureType];
	m_pCurConverter = pConvert;

	if ( ArchiveManager::File_Open(sFile.c_str()) )
	{
		ScratchPad::StartFrame();

		uint32_t uLen  = ArchiveManager::File_GetLength();
		uint8_t *pData = (uint8_t *)ScratchPad::AllocMem( uLen+1 );

		if ( pData )
		{
			ArchiveManager::File_Read(pData, 0, uLen);

			//Run through the texture converter.
			m_pConvertedData = (uint8_t *)ScratchPad::AllocMem( (uLen+1)*4 + 2048*2048 );
			uint32_t uHackID = m_pCurConverter->GetHackID( sFile.c_str() );

			if ( m_uTexColorDepth == 32 )
			{
				m_pCurConverter->ConvertTexture_Pal8_TexList(m_pConvertedData, m_nOffsX, m_nOffsY, m_uWidth, m_uHeight, pData, uLen, m_CurPal[uPalIndex].colors, nRecord, uHackID);
			}
			else if ( m_uTexColorDepth == 8 )
			{
				uFrameCnt = m_pCurConverter->ConvertTexture_8bpp_TexList(m_pConvertedData, m_nOffsX, m_nOffsY, m_uWidth, m_uHeight, pData, uLen, nRecord, uHackID);
			}
			else
			{
				assert(0);
				return 0;
			}

			m_pTexExtraData = m_pCurConverter->GetExtraTexData(m_uExtraDataSize);
		}
		ArchiveManager::File_Close();
		return uFrameCnt;
	}
	return 0;
}

bool TextureLoader::LoadTexture_Mem(const uint8_t *pImgBuffer, uint32_t uPalIndex, uint32_t width, uint32_t height)
{
	ScratchPad::StartFrame();

	uint32_t uDataSize = width*height;
	if ( m_uTexColorDepth == 32 ) uDataSize *= 4;
	m_pConvertedData = (uint8_t *)ScratchPad::AllocMem( uDataSize );
	if ( !(m_pConvertedData != NULL) )
		return false;

	if ( m_uTexColorDepth == 32 )
	{
		for (uint32_t y=0, yOffs=0; y<height; y++, yOffs+=width*4)
		{
			for (uint32_t x=0, x4=0; x<width; x++, x4+=4)
			{
				uint32_t uIndex = pImgBuffer[x+y*width]*4;
				m_pConvertedData[yOffs + x4 + 0] = m_CurPal[uPalIndex].colors[ uIndex+0 ];
				m_pConvertedData[yOffs + x4 + 1] = m_CurPal[uPalIndex].colors[ uIndex+1 ];
				m_pConvertedData[yOffs + x4 + 2] = m_CurPal[uPalIndex].colors[ uIndex+2 ];
				m_pConvertedData[yOffs + x4 + 3] = m_CurPal[uPalIndex].colors[ uIndex+3 ];
			}
		}
	}
	else if ( m_uTexColorDepth == 8 )
	{
		memcpy(m_pConvertedData, pImgBuffer, uDataSize);
	}
		
	return true;
}

void TextureLoader::GetTextureSize(int32_t& nOffsX, int32_t& nOffsY, uint32_t& uWidth, uint32_t& uHeight)
{
	nOffsX  = m_nOffsX;
	nOffsY  = m_nOffsY;
	uWidth  = m_uWidth;
	uHeight = m_uHeight;
}

uint8_t *TextureLoader::GetData_RGBA8()
{
	return m_pConvertedData;
}

void TextureLoader::FreeData()
{
	ScratchPad::FreeFrame();
	m_pConvertedData = NULL;
}

void TextureLoader::SetPalette(uint8_t uPalIndex, uint8_t *pData, uint32_t uSize, uint32_t uTransparentIndex)
{
	uint32_t uColorCount = uSize/3;
	for (uint32_t i=0; i<uColorCount; i++)
	{
		m_CurPal[uPalIndex].colors[i*4+0] = pData[i*3+0];
		m_CurPal[uPalIndex].colors[i*4+1] = pData[i*3+1];
		m_CurPal[uPalIndex].colors[i*4+2] = pData[i*3+2];
		m_CurPal[uPalIndex].colors[i*4+3] = 255;
	}

	if ( uTransparentIndex < uColorCount )
	{
		m_CurPal[uPalIndex].colors[uTransparentIndex*4+0] = 0;
		m_CurPal[uPalIndex].colors[uTransparentIndex*4+1] = 0;
		m_CurPal[uPalIndex].colors[uTransparentIndex*4+2] = 0;
		m_CurPal[uPalIndex].colors[uTransparentIndex*4+3] = 0;
	}
}

void TextureLoader::SetColormap(uint8_t uColIndex, uint8_t *pData, int nLightLevels)
{
	memcpy(m_ColorMap[uColIndex].data, pData, nLightLevels*256);
	m_ColorMap[uColIndex].lightLevels = nLightLevels;
}
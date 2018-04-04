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
u8 *TextureLoader::m_pConvertedData;
s32	TextureLoader::m_nOffsX;
s32	TextureLoader::m_nOffsY;
u32 TextureLoader::m_uWidth;
u32 TextureLoader::m_uHeight;
u32 TextureLoader::m_uExtraDataSize=0;
u32 TextureLoader::m_uTexColorDepth=32;
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
	for (u32 c=0; c<TEXTURETYPE_COUNT; c++)
	{
		if ( m_TextureConverters[c] )
		{
			xlDelete m_TextureConverters[c];
		}
		m_TextureConverters[c] = NULL;
	}
	m_pCurConverter=NULL;
}

bool TextureLoader::LoadTexture(u32 uTextureType, u32 uPalIndex, Archive *pTexArchive, const string& sFile, bool bCopyPal)
{
	TextureConverter *pConvert = m_TextureConverters[uTextureType];
	m_pCurConverter = pConvert;

	if ( ArchiveManager::GameFile_Open(pTexArchive, sFile.c_str()) )
	{
		ScratchPad::StartFrame();

		u32 uLen  = ArchiveManager::GameFile_GetLength();
		u8 *pData = (u8 *)ScratchPad::AllocMem( uLen+1 );

		if ( pData )
		{
			ArchiveManager::GameFile_Read(pData, uLen);

			//Run through the texture converter.
			m_pConvertedData = (u8 *)ScratchPad::AllocMem( (uLen+1)*4 + 2048*2048 );
			u32 uHackID = m_pCurConverter->GetHackID( sFile.c_str() );

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

u32 TextureLoader::LoadTexture_TexList(u32 uTextureType, u32 uPalIndex, Archive *pTexArchive, const string& sFile, int nRecord)
{
	u32 uFrameCnt = 1;

	TextureConverter *pConvert = m_TextureConverters[uTextureType];
	m_pCurConverter = pConvert;

	if ( ArchiveManager::GameFile_Open(pTexArchive, sFile.c_str()) )
	{
		ScratchPad::StartFrame();

		u32 uLen  = ArchiveManager::GameFile_GetLength();
		u8 *pData = (u8 *)ScratchPad::AllocMem( uLen+1 );

		if ( pData )
		{
			ArchiveManager::GameFile_Read(pData, uLen);

			//Run through the texture converter.
			m_pConvertedData = (u8 *)ScratchPad::AllocMem( (uLen+1)*4 + 2048*2048 );
			u32 uHackID = m_pCurConverter->GetHackID( sFile.c_str() );

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

bool TextureLoader::LoadTexture_NoArchive(u32 uTextureType, u32 uPalIndex, const string& sFile, bool bCopyPal)
{
	TextureConverter *pConvert = m_TextureConverters[uTextureType];
	m_pCurConverter = pConvert;

	if ( ArchiveManager::File_Open(sFile.c_str()) )
	{
		ScratchPad::StartFrame();

		u32 uLen  = ArchiveManager::File_GetLength();
		u8 *pData = (u8 *)ScratchPad::AllocMem( uLen+1 );

		if ( pData )
		{
			ArchiveManager::File_Read(pData, 0, uLen);

			//Run through the texture converter.
			m_pConvertedData = (u8 *)ScratchPad::AllocMem( (uLen+1)*4 + 2048*2048 );
			u32 uHackID = m_pCurConverter->GetHackID( sFile.c_str() );
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

u32 TextureLoader::LoadTexture_NoArchive_TexList(u32 uTextureType, u32 uPalIndex, const string& sFile, int nRecord)
{
	u32 uFrameCnt = 1;

	TextureConverter *pConvert = m_TextureConverters[uTextureType];
	m_pCurConverter = pConvert;

	if ( ArchiveManager::File_Open(sFile.c_str()) )
	{
		ScratchPad::StartFrame();

		u32 uLen  = ArchiveManager::File_GetLength();
		u8 *pData = (u8 *)ScratchPad::AllocMem( uLen+1 );

		if ( pData )
		{
			ArchiveManager::File_Read(pData, 0, uLen);

			//Run through the texture converter.
			m_pConvertedData = (u8 *)ScratchPad::AllocMem( (uLen+1)*4 + 2048*2048 );
			u32 uHackID = m_pCurConverter->GetHackID( sFile.c_str() );

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

bool TextureLoader::LoadTexture_Mem(const u8 *pImgBuffer, u32 uPalIndex, u32 width, u32 height)
{
	ScratchPad::StartFrame();

	u32 uDataSize = width*height;
	if ( m_uTexColorDepth == 32 ) uDataSize *= 4;
	m_pConvertedData = (u8 *)ScratchPad::AllocMem( uDataSize );
	if ( !(m_pConvertedData != NULL) )
		return false;

	if ( m_uTexColorDepth == 32 )
	{
		for (u32 y=0, yOffs=0; y<height; y++, yOffs+=width*4)
		{
			for (u32 x=0, x4=0; x<width; x++, x4+=4)
			{
				u32 uIndex = pImgBuffer[x+y*width]*4;
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

void TextureLoader::GetTextureSize(s32& nOffsX, s32& nOffsY, u32& uWidth, u32& uHeight)
{
	nOffsX  = m_nOffsX;
	nOffsY  = m_nOffsY;
	uWidth  = m_uWidth;
	uHeight = m_uHeight;
}

u8 *TextureLoader::GetData_RGBA8()
{
	return m_pConvertedData;
}

void TextureLoader::FreeData()
{
	ScratchPad::FreeFrame();
	m_pConvertedData = NULL;
}

void TextureLoader::SetPalette(u8 uPalIndex, u8 *pData, u32 uSize, u32 uTransparentIndex)
{
	u32 uColorCount = uSize/3;
	for (u32 i=0; i<uColorCount; i++)
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

void TextureLoader::SetColormap(u8 uColIndex, u8 *pData, int nLightLevels)
{
	memcpy(m_ColorMap[uColIndex].data, pData, nLightLevels*256);
	m_ColorMap[uColIndex].lightLevels = nLightLevels;
}
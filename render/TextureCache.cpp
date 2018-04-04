#include "TextureCache.h"
#include "IDriver3D.h"
#include "ImageLoader.h"
#include "../math/Math.h"
#include "../fileformats/ArchiveManager.h"
#include "../fileformats/TextureLoader.h"
#include "../memory/ScratchPad.h"
#include <assert.h>
#include <stdlib.h>
#include <memory.h>

ImageLoader *TextureCache::m_pLoader;
IDriver3D *TextureCache::m_pDriver;
TextureCache::TextureMap TextureCache::m_TextureMap;

//dimensions of the previously loaded texture.
s32 TextureCache::m_nPrevTexOffsX = 0;
s32 TextureCache::m_nPrevTexOffsY = 0;
u32 TextureCache::m_uPrevTexWidth = 0;
u32 TextureCache::m_uPrevTexHeight= 0;
f32 TextureCache::m_fRelSizeX     = 1.0f;
f32 TextureCache::m_fRelSizeY	  = 1.0f;
void *TextureCache::m_pPrevExtraData = NULL;
string TextureCache::m_SavedPath;

bool TextureCache::Init(IDriver3D *pDriver)
{
    m_pDriver = pDriver;
    m_pLoader = xlNew ImageLoader();
	m_SavedPath = "";

	if ( m_pLoader == NULL || m_pDriver == NULL )
		return false;

	return true;
}

void TextureCache::Destroy()
{
    if ( m_pLoader )
    {
        xlDelete m_pLoader;
        m_pLoader = NULL;
    }
	m_TextureMap.clear();
}

void TextureCache::SetPath(const string& path)
{
	if ( m_pLoader )
	{
		m_pLoader->SetPath( path.c_str() );
	}
}

void TextureCache::SavePath()
{
	m_SavedPath = m_pLoader->GetPath();
}

void TextureCache::RestorePath()
{
	m_pLoader->SetPath( m_SavedPath.c_str() );
}

TextureHandle TextureCache::LoadTexture(const string& szFile, bool bGenMips)
{
	// Don't load the same texture twice
	TextureMap::iterator iTexMap = m_TextureMap.find(szFile);
    if ( iTexMap != m_TextureMap.end() )
	{
		m_nPrevTexOffsX  = iTexMap->second.nOffsX;
		m_nPrevTexOffsY  = iTexMap->second.nOffsY;
		m_uPrevTexWidth  = iTexMap->second.uWidth;
		m_uPrevTexHeight = iTexMap->second.uHeight;
		m_fRelSizeX      = iTexMap->second.fRelSizeX;
		m_fRelSizeY		 = iTexMap->second.fRelSizeY;
		m_pPrevExtraData = iTexMap->second.pExtraData;

		return iTexMap->second.hHandle;
	}

	// The texture doesn't exist, we can try loading it now...
	if ( m_pLoader->Load_Image( szFile.c_str() ) )
	{
		s32 nOffsX  = m_pLoader->GetOffsetX();
		s32 nOffsY  = m_pLoader->GetOffsetY();
		u32 uWidth  = m_pLoader->GetWidth();
		u32 uHeight = m_pLoader->GetHeight();
		u8 *pData   = m_pLoader->GetImageData();

		//Load the texture into the renderer.
		TextureHandle uTexHandle = m_pDriver->CreateTexture(uWidth, uHeight, IDriver3D::TEX_FORMAT_RGBA8, pData, bGenMips);
		//Texture definition.
		Texture tex;
		tex.hHandle = uTexHandle;
		tex.nOffsX  = nOffsX;
		tex.nOffsY  = nOffsY;
		tex.uWidth  = uWidth;
		tex.uHeight = uHeight;
		tex.fRelSizeX = 1.0f;
		tex.fRelSizeY = 1.0f;
		tex.pExtraData = NULL;

		u32 uDataSize = 0;
		void *pExtraData = TextureLoader::GetTexExtraData(uDataSize);
		if ( uDataSize > 0 )
		{
			tex.pExtraData = malloc(uDataSize);
			memcpy(tex.pExtraData, pExtraData, uDataSize);
		}

		//store previous values so we can get them easily later.
		m_nPrevTexOffsX  = tex.nOffsX;
		m_nPrevTexOffsY  = tex.nOffsY;
		m_uPrevTexWidth  = tex.uWidth;
		m_uPrevTexHeight = tex.uHeight;
		m_fRelSizeX      = tex.fRelSizeX;
		m_fRelSizeY		 = tex.fRelSizeY;
		//Assign it to the map so we only load it once.
		m_TextureMap[szFile] = tex;
		//Return our texture handle
		return uTexHandle;
	}

	return XL_INVALID_TEXTURE;
}

TextureHandle TextureCache::LoadTextureFromMem(const u8 *pImgBuffer, u32 width, u32 height, bool bGenMips)
{
	if ( pImgBuffer )
	{
		//Load the texture into the renderer.
		TextureHandle uTexHandle = m_pDriver->CreateTexture(width, height, IDriver3D::TEX_FORMAT_RGBA8, (u8 *)pImgBuffer, bGenMips);
		//store previous values so we can get them easily later.
		m_uPrevTexWidth  = width;
		m_uPrevTexHeight = height;

		//u32 wr = Math::RoundNextPow2(width), hr = Math::RoundNextPow2(height);
		m_fRelSizeX = 1.0f;//(f32)width  / (f32)wr;
		m_fRelSizeY = 1.0f;//(f32)height / (f32)hr;

		return uTexHandle;
	}

	return XL_INVALID_TEXTURE;
}

TextureHandle TextureCache::LoadTextureFromMem_Pal(const u8 *pImgBuffer, u32 uPalIndex, u32 width, u32 height, const string& sName, bool bGenMips)
{
	// We have to modify the file name a little...
	char szName[32];
	sprintf(szName, "%s_p%d", sName.c_str(), uPalIndex);
	// Don't load the same texture twice
	TextureMap::iterator iTexMap = m_TextureMap.find(szName);
    if ( iTexMap != m_TextureMap.end() )
	{
		m_uPrevTexWidth  = iTexMap->second.uWidth;
		m_uPrevTexHeight = iTexMap->second.uHeight;
		m_fRelSizeX      = iTexMap->second.fRelSizeX;
		m_fRelSizeY		 = iTexMap->second.fRelSizeY;
		m_pPrevExtraData = iTexMap->second.pExtraData;

		return iTexMap->second.hHandle;
	}

	if ( TextureLoader::LoadTexture_Mem( pImgBuffer, uPalIndex, width, height ) )
	{
		u8 *pConvertedData = TextureLoader::GetData_RGBA8();

		//Load the texture into the renderer.
		TextureHandle uTexHandle = m_pDriver->CreateTexture(width, height, IDriver3D::TEX_FORMAT_RGBA8, pConvertedData, bGenMips);

		//Texture definition.
		Texture tex;
		tex.hHandle = uTexHandle;
		tex.uWidth  = width;
		tex.uHeight = height;
		tex.fRelSizeX = 1.0f;
		tex.fRelSizeY = 1.0f;
		tex.pExtraData = NULL;

		u32 uDataSize = 0;
		void *pExtraData = TextureLoader::GetTexExtraData(uDataSize);
		if ( uDataSize > 0 )
		{
			tex.pExtraData = malloc(uDataSize);
			memcpy(tex.pExtraData, pExtraData, uDataSize);
		}

		//store previous values so we can get them easily later.
		m_uPrevTexWidth  = tex.uWidth;
		m_uPrevTexHeight = tex.uHeight;
		m_fRelSizeX      = tex.fRelSizeX;
		m_fRelSizeY		 = tex.fRelSizeY;
		//Assign it to the map so we only load it once.
		m_TextureMap[szName] = tex;

		TextureLoader::FreeData();

		//return the texture
		return uTexHandle;
	}

	return XL_INVALID_TEXTURE;
}

TextureHandle TextureCache::GameFile_LoadTexture_TexList_API(u32 uTextureType, u32 uPalIndex, u32 uArchiveType, const char *pszArchive, const char *pszFile, s32 nRecord, XL_BOOL bGenMips)
{
	return GameFile_LoadTexture_TexList(uTextureType, uPalIndex, uArchiveType, pszArchive, pszFile, nRecord, bGenMips!=0);
}

TextureHandle TextureCache::GameFile_LoadTexture_TexList(u32 uTextureType, u32 uPalIndex, u32 uArchiveType, const string& sArchive, const string& szFile, s32 nRecord, bool bGenMips)
{
	// We have to modify the file name a little...
	char szName[32];
	sprintf(szName, "%s_%d", szFile.c_str(), nRecord);
	// Don't load the same texture twice
	TextureMap::iterator iTexMap = m_TextureMap.find(szName);
    if ( iTexMap != m_TextureMap.end() )
	{
		m_nPrevTexOffsX	 = iTexMap->second.nOffsX;
		m_nPrevTexOffsY	 = iTexMap->second.nOffsY;
		m_uPrevTexWidth  = iTexMap->second.uWidth;
		m_uPrevTexHeight = iTexMap->second.uHeight;
		m_fRelSizeX      = iTexMap->second.fRelSizeX;
		m_fRelSizeY		 = iTexMap->second.fRelSizeY;
		m_pPrevExtraData = iTexMap->second.pExtraData;

		return iTexMap->second.hHandle;
	}
	// Ok, now load it from the archive file.
	if ( uArchiveType != ARCHIVETYPE_NONE )	//File inside an archive format.
	{
		Archive *pTexArchive = ArchiveManager::OpenArchive( uArchiveType, sArchive.c_str() );
		if ( pTexArchive )
		{
			u32 uFrameCnt = TextureLoader::LoadTexture_TexList( uTextureType, uPalIndex, pTexArchive, szFile, nRecord );
			if ( uFrameCnt )
			{
				s32 nOffsX, nOffsY;
				u32 uWidth, uHeight;
				TextureLoader::GetTextureSize(nOffsX, nOffsY, uWidth, uHeight);
				u8 *pConvertedData = TextureLoader::GetData_RGBA8();

				//Load the texture into the renderer.
				TextureHandle uTexHandle = m_pDriver->CreateTexture(uWidth, uHeight, IDriver3D::TEX_FORMAT_RGBA8, pConvertedData, bGenMips, uFrameCnt);

				//Texture definition.
				Texture tex;
				tex.hHandle = uTexHandle;
				tex.nOffsX  = nOffsX;
				tex.nOffsY  = nOffsY;
				tex.uWidth  = uWidth;
				tex.uHeight = uHeight;
				tex.fRelSizeX = 1.0f;
				tex.fRelSizeY = 1.0f;
				tex.pExtraData = NULL;

				u32 uDataSize = 0;
				void *pExtraData = TextureLoader::GetTexExtraData(uDataSize);
				if ( uDataSize > 0 )
				{
					tex.pExtraData = malloc(uDataSize);
					memcpy(tex.pExtraData, pExtraData, uDataSize);
				}

				//store previous values so we can get them easily later.
				m_uPrevTexWidth  = tex.uWidth;
				m_uPrevTexHeight = tex.uHeight;
				m_fRelSizeX      = tex.fRelSizeX;
				m_fRelSizeY		 = tex.fRelSizeY;
				//Assign it to the map so we only load it once.
				m_TextureMap[szName] = tex;

				TextureLoader::FreeData();

				//return the texture
				return uTexHandle;
			}
		}
	}
	else
	{
		u32 uFrameCnt = TextureLoader::LoadTexture_NoArchive_TexList( uTextureType, uPalIndex, szFile, nRecord );
		if ( uFrameCnt )
		{
			s32 nOffsX, nOffsY;
			u32 uWidth, uHeight;
			TextureLoader::GetTextureSize(nOffsX, nOffsY, uWidth, uHeight);
			u8 *pConvertedData = TextureLoader::GetData_RGBA8();

			//Load the texture into the renderer.
			TextureHandle uTexHandle = m_pDriver->CreateTexture(uWidth, uHeight, IDriver3D::TEX_FORMAT_RGBA8, pConvertedData, bGenMips, uFrameCnt);

			//Texture definition.
			Texture tex;
			tex.hHandle = uTexHandle;
			tex.nOffsX  = nOffsX;
			tex.nOffsY  = nOffsY;
			tex.uWidth  = uWidth;
			tex.uHeight = uHeight;
			tex.fRelSizeX = 1.0f;
			tex.fRelSizeY = 1.0f;
			tex.pExtraData = NULL;

			u32 uDataSize = 0;
			void *pExtraData = TextureLoader::GetTexExtraData(uDataSize);
			if ( uDataSize > 0 )
			{
				tex.pExtraData = malloc(uDataSize);
				memcpy(tex.pExtraData, pExtraData, uDataSize);
			}

			//store previous values so we can get them easily later.
			m_uPrevTexWidth  = tex.uWidth;
			m_uPrevTexHeight = tex.uHeight;
			m_fRelSizeX      = tex.fRelSizeX;
			m_fRelSizeY		 = tex.fRelSizeY;
			m_pPrevExtraData = tex.pExtraData;
			//Assign it to the map so we only load it once.
			m_TextureMap[szName] = tex;

			TextureLoader::FreeData();

			//return the texture
			return uTexHandle;
		}
	}

	return XL_INVALID_TEXTURE;
}

TextureHandle TextureCache::GameFile_LoadTexture(u32 uTextureType, u32 uPalIndex, u32 uArchiveType, const string& sArchive, const string& szFile, bool bGenMips, bool bCopyPal)
{
	// We have to modify the file name a little...
	char szName[32];
	sprintf(szName, "%s_p%d", szFile.c_str(), uPalIndex);
	// Don't load the same texture twice
	TextureMap::iterator iTexMap = m_TextureMap.find(szName);
    if ( iTexMap != m_TextureMap.end() )
	{
		m_nPrevTexOffsX	 = iTexMap->second.nOffsX;
		m_nPrevTexOffsY	 = iTexMap->second.nOffsY;
		m_uPrevTexWidth  = iTexMap->second.uWidth;
		m_uPrevTexHeight = iTexMap->second.uHeight;
		m_fRelSizeX      = iTexMap->second.fRelSizeX;
		m_fRelSizeY		 = iTexMap->second.fRelSizeY;
		m_pPrevExtraData = iTexMap->second.pExtraData;

		return iTexMap->second.hHandle;
	}

	// Ok, now load it from the archive file.
	if ( uArchiveType != ARCHIVETYPE_NONE )	//File inside an archive format.
	{
		Archive *pTexArchive = ArchiveManager::OpenArchive( uArchiveType, sArchive.c_str() );
		if ( pTexArchive )
		{
			if ( TextureLoader::LoadTexture( uTextureType, uPalIndex, pTexArchive, szFile, bCopyPal ) )
			{
				s32 nOffsX, nOffsY;
				u32 uWidth, uHeight;
				TextureLoader::GetTextureSize(nOffsX, nOffsY, uWidth, uHeight);
				u8 *pConvertedData = TextureLoader::GetData_RGBA8();

				//Load the texture into the renderer.
				TextureHandle uTexHandle = m_pDriver->CreateTexture(uWidth, uHeight, IDriver3D::TEX_FORMAT_RGBA8, pConvertedData, bGenMips);

				//Texture definition.
				Texture tex;
				tex.hHandle = uTexHandle;
				tex.nOffsX  = nOffsX;
				tex.nOffsY  = nOffsY;
				tex.uWidth  = uWidth;
				tex.uHeight = uHeight;
				tex.fRelSizeX = 1.0f;
				tex.fRelSizeY = 1.0f;
				tex.pExtraData = NULL;

				u32 uDataSize = 0;
				void *pExtraData = TextureLoader::GetTexExtraData(uDataSize);
				if ( uDataSize > 0 )
				{
					tex.pExtraData = malloc(uDataSize);
					memcpy(tex.pExtraData, pExtraData, uDataSize);
				}

				//store previous values so we can get them easily later.
				m_uPrevTexWidth  = tex.uWidth;
				m_uPrevTexHeight = tex.uHeight;
				m_fRelSizeX      = tex.fRelSizeX;
				m_fRelSizeY		 = tex.fRelSizeY;
				//Assign it to the map so we only load it once.
				m_TextureMap[szName] = tex;

				TextureLoader::FreeData();

				//return the texture
				return uTexHandle;
			}
		}
	}
	else	//this is a loose file.
	{
		if ( TextureLoader::LoadTexture_NoArchive( uTextureType, uPalIndex, szFile, bCopyPal ) )
		{
			s32 nOffsX, nOffsY;
			u32 uWidth, uHeight;
			TextureLoader::GetTextureSize(nOffsX, nOffsY, uWidth, uHeight);
			u8 *pConvertedData = TextureLoader::GetData_RGBA8();

			//Load the texture into the renderer.
			TextureHandle uTexHandle = m_pDriver->CreateTexture(uWidth, uHeight, IDriver3D::TEX_FORMAT_RGBA8, pConvertedData, bGenMips);

			//Texture definition.
			Texture tex;
			tex.hHandle = uTexHandle;
			tex.nOffsX  = nOffsX;
			tex.nOffsY  = nOffsY;
			tex.uWidth  = uWidth;
			tex.uHeight = uHeight;
			tex.fRelSizeX = 1.0f;
			tex.fRelSizeY = 1.0f;
			tex.pExtraData = NULL;

			u32 uDataSize = 0;
			void *pExtraData = TextureLoader::GetTexExtraData(uDataSize);
			if ( uDataSize > 0 )
			{
				tex.pExtraData = malloc(uDataSize);
				memcpy(tex.pExtraData, pExtraData, uDataSize);
			}

			//store previous values so we can get them easily later.
			m_uPrevTexWidth  = tex.uWidth;
			m_uPrevTexHeight = tex.uHeight;
			m_fRelSizeX      = tex.fRelSizeX;
			m_fRelSizeY		 = tex.fRelSizeY;
			//Assign it to the map so we only load it once.
			m_TextureMap[szName] = tex;

			TextureLoader::FreeData();

			//return the texture
			return uTexHandle;
		}
	}

	return XL_INVALID_TEXTURE;
}

void TextureCache::FreeTexture(TextureHandle hTex)
{
	// Don't load the same texture twice
	TextureMap::iterator iTexMap = m_TextureMap.begin();
	TextureMap::iterator eTexMap = m_TextureMap.end();

	TextureMap::iterator iTexFound = eTexMap;

	for (; iTexMap != eTexMap; ++iTexMap)
	{
		if ( iTexMap->second.hHandle == hTex )
		{
			iTexFound = iTexMap;
		}
	}
    if ( iTexFound != eTexMap )
	{
		m_TextureMap.erase( iTexFound );
	}

	m_pDriver->FreeTexture( hTex );
}

void TextureCache::GetTextureSize( s32& nOffsX, s32& nOffsY, u32& uTexWidth, u32& uTexHeight, f32& fRelSizeX, f32& fRelSizeY )
{
	nOffsX	   = m_nPrevTexOffsX;
	nOffsY	   = m_nPrevTexOffsY;
	uTexWidth  = m_uPrevTexWidth;
	uTexHeight = m_uPrevTexHeight;
	fRelSizeX  = m_fRelSizeX;
	fRelSizeY  = m_fRelSizeY;
}

void *TextureCache::GetTexExtraData() 
{ 
	return m_pPrevExtraData; 
}

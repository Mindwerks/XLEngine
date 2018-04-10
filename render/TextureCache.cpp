#include "TextureCache.h"
#include "IDriver3D.h"
#include "ImageLoader.h"
#include "../math/Math.h"
#include "../fileformats/ArchiveManager.h"
#include "../fileformats/TextureLoader.h"
#include "../memory/ScratchPad.h"

#include <cassert>
#include <cstdlib>
#include <memory.h>
#include <cstdio>

ImageLoader *TextureCache::m_pLoader;
IDriver3D *TextureCache::m_pDriver;
TextureCache::TextureMap TextureCache::m_TextureMap;

//dimensions of the previously loaded texture.
int32_t TextureCache::m_nPrevTexOffsX = 0;
int32_t TextureCache::m_nPrevTexOffsY = 0;
uint32_t TextureCache::m_uPrevTexWidth = 0;
uint32_t TextureCache::m_uPrevTexHeight= 0;
float TextureCache::m_fRelSizeX     = 1.0f;
float TextureCache::m_fRelSizeY   = 1.0f;
void *TextureCache::m_pPrevExtraData = nullptr;
std::string TextureCache::m_SavedPath;

bool TextureCache::Init(IDriver3D *pDriver)
{
    m_pDriver = pDriver;
    m_pLoader = xlNew ImageLoader();
    m_SavedPath = "";

    if ( m_pLoader == nullptr || m_pDriver == nullptr )
        return false;

    return true;
}

void TextureCache::Destroy()
{
    if ( m_pLoader )
    {
        xlDelete m_pLoader;
        m_pLoader = nullptr;
    }
    m_TextureMap.clear();
}

void TextureCache::SetPath(const std::string& path)
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

TextureHandle TextureCache::LoadTexture(const std::string& szFile, bool bGenMips)
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
        m_fRelSizeY      = iTexMap->second.fRelSizeY;
        m_pPrevExtraData = iTexMap->second.pExtraData;

        return iTexMap->second.hHandle;
    }

    // The texture doesn't exist, we can try loading it now...
    if ( m_pLoader->Load_Image( szFile.c_str() ) )
    {
        int32_t nOffsX  = m_pLoader->GetOffsetX();
        int32_t nOffsY  = m_pLoader->GetOffsetY();
        uint32_t uWidth  = m_pLoader->GetWidth();
        uint32_t uHeight = m_pLoader->GetHeight();
        uint8_t *pData   = m_pLoader->GetImageData();

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
        tex.pExtraData = nullptr;

        uint32_t uDataSize = 0;
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
        m_fRelSizeY      = tex.fRelSizeY;
        //Assign it to the map so we only load it once.
        m_TextureMap[szFile] = tex;
        //Return our texture handle
        return uTexHandle;
    }

    return XL_INVALID_TEXTURE;
}

TextureHandle TextureCache::LoadTextureFromMem(const uint8_t *pImgBuffer, uint32_t width, uint32_t height, bool bGenMips)
{
    if ( pImgBuffer )
    {
        //Load the texture into the renderer.
        TextureHandle uTexHandle = m_pDriver->CreateTexture(width, height, IDriver3D::TEX_FORMAT_RGBA8, (uint8_t *)pImgBuffer, bGenMips);
        //store previous values so we can get them easily later.
        m_uPrevTexWidth  = width;
        m_uPrevTexHeight = height;

        //uint32_t wr = Math::RoundNextPow2(width), hr = Math::RoundNextPow2(height);
        m_fRelSizeX = 1.0f;//(float)width  / (float)wr;
        m_fRelSizeY = 1.0f;//(float)height / (float)hr;

        return uTexHandle;
    }

    return XL_INVALID_TEXTURE;
}

TextureHandle TextureCache::LoadTextureFromMem_Pal(const uint8_t *pImgBuffer, uint32_t uPalIndex, uint32_t width, uint32_t height, const std::string& sName, bool bGenMips)
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
        m_fRelSizeY      = iTexMap->second.fRelSizeY;
        m_pPrevExtraData = iTexMap->second.pExtraData;

        return iTexMap->second.hHandle;
    }

    if ( TextureLoader::LoadTexture_Mem( pImgBuffer, uPalIndex, width, height ) )
    {
        uint8_t *pConvertedData = TextureLoader::GetData_RGBA8();

        //Load the texture into the renderer.
        TextureHandle uTexHandle = m_pDriver->CreateTexture(width, height, IDriver3D::TEX_FORMAT_RGBA8, pConvertedData, bGenMips);

        //Texture definition.
        Texture tex;
        tex.hHandle = uTexHandle;
        tex.uWidth  = width;
        tex.uHeight = height;
        tex.fRelSizeX = 1.0f;
        tex.fRelSizeY = 1.0f;
        tex.pExtraData = nullptr;

        uint32_t uDataSize = 0;
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
        m_fRelSizeY      = tex.fRelSizeY;
        //Assign it to the map so we only load it once.
        m_TextureMap[szName] = tex;

        TextureLoader::FreeData();

        //return the texture
        return uTexHandle;
    }

    return XL_INVALID_TEXTURE;
}

TextureHandle TextureCache::GameFile_LoadTexture_TexList_API(uint32_t uTextureType, uint32_t uPalIndex, uint32_t uArchiveType, const char *pszArchive, const char *pszFile, int32_t nRecord, XL_BOOL bGenMips)
{
    return GameFile_LoadTexture_TexList(uTextureType, uPalIndex, uArchiveType, pszArchive, pszFile, nRecord, bGenMips!=0);
}

TextureHandle TextureCache::GameFile_LoadTexture_TexList(uint32_t uTextureType, uint32_t uPalIndex, uint32_t uArchiveType, const std::string& sArchive, const std::string& szFile, int32_t nRecord, bool bGenMips)
{
    // We have to modify the file name a little...
    char szName[32];
    sprintf(szName, "%s_%d", szFile.c_str(), nRecord);
    // Don't load the same texture twice
    TextureMap::iterator iTexMap = m_TextureMap.find(szName);
    if ( iTexMap != m_TextureMap.end() )
    {
        m_nPrevTexOffsX  = iTexMap->second.nOffsX;
        m_nPrevTexOffsY  = iTexMap->second.nOffsY;
        m_uPrevTexWidth  = iTexMap->second.uWidth;
        m_uPrevTexHeight = iTexMap->second.uHeight;
        m_fRelSizeX      = iTexMap->second.fRelSizeX;
        m_fRelSizeY      = iTexMap->second.fRelSizeY;
        m_pPrevExtraData = iTexMap->second.pExtraData;

        return iTexMap->second.hHandle;
    }
    // Ok, now load it from the archive file.
    if ( uArchiveType != ARCHIVETYPE_NONE ) //File inside an archive format.
    {
        Archive *pTexArchive = ArchiveManager::OpenArchive( uArchiveType, sArchive.c_str() );
        if ( pTexArchive )
        {
            uint32_t uFrameCnt = TextureLoader::LoadTexture_TexList( uTextureType, uPalIndex, pTexArchive, szFile, nRecord );
            if ( uFrameCnt )
            {
                int32_t nOffsX, nOffsY;
                uint32_t uWidth, uHeight;
                TextureLoader::GetTextureSize(nOffsX, nOffsY, uWidth, uHeight);
                uint8_t *pConvertedData = TextureLoader::GetData_RGBA8();

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
                tex.pExtraData = nullptr;

                uint32_t uDataSize = 0;
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
                m_fRelSizeY      = tex.fRelSizeY;
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
        uint32_t uFrameCnt = TextureLoader::LoadTexture_NoArchive_TexList( uTextureType, uPalIndex, szFile, nRecord );
        if ( uFrameCnt )
        {
            int32_t nOffsX, nOffsY;
            uint32_t uWidth, uHeight;
            TextureLoader::GetTextureSize(nOffsX, nOffsY, uWidth, uHeight);
            uint8_t *pConvertedData = TextureLoader::GetData_RGBA8();

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
            tex.pExtraData = nullptr;

            uint32_t uDataSize = 0;
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
            m_fRelSizeY      = tex.fRelSizeY;
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

TextureHandle TextureCache::GameFile_LoadTexture(uint32_t uTextureType, uint32_t uPalIndex, uint32_t uArchiveType, const std::string& sArchive, const std::string& szFile, bool bGenMips, bool bCopyPal)
{
    // We have to modify the file name a little...
    char szName[32];
    sprintf(szName, "%s_p%d", szFile.c_str(), uPalIndex);
    // Don't load the same texture twice
    TextureMap::iterator iTexMap = m_TextureMap.find(szName);
    if ( iTexMap != m_TextureMap.end() )
    {
        m_nPrevTexOffsX  = iTexMap->second.nOffsX;
        m_nPrevTexOffsY  = iTexMap->second.nOffsY;
        m_uPrevTexWidth  = iTexMap->second.uWidth;
        m_uPrevTexHeight = iTexMap->second.uHeight;
        m_fRelSizeX      = iTexMap->second.fRelSizeX;
        m_fRelSizeY      = iTexMap->second.fRelSizeY;
        m_pPrevExtraData = iTexMap->second.pExtraData;

        return iTexMap->second.hHandle;
    }

    // Ok, now load it from the archive file.
    if ( uArchiveType != ARCHIVETYPE_NONE ) //File inside an archive format.
    {
        Archive *pTexArchive = ArchiveManager::OpenArchive( uArchiveType, sArchive.c_str() );
        if ( pTexArchive )
        {
            if ( TextureLoader::LoadTexture( uTextureType, uPalIndex, pTexArchive, szFile, bCopyPal ) )
            {
                int32_t nOffsX, nOffsY;
                uint32_t uWidth, uHeight;
                TextureLoader::GetTextureSize(nOffsX, nOffsY, uWidth, uHeight);
                uint8_t *pConvertedData = TextureLoader::GetData_RGBA8();

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
                tex.pExtraData = nullptr;

                uint32_t uDataSize = 0;
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
                m_fRelSizeY      = tex.fRelSizeY;
                //Assign it to the map so we only load it once.
                m_TextureMap[szName] = tex;

                TextureLoader::FreeData();

                //return the texture
                return uTexHandle;
            }
        }
    }
    else    //this is a loose file.
    {
        if ( TextureLoader::LoadTexture_NoArchive( uTextureType, uPalIndex, szFile, bCopyPal ) )
        {
            int32_t nOffsX, nOffsY;
            uint32_t uWidth, uHeight;
            TextureLoader::GetTextureSize(nOffsX, nOffsY, uWidth, uHeight);
            uint8_t *pConvertedData = TextureLoader::GetData_RGBA8();

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
            tex.pExtraData = nullptr;

            uint32_t uDataSize = 0;
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
            m_fRelSizeY      = tex.fRelSizeY;
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

void TextureCache::GetTextureSize( int32_t& nOffsX, int32_t& nOffsY, uint32_t& uTexWidth, uint32_t& uTexHeight, float& fRelSizeX, float& fRelSizeY )
{
    nOffsX     = m_nPrevTexOffsX;
    nOffsY     = m_nPrevTexOffsY;
    uTexWidth  = m_uPrevTexWidth;
    uTexHeight = m_uPrevTexHeight;
    fRelSizeX  = m_fRelSizeX;
    fRelSizeY  = m_fRelSizeY;
}

void *TextureCache::GetTexExtraData() 
{ 
    return m_pPrevExtraData; 
}

#ifndef TEXTURECACHE_H
#define TEXTURECACHE_H

#include "../CommonTypes.h"

#include <string>
#include <map>

using namespace std;

class ImageLoader;
class IDriver3D;

class TextureCache
{
    struct Texture
    {
        TextureHandle hHandle;
        int32_t nOffsX;
        int32_t nOffsY;
        uint32_t uWidth;
        uint32_t uHeight;
        float fRelSizeX;
        float fRelSizeY;
        void *pExtraData;
    };
    typedef map<string, Texture> TextureMap;

public:
    static bool Init(IDriver3D *pDriver);
    static void Destroy();
    static void SetPath(const string& path);
    static void SavePath();
    static void RestorePath();

    static TextureHandle LoadTexture(const string& szFile, bool bGenMips=true);
    static TextureHandle GameFile_LoadTexture(uint32_t uTextureType, uint32_t uPalIndex, uint32_t uArchiveType, const string& sArchive, const string& szFile, bool bGenMips=true, bool bCopyPal=false);
    static TextureHandle GameFile_LoadTexture_TexList(uint32_t uTextureType, uint32_t uPalIndex, uint32_t uArchiveType, const string& sArchive, const string& szFile, int32_t nRecord, bool bGenMips=true);
    static TextureHandle GameFile_LoadTexture_TexList_API(uint32_t uTextureType, uint32_t uPalIndex, uint32_t uArchiveType, const char *pszArchive, const char *pszFile, int32_t nRecord, XL_BOOL bGenMips);
    static TextureHandle LoadTextureFromMem_Pal(const uint8_t *pImgBuffer, uint32_t uPalIndex, uint32_t width, uint32_t height, const string& sName, bool bGenMips=true);
    static TextureHandle LoadTextureFromMem(const uint8_t *pImgBuffer, uint32_t width, uint32_t height, bool bGenMips=true);
    static void FreeTexture(TextureHandle hTex);
    static void GetTextureSize( int32_t& nOffsX, int32_t& nOffsY, uint32_t& uTexWidth, uint32_t& uTexHeight, float& fRelSizeX, float& fRelSizeY );
    static void *GetTexExtraData();

private:
    static ImageLoader *m_pLoader;
    static IDriver3D *m_pDriver;

    static string m_SavedPath;

    static TextureMap m_TextureMap;
    //dimensions of the previously loaded texture.
    static int32_t m_nPrevTexOffsX;
    static int32_t m_nPrevTexOffsY;
    static uint32_t m_uPrevTexWidth;
    static uint32_t m_uPrevTexHeight;
    static float m_fRelSizeX;
    static float m_fRelSizeY;
    static void *m_pPrevExtraData;
};

#endif //TEXTURECACHE_H

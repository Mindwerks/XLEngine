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
		s32 nOffsX;
		s32 nOffsY;
		u32 uWidth;
		u32 uHeight;
		f32 fRelSizeX;
		f32 fRelSizeY;
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
	static TextureHandle GameFile_LoadTexture(u32 uTextureType, u32 uPalIndex, u32 uArchiveType, const string& sArchive, const string& szFile, bool bGenMips=true, bool bCopyPal=false);
	static TextureHandle GameFile_LoadTexture_TexList(u32 uTextureType, u32 uPalIndex, u32 uArchiveType, const string& sArchive, const string& szFile, s32 nRecord, bool bGenMips=true);
	static TextureHandle GameFile_LoadTexture_TexList_API(u32 uTextureType, u32 uPalIndex, u32 uArchiveType, const char *pszArchive, const char *pszFile, s32 nRecord, XL_BOOL bGenMips);
	static TextureHandle LoadTextureFromMem_Pal(const u8 *pImgBuffer, u32 uPalIndex, u32 width, u32 height, const string& sName, bool bGenMips=true);
	static TextureHandle LoadTextureFromMem(const u8 *pImgBuffer, u32 width, u32 height, bool bGenMips=true);
	static void FreeTexture(TextureHandle hTex);
	static void GetTextureSize( s32& nOffsX, s32& nOffsY, u32& uTexWidth, u32& uTexHeight, f32& fRelSizeX, f32& fRelSizeY );
	static void *GetTexExtraData();

private:
	static ImageLoader *m_pLoader;
	static IDriver3D *m_pDriver;

	static string m_SavedPath;

	static TextureMap m_TextureMap;
	//dimensions of the previously loaded texture.
	static s32 m_nPrevTexOffsX;
	static s32 m_nPrevTexOffsY;
	static u32 m_uPrevTexWidth;
	static u32 m_uPrevTexHeight;
	static f32 m_fRelSizeX;
	static f32 m_fRelSizeY;
	static void *m_pPrevExtraData;
};

#endif //TEXTURECACHE_H

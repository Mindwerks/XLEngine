#ifndef IMAGELOADER_H
#define IMAGELOADER_H

#include "../CommonTypes.h"

#include <vector>
using namespace std;

class ImageLoader
{
public:
	ImageLoader();
	~ImageLoader();

	bool Load_Image(const char *pszImage);
	void FreeImageData();
	u8 *GetImageData() { return m_pImageData; }
	u32 GetWidth()  { return m_uWidth;  }
	u32 GetHeight() { return m_uHeight; }
	u32 GetOffsetX(){ return m_uOffsX; }
	u32 GetOffsetY(){ return m_uOffsY; }

	bool Save_ImageRGBA(const char *pszImage, u8 *pData, u32 uWidth, u32 uHeight);

	void SetPath(const char *pszPath);
	const char *GetPath() { return m_szPath; }
private:
	char m_szPath[260];
	u32 m_uWidth;
	u32 m_uHeight;
	u32 m_uOffsX;
	u32 m_uOffsY;
	u8 *m_pImageData;
	u8 *m_pImageData_Work;
};

#endif //IMAGELOADER_H
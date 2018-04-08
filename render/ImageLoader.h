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
    uint8_t *GetImageData() { return m_pImageData; }
    uint32_t GetWidth()  { return m_uWidth;  }
    uint32_t GetHeight() { return m_uHeight; }
    uint32_t GetOffsetX(){ return m_uOffsX; }
    uint32_t GetOffsetY(){ return m_uOffsY; }

    bool Save_ImageRGBA(const char *pszImage, uint8_t *pData, uint32_t uWidth, uint32_t uHeight);

    void SetPath(const char *pszPath);
    const char *GetPath() { return m_szPath; }
private:
    char m_szPath[260];
    uint32_t m_uWidth;
    uint32_t m_uHeight;
    uint32_t m_uOffsX;
    uint32_t m_uOffsY;
    uint8_t *m_pImageData;
    uint8_t *m_pImageData_Work;
};

#endif //IMAGELOADER_H
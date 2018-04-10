#ifndef ART_READER_H
#define ART_READER_H

#include "../CommonTypes.h"
#include "Archive.h"
#include <cstdio>

class ART_Reader : public Archive
{
public:
    ART_Reader();

    virtual bool Open(const char *pszName) override;
    virtual void Close() override;

    virtual bool OpenFile(const char *pszFile) override;
    virtual void CloseFile() override;
    virtual uint32_t GetFileLen() override;
    virtual bool ReadFile(void *pData, uint32_t uLength) override;

    virtual int32_t GetFileCount() override;
    virtual const char *GetFileName(int32_t nFileIdx) override;

    virtual void *ReadFileInfo() override;

private:

#pragma pack(push)
#pragma pack(1)

    // Description of a tile
    struct Tile
    {
       uint16_t XSize;     // Tile's width
       uint16_t YSize;     // Tile's height
       uint32_t AnimData;  // Flags and values for animating the picture
       uint32_t Offset;    // Offset in the ART file
    };

#pragma pack(pop)

    Tile *m_pTilesList;
    uint32_t m_uFileCount;
    uint32_t m_uTilesStartNum;
    int32_t m_CurFile;

    FILE *m_pFile;
    char m_szFileName[64];
};

#endif //ART_READER_H
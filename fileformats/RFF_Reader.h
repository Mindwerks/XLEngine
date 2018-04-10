#ifndef RFF_READER_H
#define RFF_READER_H

#include "../CommonTypes.h"
#include "Archive.h"
#include <cstdio>

class RFF_Reader : public Archive
{
public:
    RFF_Reader();

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

    struct RFF_File
    {
        char szName[32];
        uint32_t offset;
        uint32_t length;
        uint32_t flags;
    };

#pragma pack(pop)

    RFF_File *m_pFileList;
    uint32_t m_uFileCount;
    int32_t m_CurFile;

    FILE *m_pFile;
    FILE *m_pFileLocal;
    char m_szFileName[64];
};

#endif //RFF_READER_H
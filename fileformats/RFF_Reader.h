#ifndef RFF_READER_H
#define RFF_READER_H

#include "../CommonTypes.h"
#include "Archive.h"
#include "Vfs.h"
#include <string>
#include <cstdio>

class RFF_Reader : public Archive
{
public:
    RFF_Reader();

    bool Open(const char *pszName) override;
    void Close() override;

    bool OpenFile(const char *pszFile) override;
    void CloseFile() override;
    uint32_t GetFileLen() override;
    bool ReadFile(void *pData, uint32_t uLength) override;

    int32_t GetFileCount() override;
    const char *GetFileName(int32_t nFileIdx) override;

    void *ReadFileInfo() override;

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

    istream_ptr mFile;
    istream_ptr mFileLocal;
    std::string mFileName;
};

#endif //RFF_READER_H

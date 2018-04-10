#ifndef BSA_READER_H
#define BSA_READER_H

#include "../CommonTypes.h"
#include "Archive.h"
#include <cstdio>

class BSA_Reader : public Archive
{
public:
    BSA_Reader();

    virtual bool Open(const char *pszName) override;
    virtual void Close() override;

    virtual bool OpenFile(const char *pszFile) override;
    virtual bool OpenFile(const uint32_t uID) override;
    virtual bool SearchForFile(const char *pszFileIn, char *pszFileOut) override;
    virtual void CloseFile() override;
    virtual uint32_t GetFileLen() override;
    virtual bool ReadFile(void *pData, uint32_t uLength) override;

    virtual int32_t GetFileCount() override;
    virtual const char *GetFileName(int32_t nFileIdx) override;
    virtual uint32_t GetFileID(int32_t nFileIdx) override;

private:

#pragma pack(push)
#pragma pack(1)

    enum DirectoryType_e
    {
        DT_NameRecord   = 0x0100,
        DT_NumberRecord = 0x0200,
    };

    struct BSA_Header
    {
        int16_t DirectoryCount;
        uint16_t DirectoryType;
    };

    struct BSA_EntryName
    {
        char  NAME[14]; //file name.
        int32_t RecordSize;
    };

    struct BSA_EntryNum
    {
        uint32_t RecordID;
        int32_t RecordSize;
    };
#pragma pack(pop)

    BSA_Header     m_Header;
    BSA_EntryName *m_pFileListName;
    BSA_EntryNum  *m_pFileListNum;
    int32_t            m_CurFile;

    FILE *m_pFile;
    char m_szFileName[64];
};

#endif //BSA_READER_H
#ifndef GOB_READER_H
#define GOB_READER_H

#include "../CommonTypes.h"
#include "Archive.h"
#include <cstdio>
#include <cstdint>

class GOB_Reader : public Archive
{
public:
    GOB_Reader();

    bool Open(const char *pszName) override;
    void Close() override;

    bool OpenFile(const char *pszFile) override;
    void CloseFile() override;
    uint32_t GetFileLen() override;
    bool ReadFile(void *pData, uint32_t uLength) override;

    int32_t GetFileCount() override;
    const char *GetFileName(int32_t nFileIdx) override;

private:

#pragma pack(push)
#pragma pack(1)

    typedef struct
    {
        char GOB_MAGIC[4];
        int32_t MASTERX;    //offset to GOX_Index_t
    } GOB_Header_t;

    typedef struct
    {
        int32_t IX;     //offset to the start of the file.
        int32_t LEN;        //length of the file.
        char NAME[13];  //file name.
    } GOB_Entry_t;

    typedef struct
    {
        int32_t MASTERN;    //num files
        GOB_Entry_t *pEntries;
    } GOB_Index_t;

#pragma pack(pop)

    GOB_Header_t m_Header;
    GOB_Index_t m_FileList;
    int32_t m_CurFile;
    bool m_bGOB;

    FILE *m_pFile;
    char m_szFileName[64];
};

#endif //GOB_READER_H
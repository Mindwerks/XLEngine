#ifndef LFD_READER_H
#define LFD_READER_H

#include "../CommonTypes.h"
#include "Archive.h"
#include "Vfs.h"
#include <string>
#include <cstdio>

class LFD_Reader : public Archive
{
public:
    LFD_Reader();

    bool Open(const char *pszName) override;
    void Close() override;

    bool OpenFile(const char *pszFile) override;
    void CloseFile() override;
    uint32_t GetFileLen() override;
    bool ReadFile(void *pData, uint32_t uLength) override;

private:

    typedef struct
    {
        char TYPE[5];
        char NAME[9];
        int32_t LENGTH;        //length of the file.
        int32_t IX;
    } LFD_Entry_t;

    typedef struct
    {
        long MASTERN;   //num files
        LFD_Entry_t *pEntries;
    } LFD_Index_t;

    LFD_Entry_t m_Header;
    LFD_Index_t m_FileList;
    int32_t m_CurFile;

    istream_ptr mFile;
    std::string mFileName;
};

#endif //LFD_READER_H

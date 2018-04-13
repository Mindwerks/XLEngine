#ifndef ARCHIVEMANAGER_H
#define ARCHIVEMANAGER_H

#include "../CommonTypes.h"
#include "ArchiveTypes.h"
#include "Vfs.h"

#include <string>
#include <vector>
#include <map>

class Archive;

class ArchiveManager
{
public:
    static void Init();
    static void Destroy();

    static Archive *OpenArchive(uint32_t uArchiveType, const char *pszArchiveName);
    static void CloseArchive(Archive *pArchive);

    static int32_t GameFile_Open(Archive *pArchive, const char *pszFileName);
    static int32_t GameFile_Open(uint32_t uArchiveType, const char *pszArchiveName, const char *pszFileName);
    static int32_t GameFile_Open(uint32_t uArchiveType, const char *pszArchiveName, uint32_t fileID);
    static int32_t GameFile_SearchForFile(uint32_t uArchiveType, const char *pszArchiveName, const char *pszFileName, char *pszFileOut);
    static uint32_t GameFile_GetLength();
    static void GameFile_Read(void *pData, uint32_t uLength);
    static void *GameFile_GetFileInfo();
    static void GameFile_Close();

    static int32_t File_Open(const char *pszFileName);
    static uint32_t File_GetLength();
    static void File_Read(void *pData, uint32_t uStart, uint32_t uLength);
    static void *File_GetFileInfo();
    static void File_Close();

private:
    static const char *m_apszArchiveExt[];
    static std::map<std::string, Archive *> m_OpenArchives;
    static std::vector<Archive *> m_ArchiveList;
    static Archive *m_pCurArchive;
    static istream_ptr sCurrentSysFile;
};

//helper defines.
#define READ_uint32_t(p, i) *((uint32_t *)&p[i]); i += 4
#define READ_int32_t(p, i) *((int32_t *)&p[i]); i += 4
#define READ_uint16_t(p, i) *((uint16_t *)&p[i]); i += 2
#define READ_int16_t(p, i) *((int16_t *)&p[i]); i += 2
#define READ_uint8_t(p, i) *((uint8_t *)&p[i]); i++
#define READ_int8_t(p, i) *((int8_t *)&p[i]); i++
#define READ_DATA(d, i, s) memcpy(d, &pData[index], s); i+=s

#endif //ARCHIVEMANAGER_H

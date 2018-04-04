#ifndef ARCHIVEMANAGER_H
#define ARCHIVEMANAGER_H

#include "../CommonTypes.h"
#include "ArchiveTypes.h"
#include <string>
#include <vector>
#include <map>

using namespace std;
class Archive;

class ArchiveManager
{
public:
	static void Init();
	static void Destroy();

	static Archive *OpenArchive(u32 uArchiveType, const char *pszArchiveName);
	static void CloseArchive(Archive *pArchive);

	static s32 GameFile_Open(Archive *pArchive, const char *pszFileName);
	static s32 GameFile_Open(u32 uArchiveType, const char *pszArchiveName, const char *pszFileName);
	static s32 GameFile_Open(u32 uArchiveType, const char *pszArchiveName, u32 fileID);
	static s32 GameFile_SearchForFile(u32 uArchiveType, const char *pszArchiveName, const char *pszFileName, char *pszFileOut);
	static u32 GameFile_GetLength();
	static void GameFile_Read(void *pData, u32 uLength);
	static void *GameFile_GetFileInfo();
	static void GameFile_Close();

	static s32 File_Open(const char *pszFileName);
	static u32 File_GetLength();
	static void File_Read(void *pData, u32 uStart, u32 uLength);
	static void *File_GetFileInfo();
	static void File_Close();

private:
	static const char *m_apszArchiveExt[];
	static map<string, Archive *> m_OpenArchives;
	static vector<Archive *> m_ArchiveList;
	static Archive *m_pCurArchive;
	static FILE *s_pCurrentSysFile;
};

//helper defines.
#define READ_U32(p, i) *((u32 *)&p[i]); i += 4
#define READ_S32(p, i) *((s32 *)&p[i]); i += 4
#define READ_U16(p, i) *((u16 *)&p[i]); i += 2
#define READ_S16(p, i) *((s16 *)&p[i]); i += 2
#define READ_U8(p, i) *((u8 *)&p[i]); i++
#define READ_S8(p, i) *((s8 *)&p[i]); i++
#define READ_DATA(d, i, s) memcpy(d, &pData[index], s); i+=s

#endif //ARCHIVEMANAGER_H
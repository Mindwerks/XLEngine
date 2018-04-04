#ifndef BSA_READER_H
#define BSA_READER_H

#include "../CommonTypes.h"
#include "Archive.h"
#include <stdio.h>

class BSA_Reader : public Archive
{
public:
	BSA_Reader();

	bool Open(const char *pszName);
	void Close();

	bool OpenFile(const char *pszFile);
	bool OpenFile(const u32 uID);
	bool SearchForFile(const char *pszFileIn, char *pszFileOut);
	void CloseFile();
	u32 GetFileLen();
	bool ReadFile(void *pData, u32 uLength);

	s32 GetFileCount();
	const char *GetFileName(s32 nFileIdx);
	u32 GetFileID(s32 nFileIdx);

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
		s16 DirectoryCount;
		u16 DirectoryType;
	};

	struct BSA_EntryName
	{
		s8  NAME[14];	//file name.
		s32 RecordSize;
	};

	struct BSA_EntryNum
	{
		u32 RecordID;
		s32 RecordSize;
	};
#pragma pack(pop)

	BSA_Header     m_Header;
	BSA_EntryName *m_pFileListName;
	BSA_EntryNum  *m_pFileListNum;
	s32			   m_CurFile;

	FILE *m_pFile;
	char m_szFileName[64];
};

#endif //BSA_READER_H
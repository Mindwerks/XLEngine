#ifndef RFF_READER_H
#define RFF_READER_H

#include "../CommonTypes.h"
#include "Archive.h"
#include <stdio.h>

class RFF_Reader : public Archive
{
public:
	RFF_Reader();

	bool Open(const char *pszName);
	void Close();

	bool OpenFile(const char *pszFile);
	void CloseFile();
	u32 GetFileLen();
	bool ReadFile(void *pData, u32 uLength);

	s32 GetFileCount();
	const char *GetFileName(s32 nFileIdx);

	void *ReadFileInfo();

private:

#pragma pack(push)
#pragma pack(1)

	struct RFF_File
	{
		char szName[32];
		u32 offset;
		u32 length;
		u32 flags;
	};

#pragma pack(pop)

	RFF_File *m_pFileList;
	u32 m_uFileCount;
	s32 m_CurFile;

	FILE *m_pFile;
	FILE *m_pFileLocal;
	char m_szFileName[64];
};

#endif //RFF_READER_H
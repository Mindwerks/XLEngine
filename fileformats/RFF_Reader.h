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
	uint32_t GetFileLen();
	bool ReadFile(void *pData, uint32_t uLength);

	int32_t GetFileCount();
	const char *GetFileName(int32_t nFileIdx);

	void *ReadFileInfo();

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
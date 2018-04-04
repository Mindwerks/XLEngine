#ifndef LFD_READER_H
#define LFD_READER_H

#include "../CommonTypes.h"
#include "Archive.h"
#include <stdio.h>

class LFD_Reader : public Archive
{
public:
	LFD_Reader();

	bool Open(const char *pszName);
	void Close();

	bool OpenFile(const char *pszFile);
	void CloseFile();
	u32 GetFileLen();
	bool ReadFile(void *pData, u32 uLength);

private:

//#pragma pack() operations are supported in modern versions of GCC.
//So this should be valid code on Linux and OS X.
#pragma pack(push)
#pragma pack(1)

	typedef struct
	{
		char TYPE[4];
		char NAME[8];
		long LENGTH;		//length of the file.
	} LFD_Entry_t;

	typedef struct
	{
		char TYPE[5];
		char NAME[9];
		long LENGTH;		//length of the file.
		long IX;
	} LFD_EntryFinal_t;

	typedef struct
	{
		long MASTERN;	//num files
		LFD_EntryFinal_t *pEntries;
	} LFD_Index_t;

#pragma pack(pop)

	LFD_Entry_t m_Header;
	LFD_Index_t m_FileList;
	s32 m_CurFile;

	FILE *m_pFile;
	char m_szFileName[64];
};

#endif //LFD_READER_H
#ifndef ART_READER_H
#define ART_READER_H

#include "../CommonTypes.h"
#include "Archive.h"
#include <stdio.h>

class ART_Reader : public Archive
{
public:
	ART_Reader();

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

	// Description of a tile
	struct Tile
	{
	   u16 XSize;     // Tile's width
	   u16 YSize;     // Tile's height
	   u32 AnimData;  // Flags and values for animating the picture
	   u32 Offset;    // Offset in the ART file
	};

#pragma pack(pop)

	Tile *m_pTilesList;
	u32 m_uFileCount;
	u32 m_uTilesStartNum;
	s32 m_CurFile;

	FILE *m_pFile;
	char m_szFileName[64];
};

#endif //ART_READER_H
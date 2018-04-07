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
	uint32_t GetFileLen();
	bool ReadFile(void *pData, uint32_t uLength);

	int32_t GetFileCount();
	const char *GetFileName(int32_t nFileIdx);

	void *ReadFileInfo();

private:

#pragma pack(push)
#pragma pack(1)

	// Description of a tile
	struct Tile
	{
	   uint16_t XSize;     // Tile's width
	   uint16_t YSize;     // Tile's height
	   uint32_t AnimData;  // Flags and values for animating the picture
	   uint32_t Offset;    // Offset in the ART file
	};

#pragma pack(pop)

	Tile *m_pTilesList;
	uint32_t m_uFileCount;
	uint32_t m_uTilesStartNum;
	int32_t m_CurFile;

	FILE *m_pFile;
	char m_szFileName[64];
};

#endif //ART_READER_H
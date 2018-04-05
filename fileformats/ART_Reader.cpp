#include "ART_Reader.h"
#include "../EngineSettings.h"
#include "../ui/XL_Console.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define MAX_NB_TILES 9216
#define PALETTE_SIZE (256 * 3)

char _szFileName[32];

ART_Reader::ART_Reader() : Archive()
{
	m_CurFile = -1;
	m_pFile = NULL;
	m_pTilesList = NULL;
	m_uFileCount = 0;
}

bool ART_Reader::Open(const char *pszName)
{
	sprintf(m_szFileName, "%s%s", EngineSettings::GetGameDataDir(), pszName);

	FILE *f = fopen(m_szFileName, "rb");
	if ( f )
	{
		// Variables
		static u8 Buffer[MAX_NB_TILES * 4];
		u32 uVersion, uTilesEndNum;
		u32 Ind;
		size_t CrtOffset;

		// Read the first part of the header
		if ( fread(Buffer, 1, 16, f) != 16 )
		{
			XL_Console::PrintF("^1Error: invalid ART file: not enough header data");
			return false;
		}
		uVersion         = *( (u32 *)(&Buffer[ 0]) );
		m_uFileCount     = *( (u32 *)(&Buffer[ 4]) );
		m_uTilesStartNum = *( (u32 *)(&Buffer[ 8]) );
		uTilesEndNum     = *( (u32 *)(&Buffer[12]) );

		// Compute the real number of tiles contained in the file
		m_uFileCount = uTilesEndNum - m_uTilesStartNum + 1;

		m_pTilesList = xlNew Tile[m_uFileCount];

		// Check the version number
		if (uVersion != 1)
		{
			XL_Console::PrintF("^1Error: invalid ART file: invalid version number (%u)", uVersion);
			return false;
		}

		// Extract sizes
		fread(Buffer, 1, m_uFileCount*2, f);
		for (Ind = 0; Ind < m_uFileCount; Ind++)
			m_pTilesList[Ind].XSize = *( (u16 *)(&Buffer[Ind * 2]) );

		fread(Buffer, 1, m_uFileCount * 2, f);
		for (Ind = 0; Ind < m_uFileCount; Ind++)
			m_pTilesList[Ind].YSize = *( (u16 *)(&Buffer[Ind * 2]) );

		// Extract animation data
		fread(Buffer, 1, m_uFileCount * 4, f);
		for (Ind = 0; Ind < m_uFileCount; Ind++)
			m_pTilesList[Ind].AnimData = *( (u32 *)(&Buffer[Ind * 4]) );

		// Compute offsets
		CrtOffset = 16 + m_uFileCount * (2 + 2 + 4);
		for (Ind = 0; Ind < m_uFileCount; Ind++)
		{
			m_pTilesList[Ind].Offset = CrtOffset;
			CrtOffset += m_pTilesList[Ind].XSize * m_pTilesList[Ind].YSize;
		}

		fclose(f);
		m_bOpen = true;

		return true;
	}
	XL_Console::PrintF("^1Error: Failed to load %s", m_szFileName);

	return false;
}

void ART_Reader::Close()
{
	CloseFile();
	if ( m_pTilesList )
	{
		xlDelete [] m_pTilesList;
	}
	m_bOpen = false;
}

//This is different then other archives, we're reading indices here...
bool ART_Reader::OpenFile(const char *pszFile)
{
	m_pFile = fopen(m_szFileName, "rb");
	m_CurFile = -1;
    
	if ( m_pFile )
	{
		//convert from the incoming index into a file index.
		char *tmp;
		s32 nIndex = (s32)strtol(pszFile, &tmp, 10);
		//subtract the start out the start tile.
		nIndex -= m_uTilesStartNum;

		if ( nIndex >= 0 && nIndex < (s32)m_uFileCount )
		{
			m_CurFile = nIndex;
		}

		if ( m_CurFile == -1 )
		{
			XL_Console::PrintF("^1Error: Failed to load %s from \"%s\"", pszFile, m_szFileName);
		}
	}

	return m_CurFile > -1 ? true : false;
}

void ART_Reader::CloseFile()
{
	if ( m_pFile )
	{
		fclose(m_pFile);
		m_pFile = NULL;
	}
	m_CurFile = -1;
}

u32 ART_Reader::GetFileLen()
{
	return m_pTilesList[m_CurFile].XSize * m_pTilesList[m_CurFile].YSize;
}

bool ART_Reader::ReadFile(void *pData, u32 uLength)
{
	if ( !m_pFile ) { return false; }

	fseek(m_pFile, m_pTilesList[m_CurFile].Offset, SEEK_SET);
	if ( uLength == 0 )
		uLength = GetFileLen();

	fread(pData, uLength, 1, m_pFile);

	return true;
}

//This is needed for this particular archive because it contains size info for the tile.
void *ART_Reader::ReadFileInfo()
{
	return (void *)&m_pTilesList[m_CurFile];
}

s32 ART_Reader::GetFileCount()
{
	return (s32)m_uFileCount;
}

const char *ART_Reader::GetFileName(s32 nFileIdx)
{
	sprintf(_szFileName, "%03d", nFileIdx+m_uTilesStartNum);
	return _szFileName;
}

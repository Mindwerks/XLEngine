#include "ART_Reader.h"
#include "../EngineSettings.h"
#include "../ui/XL_Console.h"
#include <cstring>
#include <cstdio>
#include <cstdlib>

#define MAX_NB_TILES 9216
#define PALETTE_SIZE (256 * 3)

char _szFileName[32];

ART_Reader::ART_Reader() : Archive()
{
    m_CurFile = -1;
    m_pTilesList = nullptr;
    m_uFileCount = 0;
}

bool ART_Reader::Open(const char *name)
{
    mFileName = EngineSettings::get().GetGameDataDir();
    mFileName += name;

    auto file = Vfs::get().openInput(mFileName);
    if(!file)
    {
        XL_Console::PrintF("^1Error: Failed to load %s", mFileName.c_str());
        return false;
    }
    // Variables
    static uint8_t Buffer[MAX_NB_TILES * 4];
    uint32_t uVersion, uTilesEndNum;
    uint32_t Ind;
    size_t CrtOffset;

    // Read the first part of the header
    if((file->read(reinterpret_cast<char*>(Buffer), 16),file->gcount()) != 16)
    {
        XL_Console::PrintF("^1Error: invalid ART file: not enough header data");
        return false;
    }
    uVersion         = *( (uint32_t *)(&Buffer[ 0]) );
    m_uFileCount     = *( (uint32_t *)(&Buffer[ 4]) );
    m_uTilesStartNum = *( (uint32_t *)(&Buffer[ 8]) );
    uTilesEndNum     = *( (uint32_t *)(&Buffer[12]) );

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
    file->read(reinterpret_cast<char*>(Buffer), m_uFileCount*2);
    for (Ind = 0; Ind < m_uFileCount; Ind++)
        m_pTilesList[Ind].XSize = *( (uint16_t *)(&Buffer[Ind * 2]) );

    file->read(reinterpret_cast<char*>(Buffer), m_uFileCount*2);
    for (Ind = 0; Ind < m_uFileCount; Ind++)
        m_pTilesList[Ind].YSize = *( (uint16_t *)(&Buffer[Ind * 2]) );

    // Extract animation data
    file->read(reinterpret_cast<char*>(Buffer), m_uFileCount*4);
    for (Ind = 0; Ind < m_uFileCount; Ind++)
        m_pTilesList[Ind].AnimData = *( (uint32_t *)(&Buffer[Ind * 4]) );

    // Compute offsets
    CrtOffset = 16 + m_uFileCount * (2 + 2 + 4);
    for (Ind = 0; Ind < m_uFileCount; Ind++)
    {
        m_pTilesList[Ind].Offset = CrtOffset;
        CrtOffset += m_pTilesList[Ind].XSize * m_pTilesList[Ind].YSize;
    }

    m_bOpen = true;

    return true;
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
    mFile = Vfs::get().openInput(mFileName);
    m_CurFile = -1;
    
    if(mFile)
    {
        //convert from the incoming index into a file index.
        char *tmp;
        int32_t nIndex = (int32_t)strtol(pszFile, &tmp, 10);
        //subtract the start out the start tile.
        nIndex -= m_uTilesStartNum;

        if(nIndex >= 0 && nIndex < (int32_t)m_uFileCount &&
           mFile->seekg(m_pTilesList[nIndex].Offset))
            m_CurFile = nIndex;

        if(m_CurFile == -1)
        {
            mFile = nullptr;
            XL_Console::PrintF("^1Error: Failed to load %s from \"%s\"", pszFile, mFileName.c_str());
        }
    }

    return (m_CurFile > -1) ? true : false;
}

void ART_Reader::CloseFile()
{
    mFile = nullptr;
    m_CurFile = -1;
}

uint32_t ART_Reader::GetFileLen()
{
    return m_pTilesList[m_CurFile].XSize * m_pTilesList[m_CurFile].YSize;
}

bool ART_Reader::ReadFile(void *pData, uint32_t uLength)
{
    if(!mFile) { return false; }

    if(uLength == 0) uLength = GetFileLen();
    mFile->read(reinterpret_cast<char*>(pData), uLength);
    return true;
}

//This is needed for this particular archive because it contains size info for the tile.
void *ART_Reader::ReadFileInfo()
{
    return (void *)&m_pTilesList[m_CurFile];
}

int32_t ART_Reader::GetFileCount()
{
    return (int32_t)m_uFileCount;
}

const char *ART_Reader::GetFileName(int32_t nFileIdx)
{
    sprintf(_szFileName, "%03d", nFileIdx+m_uTilesStartNum);
    return _szFileName;
}

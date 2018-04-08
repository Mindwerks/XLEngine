#include "RFF_Reader.h"
#include "../EngineSettings.h"
#include "../ui/XL_Console.h"
#include "../memory/ScratchPad.h"
#include <string.h>
#include <stdio.h>

bool m_bEncrypt;

// File flags
enum
{
    FLAG_NONE      = 0,
    FLAG_ENCRYPTED = (1 << 4)  // 0x10
};

struct RffHeader
{
    uint8_t  Magic[4];
    uint32_t Version;
    uint32_t DirOffset;
    uint32_t fileCount;
    uint32_t Unknown1;
    uint32_t Unknown2;
    uint32_t Unknown3;
    uint32_t Unknown4;
};

// Directory entry for a file
struct DirectoryEntry
{
    uint8_t  Unknown0[16];
    uint32_t Offset;
    uint32_t Size;
    uint32_t Unknown1;
    uint32_t Time;           // Obtained with the "time" standard function
    uint8_t  Flags;
    char Name[11];
    uint32_t Unknown2;       // ID ? Maybe for an enumeration function...
};

// Informations about a packed file
struct FileInfo
{
    char Name[13];
    uint8_t  Flags;
    uint32_t Time;
    uint32_t Size;
    uint32_t Offset;
};

RFF_Reader::RFF_Reader() : Archive()
{
    m_CurFile = -1;
    m_pFile = NULL;
    m_pFileLocal = NULL;
    m_pFileList = NULL;
    m_uFileCount = 0;
    m_bEncrypt = false;
}

bool RFF_Reader::Open(const char *pszName)
{
    sprintf(m_szFileName, "%s%s", EngineSettings::get().GetGameDataDir(), pszName);

    FILE *f = fopen(m_szFileName, "rb");
    if ( f )
    {
        fseek(f, 0, SEEK_END);
        uint32_t len = ftell(f)+1;
        fseek(f, 0, SEEK_SET);
        ScratchPad::StartFrame();
        uint8_t *pBuffer = (uint8_t *)ScratchPad::AllocMem(len);

        RffHeader header;
        fread(&header, sizeof(RffHeader), 1, f);
        if ( (header.Magic[0] != 'R') || (header.Magic[1] != 'F') || (header.Magic[2] != 'F') || (header.Magic[3] != 0x1a) )
        {
            fclose(f);
            XL_Console::PrintF("^1Error: %s not a valid .RFF file", pszName);
            ScratchPad::FreeFrame();
            return false;
        }

        if ( header.Version == 0x301 ) 
            m_bEncrypt = true;
        else
            m_bEncrypt = false;

        uint32_t uOffset  = header.DirOffset;
        m_uFileCount = header.fileCount;

        m_pFileList = xlNew RFF_File[m_uFileCount];

        //Read directory.
        DirectoryEntry *pDirectory = (DirectoryEntry *)pBuffer;
        fseek(f, uOffset, SEEK_SET);
        fread(pBuffer, sizeof(DirectoryEntry), m_uFileCount, f);
        fclose(f);

        // Decrypt the directory (depend on the version)
        if ( m_bEncrypt )
        {
            uint8_t CryptoByte = (uint8_t)header.DirOffset;
            for (uint32_t i = 0; i < m_uFileCount * sizeof(DirectoryEntry); i += 2)
            {
                pBuffer[i+0] ^= CryptoByte;
                pBuffer[i+1] ^= CryptoByte;
                CryptoByte++;
            }
        }

        DirectoryEntry *pDirectoryEntry = (DirectoryEntry *)pBuffer;

        //Now go through each file listing and fill out the RFF_File structure.
        char szFileName[9];
        char szFileExtension[4];
        for(uint32_t i=0, l=0; i<m_uFileCount; i++, l+=48)
        {
            strncpy(szFileExtension, pDirectoryEntry[i].Name,     3);
            strncpy(szFileName,      &pDirectoryEntry[i].Name[3], 8);
            szFileExtension[3] = 0;
            szFileName[8] = 0;

            m_pFileList[i].offset = pDirectoryEntry[i].Offset;
            m_pFileList[i].length = pDirectoryEntry[i].Size;
            m_pFileList[i].flags  = pDirectoryEntry[i].Flags;
            sprintf(m_pFileList[i].szName, "%s.%s", szFileName, szFileExtension);
        }

        m_bOpen = true;

        ScratchPad::FreeFrame();
        return true;
    }
    XL_Console::PrintF("^1Error: Failed to load %s", m_szFileName);

    return false;
}

void RFF_Reader::Close()
{
    CloseFile();
    if ( m_pFileList )
    {
        xlDelete [] m_pFileList;
    }
    m_bOpen = false;
}

FILE *OpenFile_Local(const char *pszFile)
{
    return fopen(pszFile, "rb");
}

//This is different then other archives, we're reading indices here...
bool RFF_Reader::OpenFile(const char *pszFile)
{
    if((m_pFileLocal=OpenFile_Local(pszFile)) != NULL)
    {
        m_pFile = NULL;
        return true;
    }

    m_pFile = fopen(m_szFileName, "rb");
    m_CurFile = -1;
    
    if ( m_pFile )
    {
        //search for this file.
        for (uint32_t i=0; i<m_uFileCount; i++)
        {
            if ( stricmp(pszFile, m_pFileList[i].szName) == 0 )
            {
                m_CurFile = i;
                break;
            }
        }

        if ( m_CurFile == -1 )
        {
            XL_Console::PrintF("^1Error: Failed to load %s from \"%s\"", pszFile, m_szFileName);
        }
    }

    return m_CurFile > -1 ? true : false;
}

void RFF_Reader::CloseFile()
{
    if ( m_pFileLocal )
    {
        fclose(m_pFileLocal);
        m_pFileLocal = NULL;
        return;
    }

    if ( m_pFile )
    {
        fclose(m_pFile);
        m_pFile = NULL;
    }
    m_CurFile = -1;
}

uint32_t RFF_Reader::GetFileLen()
{
    if ( m_pFileLocal )
    {
        fseek(m_pFileLocal, 0, SEEK_END);
        uint32_t len = ftell(m_pFileLocal)+1;
        fseek(m_pFileLocal, 0, SEEK_SET);

        return len;
    }

    return m_pFileList[m_CurFile].length;
}

bool RFF_Reader::ReadFile(void *pData, uint32_t uLength)
{
    if ( m_pFileLocal )
    {
        fread(pData, uLength, 1, m_pFileLocal);
        return true;
    }

    if ( !m_pFile ) { return false; }

    fseek(m_pFile, m_pFileList[m_CurFile].offset, SEEK_SET);
    if ( uLength == 0 )
        uLength = GetFileLen();

    //now reading from an RFF file is more involved due to the encryption.
    fread(pData, uLength, 1, m_pFile);

    if ( m_bEncrypt && (m_pFileList[m_CurFile].flags & FLAG_ENCRYPTED) )
    {
        // Decrypt the first bytes if they're encrypted (256 bytes max)
        for (uint32_t i = 0; i < 256 && i < uLength; i++)
        {
            ((uint8_t *)pData)[i] ^= (i >> 1);
        }
    }

    return true;
}

//This is needed for this particular archive because it contains size info for the tile.
void *RFF_Reader::ReadFileInfo()
{
    return NULL;
}

int32_t RFF_Reader::GetFileCount()
{
    return (int32_t)m_uFileCount;
}

const char *RFF_Reader::GetFileName(int32_t nFileIdx)
{
    return m_pFileList[nFileIdx].szName;
}

#include "RFF_Reader.h"
#include "../EngineSettings.h"
#include "../ui/XL_Console.h"
#include "../memory/ScratchPad.h"
#include <cstring>
#include <cstdio>

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
    m_pFileList = nullptr;
    m_uFileCount = 0;
    m_bEncrypt = false;
}

bool RFF_Reader::Open(const char *name)
{
    mFileName = EngineSettings::get().GetGameResource(name);

    auto file = Vfs::get().openInput(mFileName);
    if(!file)
    {
        XL_Console::PrintF("^1Error: Failed to load %s", mFileName.c_str());
        return false;
    }

    if(!file->seekg(0, std::ios_base::end))
    {
        XL_Console::PrintF("^1Error: Failed to get length of file %s", mFileName.c_str());
        return false;
    }
    uint32_t len = (uint32_t)file->tellg() + 1;
    file->seekg(0);

    ScratchPad::StartFrame();
    uint8_t *pBuffer = (uint8_t *)ScratchPad::AllocMem(len);

    RffHeader header;
    file->read(reinterpret_cast<char*>(&header), sizeof(RffHeader));
    if(header.Magic[0] != 'R' || header.Magic[1] != 'F' || header.Magic[2] != 'F' || header.Magic[3] != 0x1a)
    {
        XL_Console::PrintF("^1Error: %s not a valid .RFF file", name);
        ScratchPad::FreeFrame();
        return false;
    }

    m_bEncrypt = (header.Version == 0x301);

    uint32_t uOffset  = header.DirOffset;
    m_uFileCount = header.fileCount;

    m_pFileList = xlNew RFF_File[m_uFileCount];

    //Read directory.
    file->seekg(uOffset);
    file->read(reinterpret_cast<char*>(pBuffer), sizeof(DirectoryEntry)*m_uFileCount);
    file = nullptr;

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

void RFF_Reader::Close()
{
    CloseFile();
    xlDelete [] m_pFileList;
    m_bOpen = false;
}

//This is different then other archives, we're reading indices here...
bool RFF_Reader::OpenFile(const char *fname)
{
    mFileLocal = Vfs::get().openInput(fname);
    if(mFileLocal != nullptr)
    {
        mFile = nullptr;
        return true;
    }

    mFile = Vfs::get().openInput(mFileName);
    m_CurFile = -1;
    
    if(mFile)
    {
        //search for this file.
        for(uint32_t i = 0;i < m_uFileCount;i++)
        {
            if(stricmp(fname, m_pFileList[i].szName) == 0 &&
               mFile->seekg(m_pFileList[i].offset))
            {
                m_CurFile = i;
                break;
            }
        }

        if(m_CurFile == -1)
        {
            mFile = nullptr;
            XL_Console::PrintF("^1Error: Failed to load %s from \"%s\"", fname, mFileName.c_str());
        }
    }

    return (m_CurFile > -1) ? true : false;
}

void RFF_Reader::CloseFile()
{
    mFileLocal = nullptr;
    mFile = nullptr;
    m_CurFile = -1;
}

uint32_t RFF_Reader::GetFileLen()
{
    if(mFileLocal)
    {
        auto old_offset = mFileLocal->tellg();
        mFileLocal->seekg(0, std::ios_base::end);
        uint32_t len = (uint32_t)mFileLocal->tellg() + 1;
        mFileLocal->seekg(old_offset);

        return len;
    }

    return m_pFileList[m_CurFile].length;
}

bool RFF_Reader::ReadFile(void *pData, uint32_t uLength)
{
    if(mFileLocal)
    {
        mFileLocal->read(reinterpret_cast<char*>(pData), uLength);
        return true;
    }

    if(!mFile) { return false; }

    if(uLength == 0) uLength = GetFileLen();

    //now reading from an RFF file is more involved due to the encryption.
    mFile->read(reinterpret_cast<char*>(pData), uLength);

    if(m_bEncrypt && (m_pFileList[m_CurFile].flags&FLAG_ENCRYPTED))
    {
        // Decrypt the first bytes if they're encrypted (256 bytes max)
        for(uint32_t i = 0;i < 256 && i < uLength;i++)
            ((uint8_t*)pData)[i] ^= (i >> 1);
    }

    return true;
}

//This is needed for this particular archive because it contains size info for the tile.
void *RFF_Reader::ReadFileInfo()
{
    return nullptr;
}

int32_t RFF_Reader::GetFileCount()
{
    return (int32_t)m_uFileCount;
}

const char *RFF_Reader::GetFileName(int32_t nFileIdx)
{
    return m_pFileList[nFileIdx].szName;
}

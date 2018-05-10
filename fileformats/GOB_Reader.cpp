#include "GOB_Reader.h"
#include "../EngineSettings.h"
#include "../ui/XL_Console.h"
#include <cstring>
#include <cctype>
#include <cstdio>
#include <cstdint>

GOB_Reader::GOB_Reader() : Archive()
{
    m_CurFile = -1;
    m_FileList.pEntries = nullptr;
}

bool GOB_Reader::Open(const char *name)
{
    mFileName = EngineSettings::get().GetGameResource(name);

    m_bGOB = true;
    size_t l = strlen(name);
    if(l > 3 && std::tolower(name[l-3]) == 'l' && std::tolower(name[l-2]) == 'a' &&
       std::tolower(name[l-1]))
    {
        //this is a LAB file - very similar to a gob though.
        m_bGOB = false;
    }

    auto file = Vfs::get().openInput(mFileName);
    if(!file)
    {
        XL_Console::PrintF("^1Error: Failed to load %s", mFileName.c_str());
        return false;
    }

    file->read(reinterpret_cast<char*>(&m_Header), sizeof(GOB_Header_t));
    if(m_bGOB)
    {
        file->seekg(m_Header.MASTERX);
        
        m_FileList.MASTERN = read_le<int32_t>(*file);
        m_FileList.pEntries = xlNew GOB_Entry_t[m_FileList.MASTERN];
        file->read(reinterpret_cast<char*>(m_FileList.pEntries),
                   sizeof(GOB_Entry_t)*m_FileList.MASTERN);
    }
    else
    {
        m_FileList.MASTERN = read_le<int32_t>(*file);
        int32_t stringTableSize = read_le<int32_t>(*file);
        m_FileList.pEntries = xlNew GOB_Entry_t[m_FileList.MASTERN];

        //now read string table.
        file->seekg(16*(m_FileList.MASTERN+1));
        std::vector<char> pStringTable(stringTableSize+1, 0);
        file->read(pStringTable.data(), stringTableSize);

        //now read the entries.
        file->seekg(16);
        for (int32_t e=0; e<m_FileList.MASTERN; e++)
        {
            uint32_t fname_offs = read_le<uint32_t>(*file);
            uint32_t start = read_le<uint32_t>(*file);
            uint32_t size = read_le<uint32_t>(*file);
            /*uint32_t dummy =*/ read_le<uint32_t>(*file);

            m_FileList.pEntries[e].IX = start;
            m_FileList.pEntries[e].LEN = size;
            strcpy(m_FileList.pEntries[e].NAME, &pStringTable[fname_offs]);
        }
    }

    m_bOpen = true;

    return true;
}

void GOB_Reader::Close()
{
    CloseFile();
    xlDelete [] m_FileList.pEntries;
    m_FileList.pEntries = nullptr;
    m_bOpen = false;
}

bool GOB_Reader::OpenFile(const char *pszFile)
{
    mFile = Vfs::get().openInput(mFileName);
    m_CurFile = -1;
    
    if(mFile)
    {
        //search for this file.
        for(int32_t i=0; i<m_FileList.MASTERN; i++)
        {
            if(stricmp(pszFile, m_FileList.pEntries[i].NAME) == 0 &&
               mFile->seekg(m_FileList.pEntries[i].IX))
            {
                m_CurFile = i;
                break;
            }
        }

        if(m_CurFile == -1)
        {
            mFile = nullptr;
            XL_Console::PrintF("^1Error: Failed to load %s from \"%s\"", pszFile, mFileName.c_str());
        }
    }

    return (m_CurFile > -1) ? true : false;
}

void GOB_Reader::CloseFile()
{
    mFile = nullptr;
    m_CurFile = -1;
}

uint32_t GOB_Reader::GetFileLen()
{
    return (uint32_t)m_FileList.pEntries[ m_CurFile ].LEN;
}

bool GOB_Reader::ReadFile(void *pData, uint32_t uLength)
{
    if(!mFile) { return false; }

    if(uLength == 0) uLength = (uint32_t)m_FileList.pEntries[m_CurFile].LEN;
    mFile->read(reinterpret_cast<char*>(pData), uLength);
    return true;
}

int32_t GOB_Reader::GetFileCount()
{
    return m_FileList.MASTERN;
}

const char *GOB_Reader::GetFileName(int32_t nFileIdx)
{
    return m_FileList.pEntries[nFileIdx].NAME;
}

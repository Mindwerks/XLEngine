#include "LFD_Reader.h"
#include "../EngineSettings.h"
#include "../ui/XL_Console.h"
#include <cstring>
#include <cstdio>

LFD_Reader::LFD_Reader() : Archive()
{
    m_CurFile = -1;
}

bool LFD_Reader::Open(const char *name)
{
    char fname[32];
    sprintf(fname, "lfd/%s", name);
    mFileName = EngineSettings::get().GetGameResource(fname);

    auto file = Vfs::get().openInput(mFileName);
    if(!file)
    {
        XL_Console::PrintF("^1Error: Failed to load %s, make sure that LFD sub-directory exists for Dark Forces.",
                           mFileName.c_str());
        return false;
    }

    LFD_Entry_t root;
    file->read(root.TYPE, 4); root.TYPE[4] = 0;
    file->read(root.NAME, 8); root.NAME[8] = 0;
    root.LENGTH = read_le<int32_t>(*file);

    m_FileList.MASTERN = root.LENGTH / 16;
    m_FileList.pEntries = xlNew LFD_Entry_t[m_FileList.MASTERN];

    int IX = 16 + root.LENGTH;
    for(int i = 0;i < m_FileList.MASTERN;i++)
    {
        file->read(m_FileList.pEntries[i].TYPE, 4);
        m_FileList.pEntries[i].TYPE[4] = 0;
        file->read(m_FileList.pEntries[i].NAME, 8);
        m_FileList.pEntries[i].NAME[8] = 0;
        m_FileList.pEntries[i].LENGTH = read_le<int32_t>(*file);
        m_FileList.pEntries[i].IX = IX+16;

        IX += 16+m_FileList.pEntries[i].LENGTH;
    }

    m_bOpen = true;

    return true;
}

void LFD_Reader::Close()
{
    CloseFile();

    xlDelete [] m_FileList.pEntries;
    m_FileList.pEntries = nullptr;

    m_bOpen = false;
}

bool LFD_Reader::OpenFile(const char *pszFile)
{
    mFile = Vfs::get().openInput(mFileName);
    m_CurFile = -1;

    char szName[10];
    char szType[10];
    int32_t nExt = -1;
    int32_t l = (int32_t)strlen(pszFile);
    int32_t i;
    for (i=0; i<l; i++)
    {
        if ( pszFile[i] == '.' )
        {
            nExt = i;
            szName[i] = 0;
            break;
        }
        else
        {
            szName[i] = pszFile[i];
        }
    }
    for (i=nExt+1; i<l; i++)
    {
        szType[i-nExt-1] = pszFile[i];
    }
    szType[i-nExt-1] = 0;

    //search for this file.
    for (int i=0; i<m_FileList.MASTERN; i++)
    {
        if(stricmp(szName, m_FileList.pEntries[i].NAME) == 0 && stricmp(szType, m_FileList.pEntries[i].TYPE) == 0 &&
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

    return (m_CurFile > -1) ? true : false;
}

void LFD_Reader::CloseFile()
{
    mFile = nullptr;
    m_CurFile = -1;
}

uint32_t LFD_Reader::GetFileLen()
{
    return (uint32_t)m_FileList.pEntries[ m_CurFile ].LENGTH;
}

bool LFD_Reader::ReadFile(void *pData, uint32_t uLength)
{
    if(uLength == 0) uLength = (uint32_t)m_FileList.pEntries[m_CurFile].LENGTH;
    mFile->read(reinterpret_cast<char*>(pData), uLength);
    return true;
}

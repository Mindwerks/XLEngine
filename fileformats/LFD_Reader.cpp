#include "LFD_Reader.h"
#include "../EngineSettings.h"
#include "../ui/XL_Console.h"
#include <string.h>
#include <stdio.h>

LFD_Reader::LFD_Reader() : Archive()
{
    m_CurFile = -1;
    m_pFile = NULL;
}

bool LFD_Reader::Open(const char *pszName)
{
    sprintf(m_szFileName, "%sLFD/%s", EngineSettings::get().GetGameDataDir(), pszName);

    FILE *f = fopen(m_szFileName, "rb");
    if ( f )
    {
        LFD_Entry_t root, entry;
        fread(&root, sizeof(LFD_Entry_t), 1, f);
        m_FileList.MASTERN = root.LENGTH / sizeof(LFD_Entry_t);

        m_FileList.pEntries = xlNew LFD_EntryFinal_t[m_FileList.MASTERN];

        int IX = sizeof(LFD_Entry_t) + root.LENGTH;
        for (int i=0; i<m_FileList.MASTERN; i++)
        {
            fread(&entry, sizeof(LFD_Entry_t), 1, f);

            memcpy(m_FileList.pEntries[i].TYPE, entry.TYPE, 4);
            m_FileList.pEntries[i].TYPE[4] = 0;
            memcpy(m_FileList.pEntries[i].NAME, entry.NAME, 8);
            m_FileList.pEntries[i].NAME[8] = 0;
            m_FileList.pEntries[i].LENGTH = entry.LENGTH;
            m_FileList.pEntries[i].IX = IX+sizeof(LFD_Entry_t);

            IX += sizeof(LFD_Entry_t)+entry.LENGTH;
        }

        fclose(f);
        m_bOpen = true;

        return true;
    }

    XL_Console::PrintF("^1Error: Failed to load %s, make sure that %sLFD is the correct directory for Dark Forces.",
                       m_szFileName, EngineSettings::get().GetGameDataDir());

    return false;
}

void LFD_Reader::Close()
{
    CloseFile();
    if ( m_FileList.pEntries )
    {
        xlDelete [] m_FileList.pEntries;
        m_FileList.pEntries = NULL;
    }
    m_bOpen = false;
}

bool LFD_Reader::OpenFile(const char *pszFile)
{
    m_pFile = fopen(m_szFileName, "rb");
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
        if ( stricmp(szName, m_FileList.pEntries[i].NAME) == 0 && stricmp(szType, m_FileList.pEntries[i].TYPE) == 0 )
        {
            m_CurFile = i;
            break;
        }
    }

    if ( m_CurFile == -1 )
    {
        XL_Console::PrintF("^1Error: Failed to load %s from \"%s\"", pszFile, m_szFileName);
    }

    return m_CurFile > -1 ? true : false;
}

void LFD_Reader::CloseFile()
{
    if ( m_pFile )
    {
        fclose(m_pFile);
        m_pFile = NULL;
    }
    m_CurFile = -1;
}

uint32_t LFD_Reader::GetFileLen()
{
    return (uint32_t)m_FileList.pEntries[ m_CurFile ].LENGTH;
}

bool LFD_Reader::ReadFile(void *pData, uint32_t uLength)
{
    fseek(m_pFile, m_FileList.pEntries[ m_CurFile ].IX, SEEK_SET);

    if ( uLength == 0 )
        uLength = (uint32_t)m_FileList.pEntries[ m_CurFile ].LENGTH;

    fread(pData, uLength, 1, m_pFile);

    return true;
}

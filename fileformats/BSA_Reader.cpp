#include "BSA_Reader.h"
#include "../EngineSettings.h"
#include "../ui/XL_Console.h"
#include <cstring>
#include <cstdio>
#include <cassert>

BSA_Reader::BSA_Reader() : Archive()
{
    m_CurFile = -1;
    m_pFile = NULL;
    m_pFileListName = NULL;
    m_pFileListNum  = NULL;
}

bool BSA_Reader::Open(const char *pszName)
{
    sprintf(m_szFileName, "%s%s", EngineSettings::get().GetGameDataDir(), pszName);

    FILE *f = fopen(m_szFileName, "rb");
    if ( f )
    {
        fread(&m_Header, sizeof(BSA_Header), 1, f);

        fseek(f, 0, SEEK_END);
        int len = ftell(f);
        
        if ( m_Header.DirectoryType == DT_NameRecord )
        {
            m_pFileListName = new BSA_EntryName[ m_Header.DirectoryCount ];
            fseek(f, len-m_Header.DirectoryCount*sizeof(BSA_EntryName), SEEK_SET);
            fread(m_pFileListName, sizeof(BSA_EntryName), m_Header.DirectoryCount, f);
        }
        else
        {
            m_pFileListNum = new BSA_EntryNum[ m_Header.DirectoryCount ];
            fseek(f, len-m_Header.DirectoryCount*sizeof(BSA_EntryNum), SEEK_SET);
            fread(m_pFileListNum, sizeof(BSA_EntryNum), m_Header.DirectoryCount, f);
        }

        fclose(f);
        m_bOpen = true;

        return true;
    }
    XL_Console::PrintF("^1Error: Failed to load %s", m_szFileName);

    return false;
}

void BSA_Reader::Close()
{
    CloseFile();
    if ( m_pFileListName )
    {
        delete [] m_pFileListName;
        m_pFileListName = NULL;
    }
    if ( m_pFileListNum )
    {
        delete [] m_pFileListNum;
        m_pFileListNum = NULL;
    }
    m_bOpen = false;
}

bool BSA_Reader::OpenFile(const char *pszFile)
{
    if ( m_Header.DirectoryType != DT_NameRecord )
        return false;

    assert(m_pFile == NULL);
    m_pFile = fopen(m_szFileName, "rb");
    m_CurFile = -1;
    
    if ( m_pFile )
    {
        //search for this file.
        for (int i=0; i<m_Header.DirectoryCount; i++)
        {
            if ( stricmp(pszFile, m_pFileListName[i].NAME) == 0 )
            {
                m_CurFile = i;
                break;
            }
        }

        if ( m_CurFile == -1 )
        {
            fclose(m_pFile);
            m_pFile = NULL;
            XL_Console::PrintF("^1Error: Failed to load %s from \"%s\"", pszFile, m_szFileName);
        }
    }

    return m_CurFile > -1 ? true : false;
}

bool BSA_Reader::OpenFile(const uint32_t uID)
{
    if ( m_Header.DirectoryType != DT_NumberRecord )
        return false;

    assert(m_pFile == NULL);
    m_pFile = fopen(m_szFileName, "rb");
    assert(m_pFile);
    m_CurFile = -1;
    
    if ( m_pFile )
    {
        //search for this file.
        for (int i=0; i<m_Header.DirectoryCount; i++)
        {
            if ( m_pFileListNum[i].RecordID == uID )
            {
                m_CurFile = i;
                break;
            }
        }

        if ( m_CurFile == -1 )
        {
            fclose(m_pFile);
            m_pFile = NULL;

            XL_Console::PrintF("^1Error: Failed to load %u from \"%s\"", uID, m_szFileName);
        }
    }

    return m_CurFile > -1 ? true : false;
}

bool BSA_Reader::SearchForFile(const char *pszFileIn, char *pszFileOut)
{
    bool bFileFound = false;

    if ( m_Header.DirectoryType != DT_NameRecord )
        return false;

    size_t lIn = strlen(pszFileIn);
    size_t lCur;
    char aMinWild[2] = { 127, 127 };
    int fIdx = -1;
    for (int i=0; i<m_Header.DirectoryCount; i++)
    {
        lCur = strlen(m_pFileListName[i].NAME);
        if ( lIn != lCur )
            continue;

        bool bMatch = true;
        int mIdx = 0;
        char aWild[2];
        for (int c=0; c<(int)lIn; c++)
        {
            if ( pszFileIn[c] != '?' && tolower(pszFileIn[c]) != tolower(m_pFileListName[i].NAME[c]) )
            {
                bMatch = false;
                break;
            }
            else if ( pszFileIn[c] == '?' )
            {
                aWild[ mIdx++ ] = tolower(m_pFileListName[i].NAME[c]);
            }
        }
        if ( bMatch )
        {
            if ( aWild[0] < aMinWild[0] || (aWild[0] == aMinWild[0] && aWild[1] < aMinWild[1]) )
            {
                aMinWild[0] = aWild[0];
                aMinWild[1] = aWild[1];
                fIdx = i;
            }
        }
    }
    if ( fIdx > -1 )
    {
        bFileFound = true;
        strcpy(pszFileOut, m_pFileListName[fIdx].NAME);
    }

    return bFileFound;
}

void BSA_Reader::CloseFile()
{
    if ( m_pFile )
    {
        fclose(m_pFile);
        m_pFile = NULL;
    }
    m_CurFile = -1;
}

uint32_t BSA_Reader::GetFileLen()
{
    uint32_t length = 0;

    if ( m_Header.DirectoryType == DT_NameRecord )
        length = m_pFileListName[ m_CurFile ].RecordSize;
    else
        length = m_pFileListNum[ m_CurFile ].RecordSize;

    return length;
}

bool BSA_Reader::ReadFile(void *pData, uint32_t uLength)
{
    if ( !m_pFile ) { return false; }

    int file_loc = sizeof(BSA_Header);
    for (int i=0; i<m_CurFile; i++)
    {
        if ( m_Header.DirectoryType == DT_NameRecord )
            file_loc += m_pFileListName[i].RecordSize;
        else
            file_loc += m_pFileListNum[i].RecordSize;
    }
    int file_len;
    if ( m_Header.DirectoryType == DT_NameRecord )
        file_len = m_pFileListName[ m_CurFile ].RecordSize;
    else
        file_len = m_pFileListNum[ m_CurFile ].RecordSize;

    fseek(m_pFile, file_loc, SEEK_SET);
    fread(pData, file_len, 1, m_pFile);

    return true;
}

int32_t BSA_Reader::GetFileCount()
{
    return m_Header.DirectoryCount;
}

const char *BSA_Reader::GetFileName(int32_t nFileIdx)
{
    return m_pFileListName[ nFileIdx ].NAME;
}

uint32_t BSA_Reader::GetFileID(int32_t nFileIdx)
{
    return m_pFileListNum[ nFileIdx ].RecordID;
}

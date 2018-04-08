#include "GOB_Reader.h"
#include "../EngineSettings.h"
#include "../ui/XL_Console.h"

GOB_Reader::GOB_Reader() : Archive() {
    m_CurFile = -1;
    m_pFile = NULL;
    m_FileList.pEntries = NULL;
}

bool GOB_Reader::Open(const char *pszName) {
    sprintf(m_szFileName, "%s%s", EngineSettings::GetGameDataDir(), pszName);
    m_bGOB = true;
    size_t l = strlen(pszName);
    if ((pszName[l - 3] == 'l' && pszName[l - 2] == 'a' && pszName[l - 1] == 'b') ||
        (pszName[l - 3] == 'L' && pszName[l - 2] == 'A' && pszName[l - 1] == 'B'))
    {
        //this is a LAB file - very similar to a gob though.
        m_bGOB = false;
    }

    FILE *f = fopen(m_szFileName, "rb");
    if (f)
    {
        fread(&m_Header, sizeof(GOB_Header_t), 1, f);
        if (m_bGOB)
        {
            fseek(f, m_Header.MASTERX, SEEK_SET);

            fread(&m_FileList.MASTERN, sizeof(int32_t), 1, f);
            m_FileList.pEntries = xlNew GOB_Entry_t[m_FileList.MASTERN];
            fread(m_FileList.pEntries, sizeof(GOB_Entry_t), m_FileList.MASTERN, f);
        }
        else
        {
            int32_t stringTableSize;
            fread(&m_FileList.MASTERN, sizeof(int32_t), 1, f);
            fread(&stringTableSize, sizeof(int32_t), 1, f);
            m_FileList.pEntries = xlNew GOB_Entry_t[m_FileList.MASTERN];

            //now read string table.
            fseek(f, 16 * (m_FileList.MASTERN + 1), SEEK_SET);
            char *pStringTable = xlNew char[stringTableSize + 1];
            fread(pStringTable, 1, stringTableSize, f);

            //now read the entries.
            fseek(f, 16, SEEK_SET);
            for (int32_t e = 0; e < m_FileList.MASTERN; e++)
            {
                uint32_t fname_offs;
                uint32_t start, size, dummy;
                fread(&fname_offs, sizeof(uint32_t), 1, f);
                fread(&start, sizeof(uint32_t), 1, f);
                fread(&size, sizeof(uint32_t), 1, f);
                fread(&dummy, sizeof(uint32_t), 1, f);

                m_FileList.pEntries[e].IX = start;
                m_FileList.pEntries[e].LEN = size;
                strcpy(m_FileList.pEntries[e].NAME, &pStringTable[fname_offs]);
            }

            xlDelete[] pStringTable;
            pStringTable = NULL;
        }

        fclose(f);
        m_bOpen = true;

        return true;
    }
    XL_Console::PrintF("^1Error: Failed to load %s", m_szFileName);

    return false;
}

void GOB_Reader::Close() {
    CloseFile();
    if (m_FileList.pEntries)
    {
        xlDelete[] m_FileList.pEntries;
        m_FileList.pEntries = NULL;
    }
    m_bOpen = false;
}

bool GOB_Reader::OpenFile(const char *pszFile) {
    m_pFile = fopen(m_szFileName, "rb");
    m_CurFile = -1;

    if (m_pFile)
    {
        //search for this file.
        for (int32_t i = 0; i < m_FileList.MASTERN; i++)
        {
            if (stricmp(pszFile, m_FileList.pEntries[i].NAME) == 0)
            {
                m_CurFile = i;
                break;
            }
        }

        if (m_CurFile == -1)
        {
            XL_Console::PrintF("^1Error: Failed to load %s from \"%s\"", pszFile, m_szFileName);
        }
    }

    return m_CurFile > -1 ? true : false;
}

void GOB_Reader::CloseFile() {
    if (m_pFile)
    {
        fclose(m_pFile);
        m_pFile = NULL;
    }
    m_CurFile = -1;
}

uint32_t GOB_Reader::GetFileLen() {
    return (uint32_t) m_FileList.pEntries[m_CurFile].LEN;
}

bool GOB_Reader::ReadFile(void *pData, uint32_t uLength) {
    if (!m_pFile)
    { return false; }

    fseek(m_pFile, m_FileList.pEntries[m_CurFile].IX, SEEK_SET);
    if (uLength == 0)
        uLength = (uint32_t) m_FileList.pEntries[m_CurFile].LEN;

    fread(pData, uLength, 1, m_pFile);

    return true;
}

int32_t GOB_Reader::GetFileCount() {
    return m_FileList.MASTERN;
}

const char *GOB_Reader::GetFileName(int32_t nFileIdx) {
    return m_FileList.pEntries[nFileIdx].NAME;
}

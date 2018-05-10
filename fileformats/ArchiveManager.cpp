#include "ArchiveManager.h"
#include "TextureLoader.h"
#include "Archive.h"
#include "GOB_Reader.h"
#include "BSA_Reader.h"
#include "LFD_Reader.h"
#include "ART_Reader.h"
#include "RFF_Reader.h"
#include "../ui/XL_Console.h"
#include "../EngineSettings.h"

const char *ArchiveManager::m_apszArchiveExt[]=
{
    "GOB",
    "LAB",
    "LFD",
    "ART",
    "RFF",
    "BSA",
};

std::map<std::string, Archive *> ArchiveManager::m_OpenArchives;
std::vector<Archive *> ArchiveManager::m_ArchiveList;
Archive *ArchiveManager::m_pCurArchive;
istream_ptr ArchiveManager::sCurrentSysFile;

void ArchiveManager::Init()
{
    m_pCurArchive = nullptr;
    sCurrentSysFile = nullptr;
    TextureLoader::Init();
}

void ArchiveManager::Destroy()
{
    m_OpenArchives.clear();

    for (Archive *archive : m_ArchiveList)
    {
        archive->Close();
        xlDelete archive;
    }

    m_ArchiveList.clear();
    TextureLoader::Destroy();
}

Archive *ArchiveManager::OpenArchive(uint32_t uArchiveType, const char *pszArchiveName)
{
    std::map<std::string, Archive *>::iterator iArchive = m_OpenArchives.find(pszArchiveName);
    if ( iArchive != m_OpenArchives.end() )
    {
        Archive *pArchive = iArchive->second;
        if ( pArchive->IsOpen() == false )
        {
            pArchive->Open(pszArchiveName);
        }
        return pArchive;
    }

    //now we have to allocate and setup the archive...
    Archive *pArchive=nullptr;
    switch (uArchiveType)
    {
        case ARCHIVETYPE_GOB:
        case ARCHIVETYPE_LAB:
            pArchive = xlNew GOB_Reader();
            pArchive->Open(pszArchiveName);
            break;
        case ARCHIVETYPE_LFD:
            pArchive = xlNew LFD_Reader();
            pArchive->Open(pszArchiveName);
            break;
        case ARCHIVETYPE_ART:
            pArchive = xlNew ART_Reader();
            pArchive->Open(pszArchiveName);
            break;
        case ARCHIVETYPE_RFF:
            pArchive = xlNew RFF_Reader();
            pArchive->Open(pszArchiveName);
            break;
        case ARCHIVETYPE_BSA:
            pArchive = xlNew BSA_Reader();
            pArchive->Open(pszArchiveName);
            break;
        default:
            XL_Console::PrintF("^1Error: Unknown archive type: %u.", uArchiveType);
            break;
    };

    if ( pArchive )
    {
        m_OpenArchives[pszArchiveName] = pArchive;
        m_ArchiveList.push_back( pArchive );
    }
    
    return pArchive;
}

void ArchiveManager::CloseArchive(Archive *pArchive)
{
    if ( pArchive )
    {
        //Close the archive.
        pArchive->Close();
    }
}

int32_t ArchiveManager::GameFile_Open(Archive *pArchive, const char *pszFileName)
{
    if ( pArchive && pArchive->OpenFile(pszFileName) )
    {
        m_pCurArchive = pArchive;
        return 1;
    }
    return 0;
}

int32_t ArchiveManager::GameFile_Open(uint32_t uArchiveType, const char *pszArchiveName, const char *pszFileName)
{
    Archive *pArchive = OpenArchive(uArchiveType, pszArchiveName);
    if ( pArchive && pArchive->OpenFile(pszFileName) )
    {
        m_pCurArchive = pArchive;
        return 1;
    }
    return 0;
}

int32_t ArchiveManager::GameFile_Open(uint32_t uArchiveType, const char *pszArchiveName, uint32_t fileID)
{
    Archive *pArchive = OpenArchive(uArchiveType, pszArchiveName);
    if ( pArchive && pArchive->OpenFile(fileID) )
    {
        m_pCurArchive = pArchive;
        return 1;
    }
    return 0;
}

int32_t ArchiveManager::GameFile_SearchForFile(uint32_t uArchiveType, const char *pszArchiveName, const char *pszFileName, char *pszFileOut)
{
    Archive *pArchive = OpenArchive(uArchiveType, pszArchiveName);
    if ( pArchive && pArchive->SearchForFile(pszFileName, pszFileOut) )
    {
        m_pCurArchive = pArchive;
        return 1;
    }
    return 0;
}

uint32_t ArchiveManager::GameFile_GetLength()
{
    if ( m_pCurArchive )
    {
        return m_pCurArchive->GetFileLen();
    }
    return 0;
}

void ArchiveManager::GameFile_Read(void *pData, uint32_t uLength)
{
    if ( m_pCurArchive )
    {
        m_pCurArchive->ReadFile(pData, uLength);
    }
}

void *ArchiveManager::GameFile_GetFileInfo()
{
    if ( m_pCurArchive )
    {
        return m_pCurArchive->ReadFileInfo();
    }
    return nullptr;
}

void ArchiveManager::GameFile_Close()
{
    if ( m_pCurArchive )
    {
        m_pCurArchive->CloseFile();
    }
    m_pCurArchive=nullptr;
}

int32_t ArchiveManager::File_Open(const char *fname)
{
    if(sCurrentSysFile)
        return 0;

    sCurrentSysFile = Vfs::get().openInput(
        EngineSettings::get().GetGameResource(fname)
    );
    if(sCurrentSysFile) return 1;
    return 0;
}

uint32_t ArchiveManager::File_GetLength()
{
    uint32_t uLen = 0;
    if(sCurrentSysFile && sCurrentSysFile->seekg(0, std::ios_base::end))
    {
        uLen = sCurrentSysFile->tellg();
        sCurrentSysFile->seekg(0);
    }
    return uLen;
}

void ArchiveManager::File_Read(void *pData, uint32_t uStart, uint32_t uLength)
{
    if(sCurrentSysFile)
    {
        sCurrentSysFile->seekg(uStart);
        sCurrentSysFile->read(reinterpret_cast<char*>(pData), uLength);
    }
}

void *ArchiveManager::File_GetFileInfo()
{
    return nullptr;
}

void ArchiveManager::File_Close()
{
    sCurrentSysFile = nullptr;
}

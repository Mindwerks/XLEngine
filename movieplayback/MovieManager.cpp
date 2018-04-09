#include "MovieManager.h"
#include "MoviePlayer.h"
#include "../fileformats/ArchiveManager.h"
#include "../fileformats/Archive.h"
#include "LFD_Film.h"
#include "../ui/XL_Console.h"

std::map<std::string, MoviePlayer *> MovieManager::m_MoviePlayers;
std::vector<MoviePlayer *> MovieManager::m_MoviePlayerList;
MoviePlayer *MovieManager::m_pCurPlayer;
Archive *MovieManager::m_apArchives[2];
IDriver3D *MovieManager::m_pDriver;

void MovieManager::Init(IDriver3D *pDriver)
{
    m_pCurPlayer=NULL;
    m_apArchives[0] = NULL;
    m_apArchives[1] = NULL;
    m_pDriver = pDriver;
}

void MovieManager::Destroy()
{
    m_MoviePlayers.clear();

    std::vector<MoviePlayer *>::iterator iMoviePlayer = m_MoviePlayerList.begin();
    std::vector<MoviePlayer *>::iterator eMoviePlayer = m_MoviePlayerList.end();
    for (; iMoviePlayer != eMoviePlayer; ++iMoviePlayer)
    {
        xlDelete (*iMoviePlayer);
    }
    m_MoviePlayerList.clear();
}

void MovieManager::SetPlayerType(uint32_t uPlayerType)
{
    if ( m_pCurPlayer )
    {
        m_pCurPlayer->Stop();
        m_pCurPlayer = NULL;
    }

    switch (uPlayerType)
    {
        case MOVIETYPE_LFD:
            m_pCurPlayer = xlNew LFD_Film(m_pDriver);
            break;
        default:
            XL_Console::PrintF("^1Error: Unknown movie type: %u.", uPlayerType);
            break;
    };

    if ( m_pCurPlayer )
    {
        m_MoviePlayerList.push_back( m_pCurPlayer );
    }
}

void MovieManager::SetPlayerArchives(uint32_t uArchiveType, const char *pszArchive0, const char *pszArchive1)
{
    m_apArchives[0] = NULL;
    m_apArchives[1] = NULL;
    if ( pszArchive0 && pszArchive0[0] )
    {
        m_apArchives[0] = ArchiveManager::OpenArchive(uArchiveType, pszArchive0);
    }
    if ( pszArchive1 && pszArchive1[0] )
    {
        m_apArchives[1] = ArchiveManager::OpenArchive(uArchiveType, pszArchive1);
    }
}

int32_t MovieManager::StartMovie(const char *pszFile, uint32_t uFlags, int32_t nSpeed)
{
    //Stop the current movie, if there is one.
    StopMovie();

    if ( m_pCurPlayer )
    {
        if ( m_pCurPlayer->Start( m_apArchives[0], m_apArchives[1], pszFile, uFlags, nSpeed ) )
            return 1;
    }

    return 0;
}

int32_t MovieManager::UpdateMovie()
{
    if ( m_pCurPlayer )
    {
        return m_pCurPlayer->Update();
    }
    return 0;
}

void MovieManager::StopMovie(void)
{
    if ( m_pCurPlayer )
    {
        m_pCurPlayer->Stop();
    }
}

void MovieManager::RenderMovie(float fDeltaTime)
{
    if ( m_pCurPlayer )
    {
        m_pCurPlayer->Render(fDeltaTime);
    }
}

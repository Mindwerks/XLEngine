#ifndef MOVIEMANAGER_H
#define MOVIEMANAGER_H

#include "../CommonTypes.h"
#include "MovieTypes.h"

#include <string>
#include <vector>
#include <map>

class MoviePlayer;
class Archive;
class IDriver3D;

class MovieManager
{
public:
    static void Init(IDriver3D *pDriver);
    static void Destroy();

    static void SetPlayerType(uint32_t uPlayerType);
    static void SetPlayerArchives(uint32_t uArchiveType, const char *pszArchive0, const char *pszArchive1);
    static int32_t StartMovie(const char *pszFile, uint32_t uFlags, int32_t nSpeed);
    static int32_t UpdateMovie();
    static void StopMovie(void);
    static void RenderMovie(float fDeltaTime);

private:
    static std::map<std::string, MoviePlayer *> m_MoviePlayers;
    static std::vector<MoviePlayer *> m_MoviePlayerList;
    static Archive *m_apArchives[2];
    static MoviePlayer *m_pCurPlayer;
    static IDriver3D *m_pDriver;
};

#endif //MOVIEMANAGER_H
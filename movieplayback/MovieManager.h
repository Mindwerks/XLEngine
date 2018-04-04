#ifndef MOVIEMANAGER_H
#define MOVIEMANAGER_H

#include "../CommonTypes.h"
#include "MovieTypes.h"
#include <string>
#include <vector>
#include <map>

using namespace std;
class MoviePlayer;
class Archive;
class IDriver3D;

class MovieManager
{
public:
	static void Init(IDriver3D *pDriver);
	static void Destroy();

	static void SetPlayerType(u32 uPlayerType);
	static void SetPlayerArchives(u32 uArchiveType, const char *pszArchive0, const char *pszArchive1);
	static s32 StartMovie(const char *pszFile, u32 uFlags, s32 nSpeed);
	static s32 UpdateMovie();
	static void StopMovie(void);
	static void RenderMovie(f32 fDeltaTime);

private:
	static map<string, MoviePlayer *> m_MoviePlayers;
	static vector<MoviePlayer *> m_MoviePlayerList;
	static Archive *m_apArchives[2];
	static MoviePlayer *m_pCurPlayer;
	static IDriver3D *m_pDriver;
};

#endif //MOVIEMANAGER_H
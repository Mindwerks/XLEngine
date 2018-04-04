#ifndef OUTLAWSXL_GAME_H
#define OUTLAWSXL_GAME_H

#include "../plugin_framework/plugin.h"
#include <string>

using namespace std;
class OutlawsXL_Player;

class OutlawsXL_Game
{
public:
	OutlawsXL_Game(const XLEngine_Plugin_API *API);
	~OutlawsXL_Game(void);

	void FixedUpdate();
	void VariableUpdate(float dt);
	void PreRender(float dt);
	void PostRender(float dt);

	void KeyDown(s32 key);
	void GetVersion(s32& major, s32& minor) { major = m_nVersionMajor; minor = m_nVersionMinor; }
	void NewGame(s32 episode, s32 difficulty);

private:
	const XLEngine_Plugin_API *m_pAPI;
	s32 m_nVersionMajor;
	s32 m_nVersionMinor;

	OutlawsXL_Player *m_Player;

	//Script commands
	static void SC_Game_NewGameSettings(int episode, int difficulty);

	//Console commands
	static OutlawsXL_Game *s_pGame_Console;
	static void CC_GameVersion(const vector<string>& args, void *pUserData);
	static void CC_LoadMap(const vector<string>& args, void *pUserData);
	static void CC_PassThruAdjoins(const vector<string>& args, void *pUserData);
};

#endif //BLOODXL_GAME_H
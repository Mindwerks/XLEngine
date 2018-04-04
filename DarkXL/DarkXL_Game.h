#ifndef DARKXL_GAME_H
#define DARKXL_GAME_H

#include "../plugin_framework/plugin.h"
#include <string>

using namespace std;

class DarkXL_Game
{
public:
	DarkXL_Game(const XLEngine_Plugin_API *API);
	~DarkXL_Game(void);

	void FixedUpdate();
	void VariableUpdate(float dt);
	void PreRender(float dt);
	void PostRender(float dt);

	void KeyDown(s32 key);

	void GetVersion(s32& major, s32& minor) { major = m_nVersionMajor; minor = m_nVersionMinor; }

private:
	const XLEngine_Plugin_API *m_pAPI;
	s32 m_nVersionMajor;
	s32 m_nVersionMinor;

	//Console commands
	static DarkXL_Game *s_pGame_Console;
	static void CC_GameVersion(const vector<string>& args, void *pUserData);
};

#endif //DARKXL_GAME_H
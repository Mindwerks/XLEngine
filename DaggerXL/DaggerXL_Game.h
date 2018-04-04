#ifndef DAGGERXL_GAME_H
#define DAGGERXL_GAME_H

#include "../plugin_framework/plugin.h"
#include <string>
#include <vector>

using namespace std;
class DaggerXL_Player;
class Logic_Door;
class Logic_Obj_Action;
class Logic_NPC;
class NPC;

class DaggerXL_Game
{
public:
	DaggerXL_Game(const XLEngine_Plugin_API *API);
	~DaggerXL_Game(void);

	void FixedUpdate();
	void VariableUpdate(float dt);
	void PreRender(float dt);
	void PostRender(float dt);

	void KeyDown(s32 key);

	void GetVersion(s32& major, s32& minor) { major = m_nVersionMajor; minor = m_nVersionMinor; }

	void NewGame();

	bool PlaceNPC(s32 newNPC=-1);
	void CreateNPCs();

	static void WorldUpdate(s32 newWorldX, s32 newWorldY, XLEngine_Plugin_API *API, void *pUserData);

private:
	struct Color
	{
		u8 red;
		u8 green;
		u8 blue;
	};

	struct ColHeader
	{
		s32 fileSize;
		u16 FileFormatMajor;
		u16 FileFormatMinor;
	};

	void LoadPals();
	void LoadPal(struct Color *pPalData, const char *pszFile);
	void LoadCol(struct Color *pPalData, const char *pszFile);
	void LoadColormap(u8 *pColormap, const char *pszFile);
	void ClearColorToColor(u8 *pColormap, u32 color, u32 newColor);

	const XLEngine_Plugin_API *m_pAPI;
	s32 m_nVersionMajor;
	s32 m_nVersionMinor;
	DaggerXL_Player *m_Player;
	Logic_Door *m_DoorLogic;
	Logic_Obj_Action *m_ObjActionLogic;
	Logic_NPC *m_NPC_Logic;
	bool m_bNPCs_Created;

	NPC *m_NPC_List[32];

	//Script commands
	static void SC_Game_NewGame();

	//Console commands
	static DaggerXL_Game *s_pGame_Console;
	static void CC_GameVersion(const vector<string>& args, void *pUserData);
};

#endif //BLOODXL_GAME_H
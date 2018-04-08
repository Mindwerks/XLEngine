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

class DaggerXL_Game {
public:
    DaggerXL_Game(const XLEngine_Plugin_API *API);

    ~DaggerXL_Game(void);

    void FixedUpdate();

    void VariableUpdate(float dt);

    void PreRender(float dt);

    void PostRender(float dt);

    void KeyDown(int32_t key);

    void GetVersion(int32_t &major, int32_t &minor) {
        major = m_nVersionMajor;
        minor = m_nVersionMinor;
    }

    void NewGame();

    bool PlaceNPC(int32_t newNPC = -1);

    void CreateNPCs();

    static void WorldUpdate(int32_t newWorldX, int32_t newWorldY, XLEngine_Plugin_API *API, void *pUserData);

private:
    struct Color {
        uint8_t red;
        uint8_t green;
        uint8_t blue;
    };

    struct ColHeader {
        int32_t fileSize;
        uint16_t FileFormatMajor;
        uint16_t FileFormatMinor;
    };

    void LoadPals();

    void LoadPal(struct Color *pPalData, const char *pszFile);

    void LoadCol(struct Color *pPalData, const char *pszFile);

    void LoadColormap(uint8_t *pColormap, const char *pszFile);

    void ClearColorToColor(uint8_t *pColormap, uint32_t color, uint32_t newColor);

    const XLEngine_Plugin_API *m_pAPI;
    int32_t m_nVersionMajor;
    int32_t m_nVersionMinor;
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

    static void CC_GameVersion(const vector<string> &args, void *pUserData);
};

#endif //BLOODXL_GAME_H
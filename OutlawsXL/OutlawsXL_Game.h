#ifndef OUTLAWSXL_GAME_H
#define OUTLAWSXL_GAME_H

#include "../plugin_framework/plugin.h"

#include <string>

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

    void KeyDown(int32_t key);
    void GetVersion(int32_t& major, int32_t& minor) { major = m_nVersionMajor; minor = m_nVersionMinor; }
    void NewGame(int32_t episode, int32_t difficulty);

private:
    const XLEngine_Plugin_API *m_pAPI;
    int32_t m_nVersionMajor;
    int32_t m_nVersionMinor;

    OutlawsXL_Player *m_Player;

    //Script commands
    static void SC_Game_NewGameSettings(int episode, int difficulty);

    //Console commands
    static OutlawsXL_Game *s_pGame_Console;
    static void CC_GameVersion(const std::vector<std::string>& args, void *pUserData);
    static void CC_LoadMap(const std::vector<std::string>& args, void *pUserData);
    static void CC_PassThruAdjoins(const std::vector<std::string>& args, void *pUserData);
};

#endif //BLOODXL_GAME_H
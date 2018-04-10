#ifndef DARKXL_GAME_H
#define DARKXL_GAME_H

#include "../plugin_framework/plugin.h"
#include <string>

class DarkXL_Game
{
public:
    DarkXL_Game(const XLEngine_Plugin_API *API);
    ~DarkXL_Game();

    void FixedUpdate();
    void VariableUpdate(float dt);
    void PreRender(float dt);
    void PostRender(float dt);

    void KeyDown(int32_t key);

    void GetVersion(int32_t& major, int32_t& minor) { major = m_nVersionMajor; minor = m_nVersionMinor; }

private:
    const XLEngine_Plugin_API *m_pAPI;
    int32_t m_nVersionMajor;
    int32_t m_nVersionMinor;

    //Console commands
    static DarkXL_Game *s_pGame_Console;
    static void CC_GameVersion(const std::vector<std::string>& args, void *pUserData);
};

#endif //DARKXL_GAME_H
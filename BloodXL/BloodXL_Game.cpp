#include "BloodXL_Game.h"
#include "BloodXL_Player.h"
#include "../fileformats/ArchiveTypes.h"
#include "../fileformats/CellTypes.h"
#include "../ui/Console.h"

//Game instance used for console commands.
BloodXL_Game *BloodXL_Game::s_pGame_Console = NULL;

BloodXL_Game::BloodXL_Game(const XLEngine_Plugin_API *API) {
    m_pAPI = API;

    m_nVersionMajor = 0;
    m_nVersionMinor = 10;

    m_pAPI->SetConsoleColor(0.20f, 0.025f, 0.025f, 0.85f);

    m_pAPI->SetGameData("BloodXL", m_nVersionMajor, m_nVersionMinor);
    m_pAPI->PrintToConsole("Starting BloodXL version %d.%03d", m_nVersionMajor, m_nVersionMinor);

    //Add game specific console commands.
    s_pGame_Console = this;
    m_pAPI->RegisterConsoleCmd("g_version", (void *) CC_GameVersion, Console::CTYPE_FUNCTION,
                               "Prints out the name and version of the currently loaded game.", this);
    m_pAPI->RegisterConsoleCmd("g_loadmap", (void *) CC_LoadMap, Console::CTYPE_FUNCTION,
                               "Load a map, for example: g_loadmap e1m1", this);
    m_pAPI->RegisterConsoleCmd("g_passThruAdjoins", (void *) CC_PassThruAdjoins, Console::CTYPE_FUNCTION,
                               "Always pass through adjoins, 0 = default.", this);

    m_pAPI->RegisterScriptFunc("void Game_NewGame(int, int)", asFUNCTION(SC_Game_NewGameSettings));
    m_pAPI->RegisterScriptFunc("void Game_LoadMap(string &in)", asFUNCTION(SC_Game_LoadMap));

    //We only worry about the UI scripts, palettes, color maps and so on if we're running a client.
    if (m_pAPI->IsServer() == false)
    {
        //Start the UI script for title screen.
        m_pAPI->Start_UI_Script("BloodXL/CoreUI.as");

        //load the game palette so that the UI is correct.
        uint8_t *pal = xlNew uint8_t[768 * 3 + 1];
        uint8_t *new_pal = xlNew uint8_t[768 * 3 + 1];
        if (m_pAPI->GameFile_Open(ARCHIVETYPE_RFF, "BLOOD.RFF", "BLOOD.PAL"))
        {
            uint32_t len = m_pAPI->GameFile_GetLength();
            m_pAPI->GameFile_Read(pal, len);
            m_pAPI->GameFile_Close();

            m_pAPI->SetGamePalette(0, pal, 768, 255);
        }

        static const char *aszPLU_Files[] =
                {
                        "NORMAL.PLU",
                        "SATURATE.PLU",
                        "BEAST.PLU",
                        "TOMMY.PLU",
                        "SPIDER3.PLU",
                        "GRAY.PLU",
                        "GRAYISH.PLU",
                        "SPIDER1.PLU",
                        "SPIDER2.PLU",
                        "FLAME.PLU",
                        "COLD.PLU",
                        "P1.PLU",
                        "P2.PLU",
                        "P3.PLU",
                        "P4.PLU"
                };
        //load the "PLU" files for different palettes.
        uint8_t *pData = xlNew uint8_t[16385];
        for (uint32_t i = 1; i < 14; i++)
        {
            if (m_pAPI->GameFile_Open(ARCHIVETYPE_RFF, "BLOOD.RFF", aszPLU_Files[i]))
            {
                uint32_t len = m_pAPI->GameFile_GetLength();
                m_pAPI->GameFile_Read(pData, len);
                m_pAPI->GameFile_Close();

                for (uint32_t c = 0; c < 256; c++)
                {
                    new_pal[c * 3 + 0] = pal[pData[c] * 3 + 0];
                    new_pal[c * 3 + 1] = pal[pData[c] * 3 + 1];
                    new_pal[c * 3 + 2] = pal[pData[c] * 3 + 2];
                }

                m_pAPI->SetGamePalette(i, new_pal, 768, 255);
            }
        }
        xlDelete[] pData;
        xlDelete[] pal;
        xlDelete[] new_pal;
    }

    m_Player = xlNew BloodXL_Player(API);

    //If we are running a server, then just load the startup map.
    if (m_pAPI->IsServer() == true)
    {
        SC_Game_LoadMap(m_pAPI->Startup_GetStartMap());
    }
}

BloodXL_Game::~BloodXL_Game(void) {
    if (m_Player)
    {
        xlDelete m_Player;
        m_Player = NULL;
    }
}

void BloodXL_Game::FixedUpdate() {
}

void BloodXL_Game::VariableUpdate(float dt) {
}

void BloodXL_Game::PreRender(float dt) {
}

void BloodXL_Game::PostRender(float dt) {
}

void BloodXL_Game::KeyDown(int32_t key) {
    m_Player->KeyDown(key);
}

void BloodXL_Game::NewGame(int32_t episode, int32_t difficulty) {
    char szMapName[32];
    sprintf(szMapName, "E%dM1.MAP", episode + 1);

    m_pAPI->World_UnloadAllCells();
    m_pAPI->World_LoadCell(CELLTYPE_BLOOD_MAP, ARCHIVETYPE_RFF, "BLOOD.RFF", szMapName, 0, 0);
}

/************************
 *** Script Commands  ***
 ************************/
void BloodXL_Game::SC_Game_NewGameSettings(int episode, int difficulty) {
    s_pGame_Console->NewGame(episode, difficulty);
}

void BloodXL_Game::SC_Game_LoadMap(const string &mapName) {
    string mapNameFinal = mapName + ".MAP";
    s_pGame_Console->m_pAPI->World_UnloadAllCells();
    s_pGame_Console->m_pAPI->World_LoadCell(CELLTYPE_BLOOD_MAP, ARCHIVETYPE_RFF, "BLOOD.RFF", mapNameFinal.c_str(), 0,
                                            0);
}

/************************
 *** Console commands ***
 ************************/
void BloodXL_Game::CC_GameVersion(const vector<string> &args, void *pUserData) {
    s_pGame_Console->m_pAPI->PrintToConsole("BloodXL version %d.%03d", s_pGame_Console->m_nVersionMajor,
                                            s_pGame_Console->m_nVersionMinor);
}

void BloodXL_Game::CC_LoadMap(const vector<string> &args, void *pUserData) {
    BloodXL_Game *pGame = (BloodXL_Game *) pUserData;
    if (args.size() < 2)
    {
        s_pGame_Console->m_pAPI->PrintToConsole("Usage: g_loadmap mapName");
    }
    else
    {
        string mapName = args[1] + ".MAP";
        pGame->m_pAPI->World_UnloadAllCells();
        pGame->m_pAPI->World_LoadCell(CELLTYPE_BLOOD_MAP, ARCHIVETYPE_RFF, "BLOOD.RFF", mapName.c_str(), 0, 0);
    }
}

void BloodXL_Game::CC_PassThruAdjoins(const vector<string> &args, void *pUserData) {
    BloodXL_Game *pGame = (BloodXL_Game *) pUserData;

    bool bPassThru = false;    //if there are no arguments then just disable (the "default")
    if (args.size() > 1)
    {
        const char *arg = args[1].c_str();
        if (arg[0] == '1')
            bPassThru = true;
    }

    pGame->m_Player->SetPassthruAdjoins(bPassThru);
}
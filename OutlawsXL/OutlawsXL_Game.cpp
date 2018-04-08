#include "OutlawsXL_Game.h"
#include "OutlawsXL_Player.h"
#include "../fileformats/ArchiveTypes.h"
#include "../fileformats/CellTypes.h"
#include "../ui/Console.h"

//Game instance used for console commands.
OutlawsXL_Game *OutlawsXL_Game::s_pGame_Console=NULL;

OutlawsXL_Game::OutlawsXL_Game(const XLEngine_Plugin_API *API)
{
    m_pAPI = API;

    m_nVersionMajor = 0;
    m_nVersionMinor = 10;

    m_pAPI->SetConsoleColor(0.20f, 0.025f, 0.025f, 0.85f);

    m_pAPI->SetGameData("OutlawsXL", m_nVersionMajor, m_nVersionMinor);
    m_pAPI->PrintToConsole("Starting OutlawsXL version %d.%03d", m_nVersionMajor, m_nVersionMinor);

    //Add game specific console commands.
    s_pGame_Console = this;
    m_pAPI->RegisterConsoleCmd("g_version", (void *)CC_GameVersion, Console::CTYPE_FUNCTION, "Prints out the name and version of the currently loaded game.", this);
    m_pAPI->RegisterConsoleCmd("g_loadmap", (void *)CC_LoadMap, Console::CTYPE_FUNCTION, "Load a map, for example: g_loadmap e1m1", this);
    m_pAPI->RegisterConsoleCmd("g_passThruAdjoins", (void *)CC_PassThruAdjoins, Console::CTYPE_FUNCTION, "Always pass through adjoins, 0 = default.", this);

    m_pAPI->RegisterScriptFunc("void Game_NewGame(int, int)", asFUNCTION(SC_Game_NewGameSettings)); 

    //Start the UI script for title screen.
    m_pAPI->Start_UI_Script( "OutlawsXL/CoreUI.as" );

    m_Player = xlNew OutlawsXL_Player(API);
}

OutlawsXL_Game::~OutlawsXL_Game(void)
{
    if ( m_Player )
    {
        xlDelete m_Player;
        m_Player = NULL;
    }
}

void OutlawsXL_Game::FixedUpdate()
{
}

void OutlawsXL_Game::VariableUpdate(float dt)
{
}

void OutlawsXL_Game::PreRender(float dt)
{
}

void OutlawsXL_Game::PostRender(float dt)
{
}

void OutlawsXL_Game::KeyDown(int32_t key)
{
}

void OutlawsXL_Game::NewGame(int32_t episode, int32_t difficulty)
{
    m_pAPI->World_UnloadAllCells();
    m_pAPI->World_LoadCell( CELLTYPE_OUTLAWS_MAP, ARCHIVETYPE_LAB, "OLGEO_1.LAB", "HIDEOUT.LVT", 0, 0 );
}

/************************
 *** Script Commands  ***
 ************************/
void OutlawsXL_Game::SC_Game_NewGameSettings(int episode, int difficulty)
{
    s_pGame_Console->NewGame(episode, difficulty);
}

/************************
 *** Console commands ***
 ************************/
void OutlawsXL_Game::CC_GameVersion(const vector<string>& args, void *pUserData)
{
    OutlawsXL_Game *pGame = (OutlawsXL_Game *)pUserData;
    pGame->m_pAPI->PrintToConsole("OutlawsXL version %d.%03d", pGame->m_nVersionMajor, pGame->m_nVersionMinor);
}

void OutlawsXL_Game::CC_LoadMap(const vector<string>& args, void *pUserData)
{
    OutlawsXL_Game *pGame = (OutlawsXL_Game *)pUserData;
    if ( args.size() < 2 )
    {
        pGame->m_pAPI->PrintToConsole("Usage: g_loadmap mapName");
    }
    else
    {
        string mapName = args[1] + ".LVT";

        pGame->m_pAPI->World_UnloadAllCells();
        pGame->m_pAPI->World_LoadCell( CELLTYPE_OUTLAWS_MAP, ARCHIVETYPE_LAB, "OLGEO_1.LAB", mapName.c_str(), 0, 0 );
    }
}

void OutlawsXL_Game::CC_PassThruAdjoins(const vector<string>& args, void *pUserData)
{
    OutlawsXL_Game *pGame = (OutlawsXL_Game *)pUserData;

    bool bPassThru = false; //if there are no arguments then just disable (the "default")
    if ( args.size() > 1 )
    {
        const char *arg = args[1].c_str();
        if ( arg[0] == '1' )
            bPassThru = true;
    }
    //Pass bPassThru to the player here.
    pGame->m_Player->SetPassthruAdjoins( bPassThru );
}

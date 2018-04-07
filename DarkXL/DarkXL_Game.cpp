#include "DarkXL_Game.h"
#include "CutscenePlayer.h"
#include "../ui/Console.h"

//Game instance used for console commands.
DarkXL_Game *DarkXL_Game::s_pGame_Console=NULL;

DarkXL_Game::DarkXL_Game(const XLEngine_Plugin_API *API)
{
	m_pAPI = API;

	m_nVersionMajor = 9;
	m_nVersionMinor = 500;

	m_pAPI->SetConsoleColor(0.125f, 0.1875f, 0.125f, 0.85f);

	m_pAPI->SetGameData("DarkXL", m_nVersionMajor, m_nVersionMinor);
	m_pAPI->PrintToConsole("Starting DarkXL version %d.%03d", m_nVersionMajor, m_nVersionMinor);

	//Add game specific console commands.
	s_pGame_Console = this;
	m_pAPI->RegisterConsoleCmd("g_version", (void *)CC_GameVersion, Console::CTYPE_FUNCTION, "Prints out the name and version of the currently loaded game.", this);

	//Add game specific script commands.
	m_pAPI->RegisterScriptFunc("void Game_StartCutscene(int)", asFUNCTION(CutscenePlayer::StartCutscene)); 
	m_pAPI->RegisterScriptFunc("int Game_IsCutscenePlaying()", asFUNCTION(CutscenePlayer::IsCutscenePlaying)); 

	//Start the UI script for title screen.
	m_pAPI->Start_UI_Script( "DarkXL/CoreUI.as" );

	//Initialize the cutscene player. This will load the list of cutscenes for future playback.
	CutscenePlayer::Init(API);
}

DarkXL_Game::~DarkXL_Game(void)
{
}

void DarkXL_Game::FixedUpdate()
{
	if ( CutscenePlayer::IsCutscenePlaying() )
	{
		CutscenePlayer::Update();
	}
}

void DarkXL_Game::VariableUpdate(float dt)
{
}

void DarkXL_Game::PreRender(float dt)
{
}

void DarkXL_Game::PostRender(float dt)
{
	if ( CutscenePlayer::IsCutscenePlaying() )
	{
		CutscenePlayer::Render(dt);
	}
}

void DarkXL_Game::KeyDown(int32_t key)
{
	if ( CutscenePlayer::IsCutscenePlaying() )
	{
		CutscenePlayer::KeyDown(key);
	}
}

/************************
 *** Script Commands  ***
 ************************/

/************************
 *** Console commands ***
 ************************/
void DarkXL_Game::CC_GameVersion(const vector<string>& args, void *pUserData)
{
	s_pGame_Console->m_pAPI->PrintToConsole("DarkXL version %d.%03d", s_pGame_Console->m_nVersionMajor, s_pGame_Console->m_nVersionMinor);
}

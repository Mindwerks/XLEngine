#include "BloodXL_main.h"
#include "BloodXL_Game.h"

//this is static for now.
BloodXL_Game *m_pGame=NULL;

//Interface between Game classes and C library interface.
void BloodXL_Update(int32_t stage, f32 dt, XLEngine_Plugin_API *API, void *pUserData)
{
	if ( stage == UPDATE_STAGE_FIXED )
	{
		m_pGame->FixedUpdate();
	}
	else
	{
		m_pGame->VariableUpdate(dt);
	}
}

void BloodXL_Render(int32_t stage, f32 dt, XLEngine_Plugin_API *API, void *pUserData)
{
	if ( stage == RENDER_STAGE_PREWORLD )
	{
		m_pGame->PreRender(dt);
	}
	else
	{
		m_pGame->PostRender(dt);
	}
}

void BloodXL_KeyDownCallback(int32_t key)
{
	m_pGame->KeyDown(key);
}

void BloodXL_CreateGame(const XLEngine_Plugin_API *API)
{
	m_pGame = xlNew BloodXL_Game(API);
}

void BloodXL_DestroyGame()
{
	if ( m_pGame )
	{
		xlDelete m_pGame;
	}
	m_pGame = NULL;
}

//Dynamic library C interface.
extern "C" PLUGIN_API int32_t ExitFunc()
{
	BloodXL_DestroyGame();

	return 0;
}

extern "C" PLUGIN_API XL_ExitFunc XL_initPlugin(const XLEngine_Plugin_API *API)
{
	API->SetGameUpdateCallback( BloodXL_Update, NULL );
	API->SetGameRenderCallback( BloodXL_Render, NULL );
	API->AddKeyDownCallback( BloodXL_KeyDownCallback, Input::KDCb_FLAGS_NONE );
	
	BloodXL_CreateGame( API );
	
	return ExitFunc;
}

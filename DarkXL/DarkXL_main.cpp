#include "DarkXL_main.h"
#include "DarkXL_Game.h"

//this is static for now.
DarkXL_Game *m_pGame=nullptr;

//Interface between Game classes and C library interface.
void DarkXL_Update(int32_t stage, float dt, XLEngine_Plugin_API *API, void *pUserData)
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

void DarkXL_Render(int32_t stage, float dt, XLEngine_Plugin_API *API, void *pUserData)
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

void DarkXL_KeyDownCallback(int32_t key)
{
    m_pGame->KeyDown(key);
}

void DarkXL_CreateGame(const XLEngine_Plugin_API *API)
{
    m_pGame = xlNew DarkXL_Game(API);
}

void DarkXL_DestroyGame()
{
    if ( m_pGame )
    {
        xlDelete m_pGame;
    }
    m_pGame = nullptr;
}

//Dynamic library C interface.
extern "C" PLUGIN_API int32_t ExitFunc()
{
    DarkXL_DestroyGame();

    return 0;
}

extern "C" PLUGIN_API XL_ExitFunc XL_initPlugin(const XLEngine_Plugin_API *API)
{
    API->SetGameUpdateCallback( DarkXL_Update, nullptr );
    API->SetGameRenderCallback( DarkXL_Render, nullptr );
    API->AddKeyDownCallback( DarkXL_KeyDownCallback, Input::KDCb_FLAGS_NONE );
    
    DarkXL_CreateGame( API );
    
    return ExitFunc;
}

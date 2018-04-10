#include "DaggerXL_main.h"
#include "DaggerXL_Game.h"

//this is static for now.
DaggerXL_Game *m_pGame=nullptr;

void DaggerXL_Update(int32_t stage, float dt, XLEngine_Plugin_API *API, void *pUserData)
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

void DaggerXL_Render(int32_t stage, float dt, XLEngine_Plugin_API *API, void *pUserData)
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

void DaggerXL_KeyDownCallback(int32_t key)
{
    m_pGame->KeyDown(key);
}

void DaggerXL_CreateGame(const XLEngine_Plugin_API *API)
{
    m_pGame = xlNew DaggerXL_Game(API);
}

void DaggerXL_DestroyGame()
{
    if ( m_pGame )
    {
        xlDelete m_pGame;
    }
    m_pGame = nullptr;
}

extern "C" PLUGIN_API int32_t ExitFunc()
{
    DaggerXL_DestroyGame();

    return 0;
}

extern "C" PLUGIN_API XL_ExitFunc XL_initPlugin(const XLEngine_Plugin_API *API)
{
    API->SetGameUpdateCallback( DaggerXL_Update, nullptr );
    API->SetGameRenderCallback( DaggerXL_Render, nullptr );
    API->AddKeyDownCallback( DaggerXL_KeyDownCallback, Input::KDCb_FLAGS_NOREPEAT );
    
    DaggerXL_CreateGame( API );
    
    return ExitFunc;
}

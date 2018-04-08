#include "OutlawsXL_main.h"
#include "OutlawsXL_Game.h"

//this is static for now.
OutlawsXL_Game *m_pGame=NULL;

//Interface between Game classes and C library interface.
void OutlawsXL_Update(int32_t stage, float dt, XLEngine_Plugin_API *API, void *pUserData)
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

void OutlawsXL_Render(int32_t stage, float dt, XLEngine_Plugin_API *API, void *pUserData)
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

void OutlawsXL_KeyDownCallback(int32_t key)
{
    m_pGame->KeyDown(key);
}

void OutlawsXL_CreateGame(const XLEngine_Plugin_API *API)
{
    m_pGame = xlNew OutlawsXL_Game(API);
}

void OutlawsXL_DestroyGame()
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
    OutlawsXL_DestroyGame();

    return 0;
}

extern "C" PLUGIN_API XL_ExitFunc XL_initPlugin(const XLEngine_Plugin_API *API)
{
    API->SetGameUpdateCallback( OutlawsXL_Update, NULL );
    API->SetGameRenderCallback( OutlawsXL_Render, NULL );
    API->AddKeyDownCallback( OutlawsXL_KeyDownCallback, Input::KDCb_FLAGS_NONE );

    OutlawsXL_CreateGame( API );

    return ExitFunc;
}

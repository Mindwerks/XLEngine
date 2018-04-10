#include "Input.h"
#include "../Engine.h"
#include "../scriptsystem/ScriptSystem.h"
#if PLATFORM_WIN       //the Windows mapping to Virtual keys.
    #include "Win/KeyOSMapping_Win.h"
#elif PLATFORM_LINUX   //the Linux mapping to Virtual keys.
    #include "linux/KeyOSMapping_Linux.h"
    #include <memory.h>
#endif

int8_t Input::m_aKeyState[512];
std::vector<Input::KeyDownCB_t *> Input::m_KeyDownCB;
std::vector<Input::KeyDownCB_t *> Input::m_CharDownCB;
float Input::m_fMouseX;
float Input::m_fMouseY;
float Input::m_fMouseDeltaX;
float Input::m_fMouseDeltaY;
XL_BOOL Input::m_bLockMouse;

void Input::Init()
{
    memset(m_aKeyState, 0, 512);
    m_fMouseX      = 0.0f;
    m_fMouseY      = 0.0f;
    m_fMouseDeltaX = 0.0f;
    m_fMouseDeltaY = 0.0f;
    m_bLockMouse   = false;

    //setup script functions.
    ScriptSystem::RegisterFunc("void Input_EnableMouseLocking(int)", asFUNCTION(EnableMouseLocking));
    ScriptSystem::RegisterFunc("int Input_GetMouseLockState(void)",  asFUNCTION(LockMouse));
}

void Input::Destroy()
{
    for (KeyDownCB_t *keyCB : m_KeyDownCB)
    {
        xlDelete keyCB;
    }

    m_KeyDownCB.clear();

    for (KeyDownCB_t *charCB : m_CharDownCB)
    {
        xlDelete charCB;
    }

    m_CharDownCB.clear();
}

void Input::EnableMouseLocking(XL_BOOL bEnable) 
{ 
    m_bLockMouse   = bEnable; 
    m_fMouseDeltaX = 0.0f; 
    m_fMouseDeltaY = 0.0f; 
}

void Input::SetKeyDown(int32_t key)
{
    //now remap from local OS keys to global virtual keys (XL_* from VirtualKeys.h)
    key = s_OS_KeyMapping[key];

    //now fire off any callbacks...
    std::vector<KeyDownCB_t *>::iterator iter = m_KeyDownCB.begin();
    std::vector<KeyDownCB_t *>::iterator end  = m_KeyDownCB.end();
    for (; iter != end; ++iter)
    {
        KeyDownCB_t *pKeyDownCB = *iter;

        bool bFireCB = true;
        if ( (pKeyDownCB->nFlags&KDCb_FLAGS_NOREPEAT) && (m_aKeyState[key] != 0) )
             bFireCB = false;

        if ( bFireCB )
        {
            pKeyDownCB->pCB(key);
        }
    }
    m_aKeyState[key] = 1;
    if ( key == XL_LSHIFT || key == XL_RSHIFT )
    {
        m_aKeyState[XL_SHIFT] = 1;
    }
}

void Input::SetKeyUp(int32_t key)
{
    key = s_OS_KeyMapping[key];
    m_aKeyState[key] = 0;
}

void Input::SetCharacterDown(char c)
{
    //ASCII character range...
    if ( c >= 32 && c < 127 )
    {
        //fire off any "character" callbacks.
        std::vector<KeyDownCB_t *>::iterator iter = m_CharDownCB.begin();
        std::vector<KeyDownCB_t *>::iterator end  = m_CharDownCB.end();
        for (; iter != end; ++iter)
        {
            KeyDownCB_t *pCharDownCB = *iter;
            if ( pCharDownCB )
            {
                pCharDownCB->pCB(c);
            }
        }
    }
}

void Input::ClearAllKeys()
{
    memset(m_aKeyState, 0, 512);
}

void Input::SetMousePos(float x, float y)
{
    m_fMouseX = x;
    m_fMouseY = y;
}

bool Input::AddKeyDownCallback(Input_KeyDownCB pCB, int32_t nFlags)
{
    KeyDownCB_t *pKeyDownCB = xlNew KeyDownCB_t;
    if (pKeyDownCB == nullptr)
        return false;

    pKeyDownCB->pCB    = pCB;
    pKeyDownCB->nFlags = nFlags;
    m_KeyDownCB.push_back( pKeyDownCB );

    return true;
}

bool Input::AddCharDownCallback( Input_KeyDownCB pCB )
{
    KeyDownCB_t *pCharDownCB = xlNew KeyDownCB_t;
    if (pCharDownCB == nullptr)
        return false;

    pCharDownCB->pCB    = pCB;
    pCharDownCB->nFlags = KDCb_FLAGS_NONE;
    m_CharDownCB.push_back( pCharDownCB );

    return true;
}

/*******************************
  Plugin API
 *******************************/
int32_t Input_IsKeyDown(int32_t key)
{
    return Input::IsKeyDown(key) ? 1 : 0;
}

float Input_GetMousePosX()
{
    return Input::GetMouseX();
}

float Input_GetMousePosY()
{
    return Input::GetMouseY();
}

int32_t Input_AddKeyDownCB(Input_KeyDownCB pCB, int32_t nFlags)
{
    return Input::AddKeyDownCallback(pCB, nFlags) ? 1 : 0;
}

int32_t Input_AddCharDownCB(Input_KeyDownCB pCB)
{
    return Input::AddCharDownCallback(pCB) ? 1 : 0;
}

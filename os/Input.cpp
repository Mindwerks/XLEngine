#include "Input.h"
#include "../Engine.h"
#include "../scriptsystem/ScriptSystem.h"
#if PLATFORM_WIN	   //the Windows mapping to Virtual keys.
	#include "Win/KeyOSMapping_Win.h"
#elif PLATFORM_LINUX   //the Linux mapping to Virtual keys.
	#include "linux/KeyOSMapping_Linux.h"
	#include <memory.h>
#endif

s8 Input::m_aKeyState[512];
vector<Input::KeyDownCB_t *> Input::m_KeyDownCB;
vector<Input::KeyDownCB_t *> Input::m_CharDownCB;
f32 Input::m_fMouseX;
f32 Input::m_fMouseY;
f32 Input::m_fMouseDeltaX;
f32 Input::m_fMouseDeltaY;
XL_BOOL Input::m_bLockMouse;

void Input::Init()
{
	memset(m_aKeyState, 0, 512);
	m_fMouseX      = 0.0f;
	m_fMouseY      = 0.0f;
	m_fMouseDeltaX = 0.0f;
	m_fMouseDeltaY = 0.0f;
	m_bLockMouse   = XL_FALSE;

	//setup script functions.
	ScriptSystem::RegisterFunc("void Input_EnableMouseLocking(int)", asFUNCTION(EnableMouseLocking));
	ScriptSystem::RegisterFunc("int Input_GetMouseLockState(void)",  asFUNCTION(LockMouse));
}

void Input::Destroy()
{
	vector<KeyDownCB_t *>::iterator iKeyCB = m_KeyDownCB.begin();
	vector<KeyDownCB_t *>::iterator eKeyCB = m_KeyDownCB.end();
	for (; iKeyCB != eKeyCB; ++iKeyCB)
	{
		xlDelete (*iKeyCB);
	}
	m_KeyDownCB.clear();

	vector<KeyDownCB_t *>::iterator iCharCB = m_CharDownCB.begin();
	vector<KeyDownCB_t *>::iterator eCharCB = m_CharDownCB.end();
	for (; iCharCB != eCharCB; ++iCharCB)
	{
		xlDelete (*iCharCB);
	}
	m_CharDownCB.clear();
}

void Input::EnableMouseLocking(XL_BOOL bEnable) 
{ 
	m_bLockMouse   = bEnable; 
	m_fMouseDeltaX = 0.0f; 
	m_fMouseDeltaY = 0.0f; 
}

void Input::SetKeyDown(s32 key)
{
	//now remap from local OS keys to global virtual keys (XL_* from VirtualKeys.h)
	key = s_OS_KeyMapping[key];

	//now fire off any callbacks...
	vector<KeyDownCB_t *>::iterator iter = m_KeyDownCB.begin();
	vector<KeyDownCB_t *>::iterator end  = m_KeyDownCB.end();
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

void Input::SetKeyUp(s32 key)
{
	m_aKeyState[key] = 0;
}

void Input::SetCharacterDown(char c)
{
	//ASCII character range...
	if ( c >= 32 && c < 127 )
	{
		//fire off any "character" callbacks.
		vector<KeyDownCB_t *>::iterator iter = m_CharDownCB.begin();
		vector<KeyDownCB_t *>::iterator end  = m_CharDownCB.end();
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

void Input::SetMousePos(f32 x, f32 y)
{
	m_fMouseX = x;
	m_fMouseY = y;
}

bool Input::AddKeyDownCallback(Input_KeyDownCB pCB, s32 nFlags)
{
	KeyDownCB_t *pKeyDownCB = xlNew KeyDownCB_t;
	if (pKeyDownCB == NULL)
		return false;

	pKeyDownCB->pCB    = pCB;
	pKeyDownCB->nFlags = nFlags;
	m_KeyDownCB.push_back( pKeyDownCB );

	return true;
}

bool Input::AddCharDownCallback( Input_KeyDownCB pCB )
{
	KeyDownCB_t *pCharDownCB = xlNew KeyDownCB_t;
	if (pCharDownCB == NULL)
		return false;

	pCharDownCB->pCB    = pCB;
	pCharDownCB->nFlags = KDCb_FLAGS_NONE;
	m_CharDownCB.push_back( pCharDownCB );

	return true;
}

/*******************************
  Plugin API
 *******************************/
s32 Input_IsKeyDown(s32 key)
{
	return Input::IsKeyDown(key) ? 1 : 0;
}

f32 Input_GetMousePosX(void)
{
	return Input::GetMouseX();
}

f32 Input_GetMousePosY(void)
{
	return Input::GetMouseY();
}

s32 Input_AddKeyDownCB(Input_KeyDownCB pCB, s32 nFlags)
{
	return Input::AddKeyDownCallback(pCB, nFlags) ? 1 : 0;
}

s32 Input_AddCharDownCB(Input_KeyDownCB pCB)
{
	return Input::AddCharDownCallback(pCB) ? 1 : 0;
}
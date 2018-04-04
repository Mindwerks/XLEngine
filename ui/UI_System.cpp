#include "UI_System.h"
#include "XL_Console.h"
#include "../render/IDriver3D.h"
#include "../render/TextureCache.h"
#include "../render/FontManager.h"
#include "../fileformats/ArchiveManager.h"
#include "../fileformats/Archive.h"
#include "../fileformats/LFD_Anim.h"
#include "../fileformats/TextureTypes.h"
#include "../math/Math.h"
#include "../EngineSettings.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

/****************************
 ***** Static Variables *****
 ****************************/
#define MAX_RENDERFRAMEPOOL_SIZE 16384
#define MAX_LFD_ANIM 16

bool UI_System::m_bScriptsLoaded;
bool UI_System::m_bScriptExeSucceeded;
UI_Screen *UI_System::m_Top;		//the current top-level screen.
UI_Screen *UI_System::m_Leaf;	//the current bottom level screen (this absorbs the input).
UI_Screen *UI_System::m_Context;	//the current UI execute context, basically which UI_Screen the script is currently operating on.
UI_Screen *UI_System::m_PendingScreenChange;
UI_Screen *UI_System::m_PushScreen;
UI_Screen *UI_System::m_PopScreen;
//f32 UI_System::m_fVirt_Scr_W;
//f32 UI_System::m_fVirt_Scr_H;
bool UI_System::m_bFlipX;
bool UI_System::m_bFlipY;
map<string, UI_Screen *> UI_System::m_ScreenMap;
vector<UI_Screen *> UI_System::m_ScreenList;
vector<UI_Window *> UI_System::m_WindowList;
IDriver3D *UI_System::m_pDriver;
Engine *UI_System::m_pEngine;
Vector2 UI_System::m_uvTop(0,0);
Vector2 UI_System::m_uvBot(1,1);
UI_RenderFrame *m_pUI_Atlas=NULL;

UI_RenderFrame *UI_System::m_pRenderFramePool=NULL;
LFD_Anim *UI_System::m_pLFD_Anim_List[MAX_LFD_ANIM];

int Editor_UI_Atlas;

u32 UI_System::m_auImage_TexType[]=
{
	TEXTURETYPE_ART,
	TEXTURETYPE_PCX,
	TEXTURETYPE_IMG,	//Daggerfall format.
};

u32 UI_System::m_auImage_ArchiveType[]=
{
	ARCHIVETYPE_ART,
	ARCHIVETYPE_LAB,
	ARCHIVETYPE_NONE,	//Daggerfall image files are loose files.
};

/**************************
 ***** Implementation *****
 **************************/
bool UI_System::Init(IDriver3D *pDriver, Engine *pEngine)
{
	m_bScriptsLoaded = false;
	m_bScriptExeSucceeded = false;
	m_Top = NULL;
	m_Leaf = NULL;
	m_Context = NULL;
	m_PendingScreenChange = NULL;
	m_PushScreen = NULL;
	m_PopScreen = NULL;

	//m_fVirt_Scr_W = 320.0f;
	//m_fVirt_Scr_H = 200.0f;
	m_bFlipX = false;
	m_bFlipY = false;

	m_pDriver = pDriver;
	m_pEngine = pEngine;

	AllocRenderFramePool();
	memset(m_pLFD_Anim_List, 0, sizeof(LFD_Anim *)*MAX_LFD_ANIM);

	Input::AddKeyDownCallback( KeyDownCallback );
	
	return true;
}

void UI_System::Destroy()
{
	if ( m_pUI_Atlas )
	{
		UI_FreeImage( Editor_UI_Atlas );
	}

	vector<UI_Screen *>::iterator iScreen = m_ScreenList.begin();
	vector<UI_Screen *>::iterator eScreen = m_ScreenList.end();
	for (; iScreen != eScreen; ++iScreen)
	{
		xlDelete *iScreen;
	}
	m_ScreenList.clear();
	m_ScreenMap.clear();

	vector<UI_Window *>::iterator iWindow = m_WindowList.begin();
	vector<UI_Window *>::iterator eWindow = m_WindowList.end();
	for (; iWindow != eWindow; ++iWindow)
	{
		xlDelete *iWindow;
	}
	m_WindowList.clear();

	FreeRenderFramePool();
}

void UI_System::StartScript(const char *pszFile)
{
	XL_Console::PrintF("Start UI Script: %s", pszFile);

	if ( m_bScriptsLoaded == false )
	{
		//UI functions
		ScriptSystem::RegisterFunc("void UI_StartScreen(string &in)", asFUNCTION(UI_StartScreen));
		ScriptSystem::RegisterFunc("void UI_CreateWindow(string &in, string &in, int, int, int, int, int, int)", asFUNCTION(UI_CreateWindow));
		ScriptSystem::RegisterFunc("void UI_CreateWindow_FromLFDFrame(string &in, int, int, int, int)", asFUNCTION(UI_CreateWindow_FromLFDFrame));
		ScriptSystem::RegisterFunc("void UI_PushWindow(string &in, string &in, int, int, int, int, int, int)", asFUNCTION(UI_PushWindow));
		ScriptSystem::RegisterFunc("void UI_PopWindow()", asFUNCTION(UI_PopWindow));
		ScriptSystem::RegisterFunc("void UI_EnableWindow(string& in, int)", asFUNCTION(UI_EnableWindow));
		ScriptSystem::RegisterFunc("int UI_AddImage(string &in, int, int)",	asFUNCTION(UI_AddImage));
		ScriptSystem::RegisterFunc("int UI_AddGameImage(int, string &in, string &in)", asFUNCTION(UI_AddGameImage));
		ScriptSystem::RegisterFunc("void UI_FreeImage(int)", asFUNCTION(UI_FreeImage));
		ScriptSystem::RegisterFunc("void UI_EnableImageFilter(int)", asFUNCTION(UI_EnableImageFilter));
		ScriptSystem::RegisterFunc("void UI_EnableImageBlending(int)", asFUNCTION(UI_EnableImageBlending));
		ScriptSystem::RegisterFunc("void UI_SetImageRenderProp(int, int)", asFUNCTION(UI_SetImageRenderProp));
		ScriptSystem::RegisterFunc("void UI_RenderImage(int, int, int, float, int, int)", asFUNCTION(UI_RenderImage));
		ScriptSystem::RegisterFunc("void UI_GetImageSize(int, int &out, int &out)", asFUNCTION(UI_GetImageSize));
		ScriptSystem::RegisterFunc("void UI_RenderRect(int, int, int, int, float, float, float, float, int, int)", asFUNCTION(UI_RenderRect));
		ScriptSystem::RegisterFunc("void UI_RenderImageRect(int, int, int, int, int, float, int, int)", asFUNCTION(UI_RenderImageRect));
		ScriptSystem::RegisterFunc("void UI_SetImageUV_Range(float, float, float, float)", asFUNCTION(UI_SetImageUV_Range));
		ScriptSystem::RegisterFunc("void UI_SetImageUV_RangeI(int, int, int, int, int)", asFUNCTION(UI_SetImageUV_RangeI));
		ScriptSystem::RegisterFunc("void UI_RenderString(string &in, int, int, int, float, float, float, float)", asFUNCTION(UI_RenderString));
		ScriptSystem::RegisterFunc("void UI_RenderPolygon(int, int[] &in, int[] &in, float, float, float, float, int, int)", asFUNCTION(UI_RenderPolygon));
		ScriptSystem::RegisterFunc("void UI_PushScreen(string &in, int, int)",	asFUNCTION(UI_PushScreen));
		ScriptSystem::RegisterFunc("void UI_PopScreen()",	asFUNCTION(UI_PopScreen));
		ScriptSystem::RegisterFunc("int UI_IsKeyDown(int)",		asFUNCTION(UI_GetVirtualKey));
		ScriptSystem::RegisterFunc("void UI_SetVirtualScreenSize(int, int)", asFUNCTION(UI_SetVirtualScreenSize));
		ScriptSystem::RegisterFunc("void UI_GetVirtualScreenSize(int &out, int &out)", asFUNCTION(UI_GetVirtualScreenSize));
		ScriptSystem::RegisterFunc("void UI_GetScreenSize(int &out, int &out)", asFUNCTION(UI_GetScreenSize));
		ScriptSystem::RegisterFunc("void UI_SetPalette(int, int)", asFUNCTION(UI_SetPalette));
		//LFD Anim
		ScriptSystem::RegisterFunc("int UI_CreateLFD_Anim(string &in, string &in, string &in)", asFUNCTION(UI_CreateLFD_Anim));
		ScriptSystem::RegisterFunc("void UI_DestroyLFD_Anim(int)", asFUNCTION(UI_DestroyLFD_Anim));
		ScriptSystem::RegisterFunc("void UI_RenderLFD_Anim(int, int, int, int)", asFUNCTION(UI_RenderLFD_Anim));
		//Mouse
		ScriptSystem::RegisterFunc("void UI_GetMousePos(int &out, int &out)", asFUNCTION(UI_GetMousePos));
		//Debug
		ScriptSystem::RegisterFunc("void UI_PrintMousePos(int, int, int)", asFUNCTION(UI_PrintMousePos));
		//
		ScriptSystem::RegisterFunc("void UI_GetStartMap(string &out)", asFUNCTION(EngineSettings::GetStartMap_StrOut));
		//Misc.
		ScriptSystem::RegisterFunc("float UI_GetBrightness()", asFUNCTION(UI_GetCurrentBrightness));
		ScriptSystem::RegisterFunc("float UI_GetSpeed()", asFUNCTION(UI_GetSpeed));

		//UI scripts.
		ScriptSystem::LoadScript( ScriptSystem::SCR_MODULE_UI, ScriptSystem::SCR_SECTION_CORE, pszFile );
		m_bScriptsLoaded = true;
	}
	else
	{
		//UI scripts.
		ScriptSystem::ReloadScript( ScriptSystem::SCR_MODULE_UI, ScriptSystem::SCR_SECTION_CORE, pszFile );
		//Now we have to "get back to where we were" as best we can.
		//we can go through all the active screens and reset the function handles.
		//Then we'll have to call OnEnter as appropriate to setup resources again.
		return;
	}
	SHANDLE uiMain = ScriptSystem::GetFunc( ScriptSystem::SCR_MODULE_UI, "UI_Main" );
	if ( uiMain >= 0 )
	{
		ScriptSystem::ExecuteFunc( uiMain );
		m_bScriptExeSucceeded = true;
	}
	else
	{
		m_bScriptExeSucceeded = false;
	}

	m_pUI_Atlas = AllocRenderFrame();
	//Load Frame texture.
	m_pUI_Atlas->hFrame = TextureCache::LoadTexture( (string)"XLEngineUI.png", false );

	//Then get the texture size and relative scale from the Texture Cache.
	s32 nOffsX, nOffsY;
	u32 uTexWidth, uTexHeight;
	f32 fRelSizeX, fRelSizeY;
	TextureCache::GetTextureSize( nOffsX, nOffsY, uTexWidth, uTexHeight, fRelSizeX, fRelSizeY );

	m_pUI_Atlas->width  = (s16)uTexWidth;
	m_pUI_Atlas->height = (s16)uTexHeight;
	m_pUI_Atlas->fRelWidth  = fRelSizeX;
	m_pUI_Atlas->fRelHeight = fRelSizeY;

	//Finally setup the image handle used by the script.
	Editor_UI_Atlas = (int)m_pUI_Atlas->ID;
}

void UI_System::Update()
{
	if ( m_PendingScreenChange )
	{
		//Remove the current screen.
		if ( m_Top )
		{
			m_Context = m_Top;
			ScriptSystem::ExecuteFunc( m_Top->m_hOnExit );
		}

		//Enter the next screen.
		m_Top = m_PendingScreenChange;
		m_PendingScreenChange = NULL;
		assert( m_Top->m_hOnEnter >= 0 );

		m_Context = m_Top;
		ScriptSystem::ExecuteFunc( m_Top->m_hOnEnter );

		m_Leaf = m_Top;
	}
	else if ( m_PushScreen )
	{
		UI_Screen *pScreen = m_Top;
		m_Leaf->m_child = m_PushScreen;
		m_PushScreen->m_parent = m_Leaf;
		m_PushScreen->m_child  = NULL;

		assert( m_PushScreen->m_hOnEnter >= 0 );
		m_Context = m_PushScreen;
		ScriptSystem::ExecuteFunc( m_PushScreen->m_hOnEnter );

		m_Leaf = m_PushScreen;
		m_PushScreen = NULL;
	}
	else if ( m_PopScreen && m_PopScreen->m_parent )
	{
		m_Context = m_PopScreen;
		ScriptSystem::ExecuteFunc( m_PopScreen->m_hOnExit );

		m_Leaf = m_PopScreen->m_parent;
		m_Leaf->m_child = NULL;
		m_PopScreen = NULL;
	}
	m_Context = m_Leaf;

	//only the leaf gets key strokes and updates.
	if ( m_Context )
	{
		ScriptSystem::ExecuteFunc( m_Context->m_hOnUpdate );

		s32 nMouseX = (u32)Input::GetMouseX();
		s32 nMouseY = (u32)Input::GetMouseY();

		s32 nScrWidth, nScrHeight;
		m_pDriver->GetWindowSize( nScrWidth, nScrHeight );

		f32 fScrToUI_X = m_Context->m_fVirt_Scr_W/(f32)nScrWidth;
		f32 fScrToUI_Y = m_Context->m_fVirt_Scr_H/(f32)nScrHeight;

		nMouseX = (s32)( (f32)nMouseX * fScrToUI_X );
		nMouseY = (s32)( (f32)nMouseY * fScrToUI_Y );

		//now update the child windows.
		UI_Window *pWin = m_Context->m_childWindow;
		UpdateChildWindows(pWin, 0, 0, nMouseX, nMouseY);
	}
}

void UI_System::UpdateChildWindows(UI_Window *pWin, int x, int y, int nMouseX, int nMouseY)
{
	while (pWin)
	{
		if ( pWin->m_bEnabled )
		{
			bool bMouseOver = false;
			if ( nMouseX >= pWin->m_x+x && nMouseX < pWin->m_x+x+pWin->m_w &&
				 nMouseY >= pWin->m_y+y && nMouseY < pWin->m_y+y+pWin->m_h )
			{
				bMouseOver = true;
			}
			pWin->Update(bMouseOver, nMouseX, nMouseY, x, y);

			if ( pWin->m_child )
			{
				UpdateChildWindows(pWin->m_child, x+pWin->m_x, y+pWin->m_y, nMouseX, nMouseY);
			}
		}

		pWin = pWin->m_sibling;
	};
}

void UI_System::Render()
{
	if ( !m_Leaf )
		return;

	m_pDriver->SetBlendMode();
	m_pDriver->EnableAlphaTest(false);
	m_pDriver->EnableCulling(false);
	m_pDriver->EnableDepthRead(false);
	m_pDriver->EnableDepthWrite(false);

	ScriptArgument arg;
	arg.uType  = SC_ARG_U32;
	arg.argU32 = 0;

	m_Context = m_Leaf;
	if ( (m_Context->m_uFlags&UIFLAG_OVERLAY) && m_Context->m_parent )
	{
		//first render the parent...
		m_Context = m_Context->m_parent;

		ScriptSystem::ExecuteFunc( m_Context->m_hOnRender, 1, &arg );
		//Render child windows.
		UI_Window *pWin = m_Context->m_childWindow;
		RenderChildWindows(pWin, 0, 0);
	}
	//now render the current screen.
	m_Context = m_Leaf;
	ScriptSystem::ExecuteFunc( m_Context->m_hOnRender, 1, &arg );
	//Render child windows.
	UI_Window *pWin = m_Context->m_childWindow;
	RenderChildWindows(pWin, 0, 0);
	//Post Render.
	if ( m_Context->m_hOnPostRender > -1 )
	{
		ScriptSystem::ExecuteFunc( m_Context->m_hOnPostRender, 1, &arg );
	}

	m_pDriver->SetBlendMode();
	m_pDriver->EnableAlphaTest(false);
	m_pDriver->EnableCulling(true);
	m_pDriver->EnableDepthRead(true);
	m_pDriver->EnableDepthWrite(true);
}

void UI_System::RenderChildWindows(UI_Window *pWin, int x, int y)
{
	while (pWin)
	{
		if ( pWin->m_bEnabled )
		{ 
			pWin->Draw(x, y);

			if ( pWin->m_child )
			{
				RenderChildWindows(pWin->m_child, x+pWin->m_x, y+pWin->m_y);
			}
		}

		pWin = pWin->m_sibling;
	};
}

UI_Screen *UI_System::AddScreen(const string& sName)
{
	UI_Screen *pScreen = NULL;
	map<string, UI_Screen *>::iterator iScreen = m_ScreenMap.find(sName);
	if ( iScreen != m_ScreenMap.end() )
	{
		pScreen = iScreen->second;
	}
	else
	{
		//time to create it.
		pScreen = xlNew UI_Screen;
		pScreen->m_sName = sName;

		m_ScreenMap[sName] = pScreen;
		m_ScreenList.push_back( pScreen );
	}
	return pScreen;
}

UI_Window *UI_System::AddWindow(const string& sName, const string& sText, u32 uType, s32 x, s32 y, s32 w, s32 h, u32 flags, UI_Window *parent)
{
	//m_Context is the parent screen.
	UI_Window *pWindow = xlNew UI_Window();
	pWindow->m_x = x;
	pWindow->m_y = y;
	pWindow->m_w = w;
	pWindow->m_h = h;
	pWindow->m_parentScreen = m_Context;
	pWindow->m_uFlags = flags;
	pWindow->m_uType = uType;
	pWindow->m_text  = sText;
	pWindow->m_name  = sName;
	pWindow->m_bEnabled = true;
	
	char szFuncName[128];
	sprintf(szFuncName, "%s_OnRender", sName.c_str());
	pWindow->m_hOnRender = ScriptSystem::GetFunc( ScriptSystem::SCR_MODULE_UI, szFuncName );
	sprintf(szFuncName, "%s_OnUpdate", sName.c_str());
	pWindow->m_hOnUpdate = ScriptSystem::GetFunc( ScriptSystem::SCR_MODULE_UI, szFuncName );
	sprintf(szFuncName, "%s_OnKey", sName.c_str());
	pWindow->m_hOnKey = ScriptSystem::GetFunc( ScriptSystem::SCR_MODULE_UI, szFuncName );
	sprintf(szFuncName, "%s_OnRelease", sName.c_str());
	pWindow->m_hOnRelease = ScriptSystem::GetFunc( ScriptSystem::SCR_MODULE_UI, szFuncName );

	if ( parent == NULL )
	{
		pWindow->m_parent = NULL;
		if ( m_Context->m_childWindow == NULL )
		{
			m_Context->m_childWindow = pWindow;
		}
		else
		{
			UI_Window *curWin = m_Context->m_childWindow;
			while (1)
			{
				if ( curWin->m_sibling == NULL )
				{
					curWin->m_sibling = pWindow;
					break;
				}
				else
				{
					curWin = curWin->m_sibling;
				}
			};
		}
	}
	else
	{
		pWindow->m_parent = parent;
		if ( parent->m_child == NULL )
		{
			parent->m_child = pWindow;
		}
		else
		{
			UI_Window *curWin = parent->m_child;
			while (1)
			{
				if ( curWin->m_sibling == NULL )
				{
					curWin->m_sibling = pWindow;
					break;
				}
				else
				{
					curWin = curWin->m_sibling;
				}
			};
		}
	}
	m_WindowList.push_back( pWindow );

	return pWindow;
}

/********************************************************
 ***** UI Script Functions (called from UI scripts) *****
 ********************************************************/
UI_Window *_pCurWindow = NULL;
UI_Window *_apWindowStack[16];
int _stackPtr = 0;
void UI_System::UI_CreateWindow(string& sName, string& sText, int type, int x, int y, int w, int h, int flags)
{
	if ( m_Context )
	{
		UI_Window *pWin = AddWindow(sName, sText, (u32)type, x, y, w, h, (u32)flags, _pCurWindow);
		assert(pWin);
	}
}

void UI_System::UI_PushWindow(string& sName, string& sText, int type, int x, int y, int w, int h, int flags)
{
	if ( m_Context && _stackPtr < 16 )
	{
		UI_Window *pWin = AddWindow(sName, sText, (u32)type, x, y, w, h, (u32)flags, _pCurWindow);
		_apWindowStack[_stackPtr] = _pCurWindow;
		_stackPtr++;
		_pCurWindow = pWin;
		assert(pWin);
	}
}

void UI_System::UI_PopWindow()
{
	if ( m_Context )
	{
		if ( _stackPtr > 0 )
		{
			_stackPtr--;
			_pCurWindow = _apWindowStack[_stackPtr];
		}
		else
		{
			_stackPtr   = 0;
			_pCurWindow = NULL;
		}
	}
}

void UI_System::UI_EnableWindow(string& sName, int enable)
{
	vector<UI_Window *>::iterator iWindow = m_WindowList.begin();
	vector<UI_Window *>::iterator eWindow = m_WindowList.end();
	for (; iWindow != eWindow; ++iWindow)
	{
		UI_Window *pWindow = *iWindow;
		if ( pWindow->m_name == sName )
		{
			pWindow->m_bEnabled = enable ? true : false;
			break;
		}
	}
}

void UI_System::UI_CreateWindow_FromLFDFrame(string& sName, int LFDAnim_ID, int frame, int x0, int y0)
{
	if ( LFDAnim_ID >= 0 && LFDAnim_ID < MAX_LFD_ANIM && m_pLFD_Anim_List[LFDAnim_ID] )
	{
		s32 frameX0, frameY0, frameWidth, frameHeight;
		m_pLFD_Anim_List[LFDAnim_ID]->GetFrameExtents(frame, (f32)x0/320.0f, (f32)y0/200.0f, frameX0, frameY0, frameWidth, frameHeight);

		UI_CreateWindow(sName, sName, 0, frameX0, frameY0, frameWidth, frameHeight, 0);
	}
}

void UI_System::UI_StartScreen(string& sUI_Start)
{
	char szFuncName[128];
	sprintf(szFuncName, "%s_OnEnter", sUI_Start.c_str());
	SHANDLE hOnEnter = ScriptSystem::GetFunc( ScriptSystem::SCR_MODULE_UI, szFuncName );
	if ( hOnEnter >= 0 )
	{
		UI_Screen *pScreen  = AddScreen( sUI_Start );
		pScreen->m_sName    = sUI_Start;
		pScreen->m_hOnEnter = hOnEnter;
		pScreen->m_uFlags   = UIFLAG_NONE;
		pScreen->m_parent   = NULL;
		pScreen->m_child    = NULL;

		sprintf(szFuncName, "%s_OnExit", sUI_Start.c_str());
		pScreen->m_hOnExit   = ScriptSystem::GetFunc( ScriptSystem::SCR_MODULE_UI, szFuncName );

		sprintf(szFuncName, "%s_OnRender", sUI_Start.c_str());
		pScreen->m_hOnRender = ScriptSystem::GetFunc( ScriptSystem::SCR_MODULE_UI, szFuncName );

		sprintf(szFuncName, "%s_OnPostRender", sUI_Start.c_str());
		pScreen->m_hOnPostRender = ScriptSystem::GetFunc( ScriptSystem::SCR_MODULE_UI, szFuncName );

		sprintf(szFuncName, "%s_OnUpdate", sUI_Start.c_str());
		pScreen->m_hOnUpdate = ScriptSystem::GetFunc( ScriptSystem::SCR_MODULE_UI, szFuncName );

		sprintf(szFuncName, "%s_OnKey", sUI_Start.c_str());
		pScreen->m_hOnKey    = ScriptSystem::GetFunc( ScriptSystem::SCR_MODULE_UI, szFuncName );

		m_PendingScreenChange = pScreen;
	}
}

void UI_System::UI_PushScreen(string& uiName, int flags, int backgrndFX)
{
	char szFuncName[128];
	sprintf(szFuncName, "%s_OnEnter", uiName.c_str());
	SHANDLE hOnEnter = ScriptSystem::GetFunc( ScriptSystem::SCR_MODULE_UI, szFuncName );
	if ( hOnEnter >= 0 )
	{
		UI_Screen *pScreen  = AddScreen(uiName);
		pScreen->m_sName    = uiName;
		pScreen->m_hOnEnter = hOnEnter;
		pScreen->m_uFlags   = (u32)flags;
		pScreen->m_parent   = NULL;
		pScreen->m_child    = NULL;

		sprintf(szFuncName, "%s_OnExit",   uiName.c_str());
		pScreen->m_hOnExit   = ScriptSystem::GetFunc( ScriptSystem::SCR_MODULE_UI, szFuncName );

		sprintf(szFuncName, "%s_OnRender", uiName.c_str());
		pScreen->m_hOnRender = ScriptSystem::GetFunc( ScriptSystem::SCR_MODULE_UI, szFuncName );

		sprintf(szFuncName, "%s_OnPostRender", uiName.c_str());
		pScreen->m_hOnPostRender = ScriptSystem::GetFunc( ScriptSystem::SCR_MODULE_UI, szFuncName );

		sprintf(szFuncName, "%s_OnUpdate", uiName.c_str());
		pScreen->m_hOnUpdate = ScriptSystem::GetFunc( ScriptSystem::SCR_MODULE_UI, szFuncName );

		sprintf(szFuncName, "%s_OnKey",    uiName.c_str());
		pScreen->m_hOnKey    = ScriptSystem::GetFunc( ScriptSystem::SCR_MODULE_UI, szFuncName );

		m_PushScreen = pScreen;
	}
}

//pops the current screen off the stack, returning control to the previous. This works for both Overlays and screens using PushScreen
void UI_System::UI_PopScreen()
{
	m_PopScreen = m_Leaf;
}

/***************************************
 ****** RenderFrame Managerment ********
 ***************************************/

//Create a RenderFrame pool.
void UI_System::AllocRenderFramePool()
{
	m_pRenderFramePool = xlNew UI_RenderFrame[MAX_RENDERFRAMEPOOL_SIZE];
	memset(m_pRenderFramePool, 0, sizeof(UI_RenderFrame)*MAX_RENDERFRAMEPOOL_SIZE);
	for (u32 i=0; i<MAX_RENDERFRAMEPOOL_SIZE; i++)
	{
		m_pRenderFramePool[i].ID = i;
	}
}

void UI_System::FreeRenderFramePool()
{
	if ( m_pRenderFramePool )
	{
		xlDelete [] m_pRenderFramePool;
	}
	m_pRenderFramePool = NULL;
}

UI_RenderFrame *UI_System::AllocRenderFrame()
{
	if ( m_pRenderFramePool == NULL )
		return NULL;

	for (u32 i=0; i<MAX_RENDERFRAMEPOOL_SIZE; i++)
	{
		if ( m_pRenderFramePool[i].bInUse == false )
		{
			m_pRenderFramePool[i].bInUse = true;
			return &m_pRenderFramePool[i];
		}
	}
	return NULL;
}

void UI_System::FreeRenderFrame(u32 uFrameID)
{
	m_pRenderFramePool[ uFrameID ].bInUse = false;
}

UI_RenderFrame *UI_System::GetRenderFrame(u32 uFrameID)
{
	return &m_pRenderFramePool[ uFrameID ];
}

/**************************************
 ****** General UI Functions **********
 *** A valid m_Context is required ****
 **************************************/

void LoadLFD_Pal(LFD_Anim *pAnim, Archive *pArchive, string& sFile)
{
	if ( ArchiveManager::GameFile_Open(pArchive, sFile.c_str()) )
	{
		u32 len = ArchiveManager::GameFile_GetLength();
		char *data = xlNew char[len+1];
		if ( data )
		{
			ArchiveManager::GameFile_Read(data, len);
			ArchiveManager::GameFile_Close();

			pAnim->SetPLTT( (PLTT_File *)data );
			xlDelete [] data;
		}
	}
}

void LoadLFD_File(LFD_Anim *pAnim, Archive *pArchive, string& sFile)
{
	if ( ArchiveManager::GameFile_Open(pArchive, sFile.c_str()) )
	{
		u32 len = ArchiveManager::GameFile_GetLength();
		char *data = xlNew char[len+1];
		if ( data )
		{
			ArchiveManager::GameFile_Read(data, len);
			ArchiveManager::GameFile_Close();

			pAnim->Load(data, len);
			xlDelete [] data;
		}
	}
}

int UI_System::UI_CreateLFD_Anim(string& sArchive, string& sAnim, string& sPal)
{
	//find a free slot.
	s32 ID = -1;
	for (s32 i=0; i<MAX_LFD_ANIM; i++)
	{
		if ( m_pLFD_Anim_List[i] == NULL )
		{
			ID = i;
			break;
		}
	}
	if ( ID > -1 )
	{
		m_pLFD_Anim_List[ID] = xlNew LFD_Anim( m_pDriver );
		Archive *pArchive = ArchiveManager::OpenArchive(ARCHIVETYPE_LFD, sArchive.c_str());
		if ( pArchive )
		{
			LoadLFD_Pal(  m_pLFD_Anim_List[ID], pArchive, sPal );
			LoadLFD_File( m_pLFD_Anim_List[ID], pArchive, sAnim );

			ArchiveManager::CloseArchive( pArchive );
		}
	}

	return ID;
}

void UI_System::UI_DestroyLFD_Anim(int ID)
{
	if ( ID >= 0 && ID < MAX_LFD_ANIM && m_pLFD_Anim_List[ID] )
	{
		xlDelete m_pLFD_Anim_List[ID];
	}
	m_pLFD_Anim_List[ID] = NULL;
}

void UI_System::UI_RenderLFD_Anim(int ID, int frame, int x, int y)
{
	if ( ID >= 0 && ID < MAX_LFD_ANIM && m_pLFD_Anim_List[ID] )
	{
		m_pDriver->SetBlendMode();
		m_pDriver->EnableAlphaTest(true);

		m_pLFD_Anim_List[ID]->Render(frame, (f32)x/320.0f, (f32)y/200.0f);
	}
}

int UI_System::UI_AddImage(string& sImage, int cutoutMinIdx, int cutoutMaxIdx)
{
	int hImageHandle = -1;
	if ( m_Context )
	{
		UI_RenderFrame *frame = AllocRenderFrame();
		//Load Frame texture.
		frame->hFrame = TextureCache::LoadTexture( sImage, false );

		//Then get the texture size and relative scale from the Texture Cache.
		s32 nOffsX, nOffsY;
		u32 uTexWidth, uTexHeight;
		f32 fRelSizeX, fRelSizeY;
		TextureCache::GetTextureSize( nOffsX, nOffsY, uTexWidth, uTexHeight, fRelSizeX, fRelSizeY );

		frame->width  = (s16)uTexWidth;
		frame->height = (s16)uTexHeight;
		frame->fRelWidth  = fRelSizeX;
		frame->fRelHeight = fRelSizeY;

		//Finally setup the image handle used by the script.
		hImageHandle = (int)frame->ID;
	}
	return hImageHandle;
}

int UI_System::UI_AddGameImage(u32 uImageType, string &sArchive, string &sImage)
{
	int hImageHandle = -1;
	if ( m_Context )
	{
		UI_RenderFrame *frame = AllocRenderFrame();
		//Load Frame texture.
		frame->hFrame = TextureCache::GameFile_LoadTexture( m_auImage_TexType[uImageType], 0, m_auImage_ArchiveType[uImageType], sArchive, sImage, false );
		
		//Then get the texture size and relative scale from the Texture Cache.
		s32 nOffsX, nOffsY;
		u32 uTexWidth, uTexHeight;
		f32 fRelSizeX, fRelSizeY;
		TextureCache::GetTextureSize( nOffsX, nOffsY, uTexWidth, uTexHeight, fRelSizeX, fRelSizeY );

		frame->width  = (s16)uTexWidth;
		frame->height = (s16)uTexHeight;
		frame->fRelWidth  = fRelSizeX;
		frame->fRelHeight = fRelSizeY;

		//Finally setup the image handle used by the script.
		hImageHandle = (int)frame->ID;
	}
	return hImageHandle;
}

void UI_System::UI_FreeImage(int hImageHandle)
{
	if ( hImageHandle > -1 )
	{
		FreeRenderFrame( (u32)hImageHandle );
	}
}

void UI_System::UI_SetVirtualScreenSize(int w, int h)
{
	m_Context->m_fVirt_Scr_W = (f32)w;
	m_Context->m_fVirt_Scr_H = (f32)h;
}

void UI_System::UI_GetVirtualScreenSize(int& w, int& h)
{
	w = (int)m_Context->m_fVirt_Scr_W;
	h = (int)m_Context->m_fVirt_Scr_H;
}

void UI_System::UI_GetScreenSize(int& w, int& h)
{
	m_pDriver->GetWindowSize(w, h);
}

void UI_System::UI_SetPalette(int pal, int colMap)
{
	m_pDriver->SetCurrentPalette(pal);
	m_pDriver->SetCurrentColormap(colMap);
}

//Solid color quad.
void UI_System::UI_RenderRect(int x, int y, int w, int h, float r, float g, float b, float a, int alignHoriz, int alignVert)
{
	if ( m_Context )
	{
		s32 nScrWidth, nScrHeight;

		m_pDriver->SetTexture(0, XL_INVALID_TEXTURE);
		m_pDriver->SetBlendMode( IDriver3D::BLEND_ALPHA );
		m_pDriver->GetWindowSize( nScrWidth, nScrHeight );

		float virt_scr_w = m_Context->m_fVirt_Scr_W;
		float virt_scr_h = m_Context->m_fVirt_Scr_H;
		float fUItoScrX = (f32)nScrWidth /virt_scr_w;
		float fUItoScrY = (f32)nScrHeight/virt_scr_h;

		//render rect...
		float fdX = (float)w * fUItoScrX;
		float fdY = (float)h * fUItoScrY;
		float fX0, fY0;
		//horizontal alignment.
		if ( alignHoriz == UI_Align_Left )
			fX0 = (float)x * fUItoScrX;
		else if ( alignHoriz == UI_Align_Right )
			fX0 = ( virt_scr_w + (float)(x - w) ) * fUItoScrX;
		else	//center
			fX0 = (float)( virt_scr_w*0.5f + (float)(x - (w>>1)) ) * fUItoScrX;
		//vertical alignment
		if ( alignVert == UI_Align_Bottom )
			fY0 = (float)y * fUItoScrY;
		else if (alignVert == UI_Align_Top )
			fY0 = (virt_scr_h + (float)(y - h)) * fUItoScrY;
		else //center
			fY0 = ( virt_scr_h + (float)(y - (h>>1)) ) * fUItoScrY;

		Vector4 color(r, g, b, a);
		Vector4 posScale(fX0, fY0, fdX, fdY);
		Vector2 uv(0,0);
		m_pDriver->SetColor( &color );
		m_pDriver->RenderScreenQuad(posScale, uv, uv, color, color);
	}
}

//Solid color polygon.
void UI_System::UI_RenderPolygon(int npt, asIScriptArray *x, asIScriptArray *y, float r, float g, float b, float a, int alignHoriz, int alignVert)
{
	if ( m_Context )
	{
#if 0
		Vector3 polygon[16];
		float u[16], v[16];

		Driver3D_DX9::SetShaders(Driver3D_DX9::VS_SHADER_SCREEN, Driver3D_DX9::PS_SHADER_SCREEN_NOTEX);
		Driver3D_DX9::EnableAlphaBlend(TRUE);

		float aspectScale = Driver3D_DX9::GetAspectScale();
		if ( Driver3D_DX9::Is320x200Enabled() )
		{
			aspectScale = 1.0f;
		}
		float virt_scr_w = m_Context->m_fVirt_Scr_W / aspectScale;
		float virt_scr_h = m_Context->m_fVirt_Scr_H;
		float fUItoScrX = 1.0f/virt_scr_w;
		float fUItoScrY = 1.0f/virt_scr_h;

		int min_x=100000, min_y=100000;
		int max_x = -min_x, max_y = -min_y;
		for (int i=0; i<npt; i++)
		{
			float fX, fY;
			int vx = *((int *)x->GetElementPointer(i));
			int vy = *((int *)y->GetElementPointer(i));

			if ( vx < min_x ) min_x = vx;
			if ( vy < min_y ) min_y = vy;

			if ( vx > max_x ) max_x = vx;
			if ( vy > max_y ) max_y = vy;
		}
		int w = max_x - min_x;
		int h = max_y - min_y;

		for (int i=0; i<npt; i++)
		{
			float fX, fY;
			int vx = *((int *)x->GetElementPointer(i));
			int vy = *((int *)y->GetElementPointer(i));

			//horizontal alignment.
			if ( alignHoriz == UI_Align_Left )
				fX = (float)vx * fUItoScrX;
			else if ( alignHoriz == UI_Align_Right )
				fX = ( virt_scr_w + (float)(vx - w) ) * fUItoScrX;
			else	//center
				fX = (float)( virt_scr_w*0.5f + (float)(vx - (w>>1)) ) * fUItoScrX;
			//vertical alignment
			if ( alignVert == UI_Align_Bottom )
				fY = (float)vy * fUItoScrY;
			else if (alignVert == UI_Align_Top )
				fY = (virt_scr_h + (float)(vy - h)) * fUItoScrY;
			else //center
				fY = ( virt_scr_h + (float)(vy - (h>>1)) ) * fUItoScrY;

			polygon[i].Set(fX, fY, 0.9f);
		}

		Driver3D_DX9::SetAmbientColor(r, g, b, a);
		Driver3D_DX9::RenderPolygon(npt, polygon, u, v);
#endif
	}
}

void UI_System::UI_GetImageSize(int hImage, int& w, int& h)
{
	UI_RenderFrame *frame = GetRenderFrame(hImage);
	w = (int)frame->width;
	h = (int)frame->height;
}

static bool m_bEnableFiltering = true;
static bool m_bEnableBlending  = true;
void UI_System::UI_EnableImageFilter(int enable)
{
	m_bEnableFiltering = (enable) ? true : false;
}

void UI_System::UI_EnableImageBlending(int enable)
{
	m_bEnableBlending = (enable) ? true : false;
}

void UI_System::UI_SetImageRenderProp(int flipX, int flipY)
{
	m_bFlipX = flipX ? true : false;
	m_bFlipY = flipY ? true : false;
}

//Image with alpha blending.
void UI_System::UI_RenderImage(int hImage, int x, int y, float intensity, int alignHoriz, int alignVert)
{
	if ( m_Context && hImage >= 0 )
	{
		UI_RenderFrame *frame = GetRenderFrame(hImage);
		s32 nScrWidth, nScrHeight;

		m_pDriver->SetTexture(0, frame->hFrame, m_bEnableFiltering ? IDriver3D::FILTER_NORMAL_NO_MIP : IDriver3D::FILTER_POINT);
		m_pDriver->SetBlendMode( m_bEnableBlending ? IDriver3D::BLEND_ALPHA : IDriver3D::BLEND_NONE );
		m_pDriver->GetWindowSize( nScrWidth, nScrHeight );

		float virt_scr_w = m_Context->m_fVirt_Scr_W;
		float virt_scr_h = m_Context->m_fVirt_Scr_H;
		float fUItoScrX = (f32)nScrWidth /virt_scr_w;
		float fUItoScrY = (f32)nScrHeight/virt_scr_h;

		s32 w = frame->width;
		s32 h = frame->height;
		//render rect...
		float fdX = (float)w * fUItoScrX;
		float fdY = (float)h * fUItoScrY;
		float fX0, fY0;
		//horizontal alignment.
		if ( alignHoriz == UI_Align_Left )
			fX0 = (float)x * fUItoScrX;
		else if ( alignHoriz == UI_Align_Right )
			fX0 = ( virt_scr_w + (float)(x - w) ) * fUItoScrX;
		else	//center
			fX0 = (float)( virt_scr_w*0.5f + (float)(x - (w>>1)) ) * fUItoScrX;
		//vertical alignment
		if ( alignVert == UI_Align_Bottom )
			fY0 = (float)y * fUItoScrY;
		else if (alignVert == UI_Align_Top )
			fY0 = (virt_scr_h + (float)(y - h)) * fUItoScrY;
		else //center
			fY0 = ( virt_scr_h + (float)(y - (h>>1)) ) * fUItoScrY;

		Vector4 color(intensity, intensity, intensity, 1.0f);
		Vector4 posScale(fX0, fY0, fdX, fdY);
		Vector2 uvTop(0,0), uvBot(1,1);

		if ( m_bFlipX )
		{
			uvTop.x = 1.0f;
			uvBot.x = 0.0f;
		}
		if ( m_bFlipY )
		{
			uvTop.y = 1.0f;
			uvBot.y = 0.0f;
		}

		m_pDriver->SetColor( &color );
		m_pDriver->RenderScreenQuad(posScale, uvTop, uvBot, color, color);
	}
}

void UI_System::UI_RenderString(string& sString, int x, int y, int size, float r, float g, float b, float a)
{
	XLFont *pFont = m_pEngine->GetSystemFont(size);
	if ( pFont )
	{
		s32 nScrWidth, nScrHeight;
		m_pDriver->GetWindowSize( nScrWidth, nScrHeight );

		f32 fUItoScrX = (f32)nScrWidth /m_Context->m_fVirt_Scr_W;
		f32 fUItoScrY = (f32)nScrHeight/m_Context->m_fVirt_Scr_H;

		x = (s32)( (f32)x * fUItoScrX );
		y = (s32)( (f32)y * fUItoScrX );

		FontManager::BeginTextRendering();
		{
			Vector4 vColor(r, g, b, a);
			FontManager::RenderString(x, y, sString, pFont, &vColor);
		}
		FontManager::EndTextRendering();
	}
}

void UI_System::UI_GetMousePos(int& x, int& y)
{
	s32 nScrWidth, nScrHeight;
	m_pDriver->GetWindowSize( nScrWidth, nScrHeight );

	s32 nMouseX = (u32)Input::GetMouseX();
	s32 nMouseY = (u32)Input::GetMouseY();

	f32 fScrToUI_X = m_Context->m_fVirt_Scr_W/(f32)nScrWidth;
	f32 fScrToUI_Y = m_Context->m_fVirt_Scr_H/(f32)nScrHeight;

	x = (s32)( (f32)nMouseX * fScrToUI_X );
	y = (s32)( (f32)nMouseY * fScrToUI_Y );
}

void UI_System::UI_PrintMousePos(int x, int y, int size)
{
	XLFont *pFont = m_pEngine->GetSystemFont(size);
	if ( pFont )
	{
		s32 nScrWidth, nScrHeight;
		m_pDriver->GetWindowSize( nScrWidth, nScrHeight );

		f32 fUItoScrX = (f32)nScrWidth /m_Context->m_fVirt_Scr_W;
		f32 fUItoScrY = (f32)nScrHeight/m_Context->m_fVirt_Scr_H;
	
		x = (s32)( (f32)x * fUItoScrX );
		y = (s32)( (f32)y * fUItoScrX );

		s32 nMouseX = (u32)Input::GetMouseX();
		s32 nMouseY = (u32)Input::GetMouseY();

		f32 fScrToUI_X = m_Context->m_fVirt_Scr_W/(f32)nScrWidth;
		f32 fScrToUI_Y = m_Context->m_fVirt_Scr_H/(f32)nScrHeight;

		nMouseX = (s32)( (f32)nMouseX * fScrToUI_X );
		nMouseY = (s32)( (f32)nMouseY * fScrToUI_Y );

		char szMousePos[64];
		sprintf(szMousePos, "mousePos: %d, %d", nMouseX, nMouseY);

		FontManager::BeginTextRendering();
		{
			FontManager::RenderString(x, y, szMousePos, pFont, &Vector4::One);
		}
		FontManager::EndTextRendering();
	}
}

f32 UI_System::UI_GetCurrentBrightness()
{
	if ( m_pEngine == NULL )
		return 1.0f;

	return m_pEngine->GetCurrentBrightness();
}

f32 UI_System::UI_GetSpeed()
{
	if ( m_pEngine == NULL )
		return 0.0f;

	return m_pEngine->GetCurrentSpeed();
}

//Image with alpha blending.
void UI_System::UI_SetImageUV_Range(f32 u0, f32 v0, f32 u1, f32 v1)
{
	m_uvTop.Set(u0, v0);
	m_uvBot.Set(u1, v1);
}

void UI_System::UI_SetImageUV_RangeI(int hImage, int u0, int v0, int w, int h)
{
	UI_RenderFrame *frame = GetRenderFrame(hImage);
	m_uvTop.x = (float)u0 / (float)frame->width;
	m_uvTop.y = (float)v0 / (float)frame->height;
	m_uvBot.x = (float)(u0+w) / (float)frame->width;
	m_uvBot.y = (float)(v0+h) / (float)frame->height;
}

void UI_System::UI_RenderImageRect(int hImage, int x, int y, int w, int h, float intensity, int alignHoriz, int alignVert)
{
	if ( m_Context && hImage >= 0 )
	{
		UI_RenderFrame *frame = GetRenderFrame(hImage);
		s32 nScrWidth, nScrHeight;

		m_pDriver->SetTexture(0, frame->hFrame, IDriver3D::FILTER_NORMAL_NO_MIP);
		m_pDriver->SetBlendMode( IDriver3D::BLEND_ALPHA );
		m_pDriver->GetWindowSize( nScrWidth, nScrHeight );

		float virt_scr_w = m_Context->m_fVirt_Scr_W;
		float virt_scr_h = m_Context->m_fVirt_Scr_H;
		float fUItoScrX = (f32)nScrWidth /virt_scr_w;
		float fUItoScrY = (f32)nScrHeight/virt_scr_h;

		//render rect...
		float fdX = (float)w * fUItoScrX;
		float fdY = (float)h * fUItoScrY;
		float fX0, fY0;
		//horizontal alignment.
		if ( alignHoriz == UI_Align_Left )
			fX0 = (float)x * fUItoScrX;
		else if ( alignHoriz == UI_Align_Right )
			fX0 = ( virt_scr_w + (float)(x - w) ) * fUItoScrX;
		else	//center
			fX0 = (float)( virt_scr_w*0.5f + (float)(x - (w>>1)) ) * fUItoScrX;
		//vertical alignment
		if ( alignVert == UI_Align_Bottom )
			fY0 = (float)y * fUItoScrY;
		else if (alignVert == UI_Align_Top )
			fY0 = (virt_scr_h + (float)(y - h)) * fUItoScrY;
		else //center
			fY0 = ( virt_scr_h + (float)(y - (h>>1)) ) * fUItoScrY;

		Vector4 color(intensity, intensity, intensity, 1.0f);
		Vector4 posScale(fX0, fY0, fdX, fdY);
		Vector2 uvTop(0,0), uvBot(1,1);

		if ( m_bFlipX )
		{
			uvTop.x = 1.0f;
			uvBot.x = 0.0f;
		}
		if ( m_bFlipY )
		{
			uvTop.y = 1.0f;
			uvBot.y = 0.0f;
		}

		m_pDriver->SetColor( &color );
		m_pDriver->RenderScreenQuad(posScale, m_uvTop, m_uvBot, color, color);
	}
}

int UI_System::UI_GetVirtualKey(int key)
{
	return Input::IsKeyDown(key);
}

void UI_System::KeyDownCallback(s32 key)
{
	if ( m_Context )	//this on doesn't actually need a context... but might as well be consistent.
	{
		if ( m_Context->m_hOnKey >= 0 )
		{
			ScriptArgument arg;
			arg.uType  = SC_ARG_U32;
			arg.argU32 = (u32)key;
			ScriptSystem::ExecuteFunc( m_Context->m_hOnKey, 1, &arg );
		}
	}
	//Built-in UI Editor.
	if ( key == XL_F11 )
	{
		UI_Screen *pUI_Editor = FindScreen("XL_UI_Editor");
		if ( pUI_Editor == NULL || m_Context != pUI_Editor )
		{
			UI_PushScreen(string("XL_UI_Editor"), UIFLAG_OVERLAY, 0);
		}
	}
}

UI_Screen *UI_System::FindScreen(const string& sName)
{
	UI_Screen *pScreen = NULL;
	map<string, UI_Screen *>::iterator iScreen = m_ScreenMap.find(sName);
	if ( iScreen != m_ScreenMap.end() )
	{
		pScreen = iScreen->second;
	}
	return pScreen;
}

/****** Basic UI_Screen *******/
UI_Screen::UI_Screen()
{
	m_hOnEnter	= -1;
	m_hOnExit	= -1;
	m_hOnRender	= -1;
	m_hOnPostRender = -1;
	m_hOnUpdate	= -1;
	m_hOnKey	= -1;

	m_uFlags = UI_System::UIFLAG_NONE;
	m_parent = NULL;
	m_child  = NULL;
	m_childWindow = NULL;
}

UI_Screen::~UI_Screen()
{
}

/****** Basic UI_Window ********/
bool UI_Window::s_bWindowMoving=false;

UI_Window::UI_Window()
{
	m_uType = 0;
	m_x = 0; m_y = 0;
	m_w = 0; m_h = 0;
	m_hOnRender  = -1;
	m_hOnUpdate  = -1;
	m_hOnKey     = -1;
	m_hOnRelease = -1;

	m_uFlags = UIWinFlag_None;
	m_uState = 0;
	m_bMouseHeld = false;
	m_bMoving = false;
	m_bEnabled = true;

	m_parentScreen = NULL;
	m_child		   = NULL;
	m_sibling	   = NULL;
	m_parent	   = NULL;
}

UI_Window::~UI_Window()
{
}

void UI_Window::Update(bool bMouseOver, int nMouseX, int nMouseY, int x, int y)
{
	m_uState = UI_State_Normal;
	if ( m_hOnUpdate>=0 )	//custom script update function.
	{
		m_uState = ScriptSystem::ExecuteFunc( m_hOnUpdate, 0, NULL, true );
	}
	else if ( bMouseOver )
	{
		m_uState = UI_State_MouseOver;
		if ( Input::IsKeyDown(XL_LBUTTON) )
		{
			m_bMouseHeld = true;
			m_uState = UI_State_Pressed;
		}
		else
		{
			if ( m_bMouseHeld == true )
			{
				if ( m_hOnRelease >= 0 )
				{
					ScriptSystem::ExecuteFunc( m_hOnRelease );
				}
				else
				{
					//Release code...
				}
			}
			m_bMouseHeld = false;
		}
	}
	else
	{
		m_bMouseHeld = false;
	}

	if ( Input::IsKeyDown(XL_LBUTTON) )
	{
		if ( m_uType == 0 && m_hOnRender < 0 && m_bMoving == false && s_bWindowMoving == false )
		{
			if ( nMouseX >= x+m_x && nMouseX <= x+m_x+m_w && nMouseY >= y+m_y && nMouseY < y+m_y+40 )
			{
				m_bMoving = true;
				s_bWindowMoving = true;
				m_prevX = nMouseX;
				m_prevY = nMouseY;
			}
		}
		else if ( m_bMoving )
		{
			m_x += (nMouseX-m_prevX);
			m_y += (nMouseY-m_prevY);

			m_prevX = nMouseX;
			m_prevY = nMouseY;
		}
	}
	else
	{
		s_bWindowMoving = false;
		m_bMoving = false;
	}
}

void UI_Window::Draw(int x, int y)
{
	if ( m_hOnRender>=0 )	//custom script draw function.
	{
		ScriptArgument arg[3];
		arg[0].uType  = SC_ARG_U32;
		arg[0].argU32 = m_uState;
		arg[1].uType  = SC_ARG_U32;
		arg[1].argU32 = m_x+x;
		arg[2].uType  = SC_ARG_U32;
		arg[2].argU32 = m_y+y;
		ScriptSystem::ExecuteFunc( m_hOnRender, 3, arg );
	}
	else	//basic window drawing.
	{
		if ( m_uType == 0 )	//window
		{
			RenderWindow(m_x+x, m_y+y, m_w, m_h, m_text);
		}
		else if ( m_uType == 1 ) //button
		{
			RenderButton(m_x+x, m_y+y, m_w, m_text, m_uState);
		}
	}
}

////////////////////////////////////////////////////////
// Default UI Rendering
////////////////////////////////////////////////////////
void UI_Window::RenderWindow(int x, int y, int w, int h, const string& name)
{
	if ( w < 28 ) w = 28;
	if ( h < 69 ) h = 69;

	//Render title bar.
	int xx = x;
	
	UI_System::UI_SetImageUV_RangeI(Editor_UI_Atlas, 0, 56, 10, 41);
	UI_System::UI_RenderImageRect(Editor_UI_Atlas, xx, y,   10, 41, 1.0f, UI_System::UI_Align_Left, UI_System::UI_Align_Bottom);
	xx += 10;
	
	UI_System::UI_SetImageUV_RangeI(Editor_UI_Atlas, 8, 56, 1, 41);
	UI_System::UI_RenderImageRect(Editor_UI_Atlas, xx, y, w-20, 41, 1.0f, UI_System::UI_Align_Left, UI_System::UI_Align_Bottom);
	xx += (w-20);
	
	UI_System::UI_SetImageUV_RangeI(Editor_UI_Atlas, 10, 56, 10, 41);
	UI_System::UI_RenderImageRect(Editor_UI_Atlas, xx, y,   10, 41, 1.0f, UI_System::UI_Align_Left, UI_System::UI_Align_Bottom);
	xx += 10;
	
	//Draw title bar text.
	UI_System::UI_RenderString((string)name, x+25, y+9, 24, 0.0f, 0.0f, 0.0f, 1.0f);
	UI_System::UI_RenderString((string)name, x+24, y+8, 24, 0.8f, 0.8f, 0.8f, 1.0f);
	
	//Render the window background.
	int yy = y+41;
	xx = x;
	UI_System::UI_SetImageUV_RangeI(Editor_UI_Atlas, 20, 56, 14, 14);
	UI_System::UI_RenderImageRect(Editor_UI_Atlas, xx, yy, 14, 14, 1.0f, UI_System::UI_Align_Left, UI_System::UI_Align_Bottom);
	xx += 14;
	
	UI_System::UI_SetImageUV_RangeI(Editor_UI_Atlas, 34, 56, 1, 14);
	UI_System::UI_RenderImageRect(Editor_UI_Atlas, xx, yy, w-28, 14, 1.0f, UI_System::UI_Align_Left, UI_System::UI_Align_Bottom);
	xx += (w-28);
	
	UI_System::UI_SetImageUV_RangeI(Editor_UI_Atlas, 34, 56, 14, 14);
	UI_System::UI_RenderImageRect(Editor_UI_Atlas, xx, yy, 14, 14, 1.0f, UI_System::UI_Align_Left, UI_System::UI_Align_Bottom);
	
	xx = x+14; yy += 14;
	UI_System::UI_SetImageUV_RangeI(Editor_UI_Atlas, 34, 60, 1, 1);
	UI_System::UI_RenderImageRect(Editor_UI_Atlas, xx, yy, (w-28), (h-69), 1.0f, UI_System::UI_Align_Left, UI_System::UI_Align_Bottom);
	
	yy = y+h-14;
	xx = x;
	UI_System::UI_SetImageUV_RangeI(Editor_UI_Atlas, 20, 70, 14, 14);
	UI_System::UI_RenderImageRect(Editor_UI_Atlas, xx, yy, 14, 14, 1.0f, UI_System::UI_Align_Left, UI_System::UI_Align_Bottom);
	xx += 14;
	
	UI_System::UI_SetImageUV_RangeI(Editor_UI_Atlas, 28, 70, 1, 14);
	UI_System::UI_RenderImageRect(Editor_UI_Atlas, xx, yy, w-28, 14, 1.0f, UI_System::UI_Align_Left, UI_System::UI_Align_Bottom);
	xx += (w-28);
	
	UI_System::UI_SetImageUV_RangeI(Editor_UI_Atlas, 34, 70, 14, 14);
	UI_System::UI_RenderImageRect(Editor_UI_Atlas, xx, yy, 14, 14, 1.0f, UI_System::UI_Align_Left, UI_System::UI_Align_Bottom);
	xx += 14;
	
	xx = x;
	yy = y+55;
	UI_System::UI_SetImageUV_RangeI(Editor_UI_Atlas, 20, 60, 14, 1);
	UI_System::UI_RenderImageRect(Editor_UI_Atlas, xx, yy, 14, (h-69), 1.0f, UI_System::UI_Align_Left, UI_System::UI_Align_Bottom);
	
	xx = x+w-14;
	yy = y+55;
	UI_System::UI_SetImageUV_RangeI(Editor_UI_Atlas, 34, 60, 14, 1);
	UI_System::UI_RenderImageRect(Editor_UI_Atlas, xx, yy, 14, (h-69), 1.0f, UI_System::UI_Align_Left, UI_System::UI_Align_Bottom);
}

void UI_Window::RenderButton(int x, int y, int w, const string& text, int state)
{
	UI_System::UI_EnableImageFilter(0);
	
	if ( state == 0 )	//normal
	{
		UI_System::UI_SetImageUV_RangeI(Editor_UI_Atlas, 0, 0, 10, 28);
		UI_System::UI_RenderImageRect(Editor_UI_Atlas, x, y, 10, 28, 1.0f, UI_System::UI_Align_Left, UI_System::UI_Align_Bottom);
		
		UI_System::UI_SetImageUV_RangeI(Editor_UI_Atlas, 10, 0, 1, 28);
		UI_System::UI_RenderImageRect(Editor_UI_Atlas, x+10, y, (w-20), 28, 1.0f, UI_System::UI_Align_Left, UI_System::UI_Align_Bottom);
		
		UI_System::UI_SetImageUV_RangeI(Editor_UI_Atlas, 10, 0, 10, 28);
		UI_System::UI_RenderImageRect(Editor_UI_Atlas, x+w-10, y, 10, 28, 1.0f, UI_System::UI_Align_Left, UI_System::UI_Align_Bottom);
		
		//Draw title bar text.
		UI_System::UI_RenderString((string)text, x+15, y+6, 16, 0.0f, 0.0f, 0.0f, 1.0f);
		UI_System::UI_RenderString((string)text, x+14, y+5, 16, 0.8f, 0.8f, 0.8f, 1.0f);
	}
	else if ( state == 1 )	//pressed.
	{
		UI_System::UI_SetImageUV_RangeI(Editor_UI_Atlas, 20, 0, 10, 28);
		UI_System::UI_RenderImageRect(Editor_UI_Atlas, x, y, 10, 28, 1.0f, UI_System::UI_Align_Left, UI_System::UI_Align_Bottom);
		
		UI_System::UI_SetImageUV_RangeI(Editor_UI_Atlas, 30, 0, 1, 28);
		UI_System::UI_RenderImageRect(Editor_UI_Atlas, x+10, y, (w-20), 28, 1.0f, UI_System::UI_Align_Left, UI_System::UI_Align_Bottom);
		
		UI_System::UI_SetImageUV_RangeI(Editor_UI_Atlas, 30, 0, 10, 28);
		UI_System::UI_RenderImageRect(Editor_UI_Atlas, x+w-10, y, 10, 28, 1.0f, UI_System::UI_Align_Left, UI_System::UI_Align_Bottom);
		
		//Draw title bar text.
		UI_System::UI_RenderString((string)text, x+15, y+6, 16, 0.0f, 0.0f, 0.0f, 1.0f);
		UI_System::UI_RenderString((string)text, x+14, y+5, 16, 0.6f, 0.6f, 0.6f, 1.0f);
	}
	else	//mouse over.
	{
		UI_System::UI_SetImageUV_RangeI(Editor_UI_Atlas, 0, 0, 10, 28);
		UI_System::UI_RenderImageRect(Editor_UI_Atlas, x, y, 10, 28, 1.0f, UI_System::UI_Align_Left, UI_System::UI_Align_Bottom);
		
		UI_System::UI_SetImageUV_RangeI(Editor_UI_Atlas, 10, 0, 1, 28);
		UI_System::UI_RenderImageRect(Editor_UI_Atlas, x+10, y, (w-20), 28, 1.0f, UI_System::UI_Align_Left, UI_System::UI_Align_Bottom);
		
		UI_System::UI_SetImageUV_RangeI(Editor_UI_Atlas, 10, 0, 10, 28);
		UI_System::UI_RenderImageRect(Editor_UI_Atlas, x+w-10, y, 10, 28, 1.0f, UI_System::UI_Align_Left, UI_System::UI_Align_Bottom);
		
		//Draw title bar text.
		UI_System::UI_RenderString((string)text, x+15, y+6, 16, 0.0f, 0.0f, 0.0f, 1.0f);
		UI_System::UI_RenderString((string)text, x+14, y+5, 16, 1.0f, 1.0f, 0.8f, 1.0f);
	}
	
	UI_System::UI_EnableImageFilter(1);
}
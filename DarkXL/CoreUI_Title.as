////////////////////////
/// DXL_Title Screen ///
////////////////////////

int DXL_Title_Img0, DXL_Title_Img1;
int DXL_StartBtn0, DXL_StartBtn1;
int DXL_SettingsBtn0, DXL_SettingsBtn1;
int DXL_ExitBtn0, DXL_ExitBtn1;
int scrW, scrH;
int virtW, virtH;
int offset, extOffs;
float scale;

void UI_Title_OnEnter()
{
	//Set the virtual screen size.
	UI_GetScreenSize(scrW, scrH);
	virtW = 768*scrW/scrH;
	virtH = 768;
	UI_SetVirtualScreenSize(virtW, virtH);
	
	//handle widescreen resolutions.
	offset  = (virtW - 1024) / 2;
	extOffs = 0;
	scale  = 1.0f;
	if ( offset > 64 ) 
	{
		offset = 64;
		if ( virtW > 1152 )
		{
			scale = virtW/1152.0f;
			extOffs = -4;
		}
	}

	//Load the background.
	DXL_Title_Img0 = UI_AddImage("DarkXL_Title1.png", 0, 0);
	DXL_Title_Img1 = UI_AddImage("DarkXL_Title2.png", 0, 0);
	//Buttons, each with 2 states.
	DXL_StartBtn0  = UI_AddImage("DXL_LaunchBtn0.png", 0, 0);
	DXL_StartBtn1  = UI_AddImage("DXL_LaunchBtn1.png", 0, 0);
	DXL_SettingsBtn0 = UI_AddImage("DXL_SettingsBtn0.png", 0, 0);
	DXL_SettingsBtn1 = UI_AddImage("DXL_SettingsBtn1.png", 0, 0);
	DXL_ExitBtn0  = UI_AddImage("DXL_ExitBtn0.png", 0, 0);
	DXL_ExitBtn1  = UI_AddImage("DXL_ExitBtn1.png", 0, 0);
	
	//Start music playback.

	//buttons	
	UI_CreateWindow("UIT_StartBtn", "", 0, int((196+offset)*scale), 652, int(204*scale), 66, UIWinFlag_None);
	UI_CreateWindow("UIT_SettingsBtn", "", 0, int((409+offset)*scale), 652, int(204*scale), 66, UIWinFlag_None);
	UI_CreateWindow("UIT_ExitBtn", "", 0, int((622+offset)*scale), 652, int(204*scale), 66, UIWinFlag_None);
}

void UI_Title_OnExit()
{
	//Unload background and other unique graphics.
	UI_FreeImage( DXL_Title_Img0 );
	UI_FreeImage( DXL_Title_Img1 );
	
	UI_FreeImage( DXL_StartBtn0 );
	UI_FreeImage( DXL_StartBtn1 );
	UI_FreeImage( DXL_SettingsBtn0 );
	UI_FreeImage( DXL_SettingsBtn1 );
	UI_FreeImage( DXL_ExitBtn0 );
	UI_FreeImage( DXL_ExitBtn1 );
	
	//Stop music playback.
	
}

void UI_Title_OnRender(int state)
{
	//Render the background.
	UI_RenderImageRect(DXL_Title_Img0,  int((-64+offset)*scale), 0, int(768*scale), 820, 1.0, UI_Align_Left,  UI_Align_Bottom);
	UI_RenderImageRect(DXL_Title_Img1,  int((64-offset)*scale+extOffs), 0, int(384*scale), 820, 1.0, UI_Align_Right, UI_Align_Bottom);
	
	//Render version text.
	UI_RenderString("Alpha Build 9.50", int((850+offset)*scale), 740, 16, 0.8f, 0.8f, 0.8f, 1.0f);
}

void UI_Title_OnUpdate()
{
	//Mouse over eventually?
}

void UI_Title_OnKey(int key, int msg)
{
	//maybe ENTER to start, ESC for options.
}

//Start button
void UIT_StartBtn_OnRender(int state)
{
	UI_SetImageUV_Range(0.0, 0.0, 0.53125, 0.640625);
	if ( state == 1 )
	{
		UI_RenderImageRect(DXL_StartBtn1, int((196+offset)*scale), 652, int(204*scale), 66, 1.0, UI_Align_Left, UI_Align_Bottom);
	}
	else
	{
		UI_RenderImageRect(DXL_StartBtn0, int((196+offset)*scale), 652, int(204*scale), 66, 1.0, UI_Align_Left, UI_Align_Bottom);
	}
	UI_SetImageUV_Range(0.0, 0.0, 1.0, 1.0);
}

void UIT_StartBtn_OnRelease()
{
	UI_StartScreen("UI_LevelSelect");
}

//Settings button
void UIT_SettingsBtn_OnRender(int state)
{
	UI_SetImageUV_Range(0.0, 0.0, 0.53125, 0.640625);
	if ( state == 1 )
	{
		UI_RenderImageRect(DXL_SettingsBtn1, int((409+offset)*scale), 652, int(204*scale), 66, 1.0, UI_Align_Left, UI_Align_Bottom);
	}
	else
	{
		UI_RenderImageRect(DXL_SettingsBtn0, int((409+offset)*scale), 652, int(204*scale), 66, 1.0, UI_Align_Left, UI_Align_Bottom);
	}
	UI_SetImageUV_Range(0.0, 0.0, 1.0, 1.0);
}

void UIT_SettingsBtn_OnRelease()
{
	UI_PushScreen( "UI_Config", UIFLAG_NONE, UI_BCKGRND_FX_NONE );
}

//Exit button
void UIT_ExitBtn_OnRender(int state)
{
	UI_SetImageUV_Range(0.0, 0.0, 0.53125, 0.640625);
	if ( state == 1 )
	{
		UI_RenderImageRect(DXL_ExitBtn1, int((622+offset)*scale), 652, int(204*scale), 66, 1.0, UI_Align_Left, UI_Align_Bottom);
	}
	else
	{
		UI_RenderImageRect(DXL_ExitBtn0, int((622+offset)*scale), 652, int(204*scale), 66, 1.0, UI_Align_Left, UI_Align_Bottom);
	}
	UI_SetImageUV_Range(0.0, 0.0, 1.0, 1.0);
}

void UIT_ExitBtn_OnRelease()
{
	Game_Exit();
}

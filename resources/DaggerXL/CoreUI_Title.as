////////////////////////
/// DXL_Title Screen ///
////////////////////////

int UIT_scrW, UIT_scrH;
int UIT_virtW, UIT_virtH;
int UIT_offset;
float UIT_scale;

int UIT_Backgrnd;

void UI_Title_OnEnter()
{
	//Set the virtual screen size.
	UI_GetScreenSize(UIT_scrW, UIT_scrH);
	UIT_virtW = 600*UIT_scrW/UIT_scrH;
	UIT_virtH = 600;
	UI_SetVirtualScreenSize(UIT_virtW, UIT_virtH);
	
	//handle widescreen resolutions.
	UIT_offset  = (UIT_virtW - 800) / 2;
	
	//load background.
	UIT_Backgrnd = UI_AddImage("TitleScreen.png", 0, 0);
}

void UI_Title_OnExit()
{
	UI_FreeImage( UIT_Backgrnd );
	UI_SetImageRenderProp( 0, 0 );
}

void UI_Title_OnRender(int state)
{
	UI_EnableImageFilter(1);
	//Render the background.
	UI_RenderImage(UIT_Backgrnd, UIT_offset, 0, 1.0f, UI_Align_Left, UI_Align_Bottom);
	
	UI_RenderString("ALPHA - Version 0.25", 661+UIT_offset, 571, 16, 0.0f, 0.0f, 0.0f, 1.0f);
	UI_RenderString("ALPHA - Version 0.25", 660+UIT_offset, 570, 16, 0.8f, 0.8f, 0.8f, 1.0f);
	
	UI_RenderString("Press Start to Begin", 301+UIT_offset, 533, 32, 0.1f, 0.1f, 0.0f, 1.0f);
	UI_RenderString("Press Start to Begin", 300+UIT_offset, 532, 32, 0.8f, 0.8f, 0.4f, 1.0f);
}

void UI_Title_OnUpdate()
{
}

void UI_Title_OnKey(int key)
{
	if ( key == KEY_ENTER )
	{
		UI_StartScreen("UI_StartMenu");
	}
}

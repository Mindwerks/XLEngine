////////////////////////
/// OXL_Title Screen ///
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
	UIT_virtW = 480*UIT_scrW/UIT_scrH;
	UIT_virtH = 480;
	UI_SetVirtualScreenSize(UIT_virtW, UIT_virtH);
	
	//handle widescreen resolutions.
	UIT_offset  = (UIT_virtW - 640) / 2;
	
	//load background.
	UIT_Backgrnd = UI_AddGameImage(IMAGE_TYPE_PCX, "OUTLAWS.LAB", "MM220.PCX");
}

void UI_Title_OnExit()
{
	//restore mouse lock state.
	Input_EnableMouseLocking(1);
	
	UI_FreeImage( UIT_Backgrnd );
	UI_SetImageRenderProp( 0, 0 );
}

void UI_Title_OnRender(int state)
{
	UI_EnableImageFilter(0);
	//Render the background.
	UI_RenderImage(UIT_Backgrnd, UIT_offset, 0, 1.0f, UI_Align_Left, UI_Align_Bottom);
	
	UI_RenderString("Pre-Alpha Build 0.01", 500+UIT_offset, 460, 16, 0.8f, 0.8f, 0.8f, 1.0f);
}

void UI_Title_OnUpdate()
{
}

void UI_Title_OnKey(int key)
{
	if ( key == KEY_ENTER )
	{
		Game_NewGame( 0, 0 );
		UI_StartScreen("UI_Game");
	}
	else if ( key == KEY_ESC )
	{
		Game_Exit();
	}
}

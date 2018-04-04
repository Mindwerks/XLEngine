////////////////////////
///   DXL_StartMenu  ///
////////////////////////

int UIS_scrW, UIS_scrH;
int UIS_virtW, UIS_virtH;
int UIS_offset;
float UIS_scale;

int UIS_Backgrnd;
int UIS_MouseCursor;

void UI_StartMenu_OnEnter()
{
	//Set the virtual screen size.
	UI_GetScreenSize(UIS_scrW, UIS_scrH);
	UIS_virtW = 240*UIS_scrW/UIS_scrH;
	UIS_virtH = 200;
	UI_SetVirtualScreenSize(UIS_virtW, UIS_virtH);
	
	//handle widescreen resolutions.
	UIS_offset  = (UIS_virtW - 320) / 2;
	
	//load background.
	UIS_Backgrnd = UI_AddGameImage(IMAGE_TYPE_IMG, "", "PICK03I0.IMG");
	//load the mouse cursor.
	UIS_MouseCursor = UI_AddImage("Cursor.png", 0, 0);
	
	UI_SetPalette(31, 0);
}

void UI_StartMenu_OnExit()
{
	//restore mouse lock state.
	Input_EnableMouseLocking(1);
	
	UI_FreeImage( UIS_Backgrnd );
	UI_FreeImage( UIS_MouseCursor );
	UI_SetImageRenderProp( 0, 0 );
}

void UI_StartMenu_OnRender(int state)
{
	UI_EnableImageFilter(0);
	UI_EnableImageBlending(0);
	//Render the background.
	UI_RenderImage(UIS_Backgrnd, UIS_offset, 0, 1.0f, UI_Align_Left, UI_Align_Bottom);
	//Restore.
	UI_EnableImageBlending(1);

	//Get the current mouse position.	
	int mouse_x, mouse_y;
	UI_GetMousePos(mouse_x, mouse_y);
	
	//Draw the Mouse.
	UI_EnableImageBlending(1);
	UI_RenderImage(UIS_MouseCursor, mouse_x, mouse_y, 1.0f, UI_Align_Left, UI_Align_Bottom);
}

void UI_StartMenu_OnUpdate()
{
	int mouse_x, mouse_y;
	UI_GetMousePos(mouse_x, mouse_y);
	mouse_x -= UIS_offset;
}

void UI_StartMenu_OnKey(int key)
{
	if ( key == KEY_E )
	{
		//Exit.
	}
	else if ( key == KEY_L )
	{
		//Load Game.
	}
	else if ( key == KEY_S )
	{
		//Start New Game.
	}
	Game_NewGame();
	UI_StartScreen("UI_GameView");
}

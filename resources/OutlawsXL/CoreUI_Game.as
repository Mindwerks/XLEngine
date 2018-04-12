////////////////////////
/// BXL_Game Screen ////
////////////////////////

int UIG_scrW,  UIG_scrH;
int UIG_virtW, UIG_virtH;
int UIG_offset;
float UIG_scale;

//int[] UIG_HUD(2);
//int[] UIG_FONT_RED(10);

void UI_Game_OnEnter()
{
	//Enable mouse locking for mouse look.
	Input_EnableMouseLocking(1);
	//Set the virtual screen size.
	UI_GetScreenSize(UIG_scrW, UIG_scrH);
	UIG_virtW = 480*UIG_scrW/UIG_scrH;
	UIG_virtH = 480;
	UI_SetVirtualScreenSize(UIG_virtW, UIG_virtH);
	
	//handle widescreen resolutions.
	UIG_offset  = (UIG_virtW - 640) / 2;
	
	//load HUD.
	//string num = 2201;
	//UIG_HUD[0] = UI_AddGameImage(IMAGE_TYPE_ART, "TILES008.ART", num);
	
	//num = 2173;
	//UIG_HUD[1] = UI_AddGameImage(IMAGE_TYPE_ART, "TILES008.ART", num);
	
	/*for (int i=0; i<10; i++)
	{
		num = (2190+i);
		UIG_FONT_RED[i] = UI_AddGameImage(IMAGE_TYPE_ART, "TILES008.ART", num);
	}*/
}

void UI_Game_OnExit()
{
	//UI_FreeImage( UIG_HUD[0] );
	//UI_FreeImage( UIG_HUD[1] );
	
	/*for (int i=0; i<10; i++)
	{
		UI_FreeImage( UIG_FONT_RED[i] );
	}*/
	
	UI_SetImageRenderProp( 0, 0 );
	
	//disable mouse lock.
	Input_EnableMouseLocking(0);
}

void UI_Game_OnRender(int state)
{
	UI_EnableImageFilter(0);
	
	//UI_RenderImage(UIG_HUD[0], 0, 0, 1.0f, UI_Align_Left,  UI_Align_Top);
	//UI_RenderImage(UIG_HUD[1], 0, 0, 1.0f, UI_Align_Right, UI_Align_Top);
	
	/*
	int x = 4, w, h;
	UI_RenderImage(UIG_FONT_RED[1], x, -11, 1.0f, UI_Align_Left,  UI_Align_Top);
	UI_GetImageSize(UIG_FONT_RED[1], w, h); 
	x+=w+1;
	UI_RenderImage(UIG_FONT_RED[0], x, -11, 1.0f, UI_Align_Left,  UI_Align_Top);
	UI_GetImageSize(UIG_FONT_RED[0], w, h); 
	x+=w+1;
	UI_RenderImage(UIG_FONT_RED[0], x, -11, 1.0f, UI_Align_Left,  UI_Align_Top);
	*/
	
	UI_EnableImageFilter(1);
}

void UI_Game_OnUpdate()
{
}

void UI_Game_OnKey(int key)
{
	if ( key == KEY_ESC )
	{
		Game_Exit();
	}
}

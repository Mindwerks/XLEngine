////////////////////////
/// BXL_Game Screen ////
////////////////////////

int UIG_scrW,  UIG_scrH;
int UIG_virtW, UIG_virtH;
int UIG_offset;
float UIG_scale;

int[] UIG_HUD(2);
int[] UIG_FONT_RED(10);
int[] UIG_Pickfork(3);
int[] forkFrameList(14);

int forkFrame;
int forkAnim;
int forkDelay;

void UI_Game_OnEnter()
{
	//Enable mouse locking for mouse look.
	Input_EnableMouseLocking(1);
	//Set the virtual screen size.
	UI_GetScreenSize(UIG_scrW, UIG_scrH);
	UIG_virtW = 240*UIG_scrW/UIG_scrH;
	UIG_virtH = 200;
	UI_SetVirtualScreenSize(UIG_virtW, UIG_virtH);
	
	//handle widescreen resolutions.
	UIG_offset  = (UIG_virtW - 320) / 2;
	
	//load HUD.
	string num = 2201;
	UIG_HUD[0] = UI_AddGameImage(IMAGE_TYPE_ART, "TILES008.ART", num);
	
	num = 2173;
	UIG_HUD[1] = UI_AddGameImage(IMAGE_TYPE_ART, "TILES008.ART", num);
	
	for (int i=0; i<10; i++)
	{
		num = (2190+i);
		UIG_FONT_RED[i] = UI_AddGameImage(IMAGE_TYPE_ART, "TILES008.ART", num);
	}
	
	num = 3222;
	UIG_Pickfork[0] = UI_AddGameImage(IMAGE_TYPE_ART, "TILES012.ART", num);
	num = 3223;
	UIG_Pickfork[1] = UI_AddGameImage(IMAGE_TYPE_ART, "TILES012.ART", num);
	num = 3224;
	UIG_Pickfork[2] = UI_AddGameImage(IMAGE_TYPE_ART, "TILES012.ART", num);
	
	forkFrame = 0;
	forkAnim = 0;
	forkDelay = 0;
	
	//fill in fork anim
	forkFrameList[0] = 0;
	forkFrameList[1] = 0;
	forkFrameList[2] = 1;
	forkFrameList[3] = 1;
	forkFrameList[4] = 2;
	forkFrameList[5] = 2;
	forkFrameList[6] = 2;
	forkFrameList[7] = 2;
	forkFrameList[8] = 2;
	forkFrameList[9] = 2;
	forkFrameList[10] = 1;
	forkFrameList[11] = 1;
	forkFrameList[12] = 1;
	forkFrameList[13] = 1;
}

void UI_Game_OnExit()
{
	UI_FreeImage( UIG_HUD[0] );
	UI_FreeImage( UIG_HUD[1] );
	UI_FreeImage( UIG_Pickfork[0] );
	UI_FreeImage( UIG_Pickfork[1] );
	UI_FreeImage( UIG_Pickfork[2] );
	
	for (int i=0; i<10; i++)
	{
		UI_FreeImage( UIG_FONT_RED[i] );
	}
	
	UI_SetImageRenderProp( 0, 0 );
	
	//disable mouse lock.
	Input_EnableMouseLocking(0);
}

float time_sum=0.0f;

void UI_Game_OnRender(int state)
{
	UI_EnableImageFilter(0);
	
	//for now render the weapon, this will happen later in the proper weapon scripts.
	int frame = forkFrameList[ forkFrame ];
	
	//now compute a bobbing bias...
	time_sum = math_modf(time_sum+4.0f/60.0f, 6.28318530718f);
	float wxAnim = math_cosf( time_sum ) * UI_GetSpeed() * 0.075f;
	float wyAnim = math_absf( math_sinf( time_sum ) * UI_GetSpeed() * 0.075f );
	int dx = wxAnim * 320; 
	int dy = wyAnim * 200;
		
	UI_RenderImage(UIG_Pickfork[frame], UIG_offset+123+dx, 34+dy, UI_GetBrightness(), UI_Align_Left,  UI_Align_Top);
	
	UI_RenderImage(UIG_HUD[0], 0, 0, 1.0f, UI_Align_Left,  UI_Align_Top);
	UI_RenderImage(UIG_HUD[1], 0, 0, 1.0f, UI_Align_Right, UI_Align_Top);
	
	int x = 4, w, h;
	UI_RenderImage(UIG_FONT_RED[1], x, -11, 1.0f, UI_Align_Left,  UI_Align_Top);
	UI_GetImageSize(UIG_FONT_RED[1], w, h); 
	x+=w+1;
	UI_RenderImage(UIG_FONT_RED[0], x, -11, 1.0f, UI_Align_Left,  UI_Align_Top);
	UI_GetImageSize(UIG_FONT_RED[0], w, h); 
	x+=w+1;
	UI_RenderImage(UIG_FONT_RED[0], x, -11, 1.0f, UI_Align_Left,  UI_Align_Top);
	
	UI_EnableImageFilter(1);
}

void UI_Game_OnUpdate()
{
	if ( forkAnim != 0 )
	{
		forkFrame++;
		if ( forkFrame >= 14 )
		{
			forkFrame = 0;
			forkAnim = 0;
		}
	}
	else if ( UI_IsKeyDown( KEY_ATTACK ) == 1 && forkDelay == 0 )
	{
		forkFrame = 0;
		forkAnim = 1;
		forkDelay = 24;
	}
	if ( forkDelay > 0 ) forkDelay--;
}

void UI_Game_OnKey(int key)
{
	if ( key == KEY_ESC )
	{
		UI_PushScreen( "UI_Title", UIFLAG_OVERLAY, UI_BCKGRND_FX_NONE );
	}
}

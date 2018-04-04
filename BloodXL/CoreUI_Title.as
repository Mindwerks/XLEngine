////////////////////////
/// BXL_Title Screen ///
////////////////////////

int UIT_scrW, UIT_scrH;
int UIT_virtW, UIT_virtH;
int UIT_offset;
float UIT_scale;

int[] UIT_Font(26);
int[] UIT_Blood(8);
int UIT_Box;
int[] UIT_Help(3);
int[] UIT_Credits(2);

int bloodAnim = 0;
float selAnim = 0.5f;
int selection = 0;
int UIT_State = 0;
int UIT_Episode = 0;
int UIT_Difficulty = 0;
int UIT_RanOnce = 0;

int UIT_mouseLockPrev;

void UI_Title_OnEnter()
{
	bloodAnim = 0;
	selAnim = 0.5f;
	selection = 0;
	UIT_State = 0;
	UIT_Episode = 0;
	UIT_Difficulty = 0;

	//Set the virtual screen size.
	UI_GetScreenSize(UIT_scrW, UIT_scrH);
	UIT_virtW = 240*UIT_scrW/UIT_scrH;
	UIT_virtH = 200;
	UI_SetVirtualScreenSize(UIT_virtW, UIT_virtH);
	
	//handle widescreen resolutions.
	UIT_offset  = (UIT_virtW - 320) / 2;
	
	//load font.
	for (int i=0; i<26; i++)
	{
		int tilenum = 4225 + i;
		int archiveNum = (tilenum/256);
		string archive = "TILES0" + archiveNum + ".ART";
		string num = tilenum;
		
		UIT_Font[i] = UI_AddGameImage(IMAGE_TYPE_ART, archive, num);
	}
	
	//load blood.
	for (int i=0; i<8; i++)
	{
		int tilenum = 2030 + i;
		int archiveNum = (tilenum/256);
		string archive = "TILES00" + archiveNum + ".ART";
		string num = tilenum;
		
		UIT_Blood[i] = UI_AddGameImage(IMAGE_TYPE_ART, archive, num);
	}
	
	//load box.
	string num = 2038;
	UIT_Box = UI_AddGameImage(IMAGE_TYPE_ART, "TILES007.ART", num);
	
	num = 2570;
	UIT_Help[0] = UI_AddGameImage(IMAGE_TYPE_ART, "TILES010.ART", num);
	
	num = 2571;
	UIT_Help[1] = UI_AddGameImage(IMAGE_TYPE_ART, "TILES010.ART", num);
	
	num = 2593;
	UIT_Help[2] = UI_AddGameImage(IMAGE_TYPE_ART, "TILES010.ART", num);
	
	num = 2573;
	UIT_Credits[0] = UI_AddGameImage(IMAGE_TYPE_ART, "TILES010.ART", num);
	
	num = 2574;
	UIT_Credits[1] = UI_AddGameImage(IMAGE_TYPE_ART, "TILES010.ART", num);
	
	//get the mouse lock state so we can restore it on exit.
	UIT_mouseLockPrev = Input_GetMouseLockState();
	//disable mouse lock
	Input_EnableMouseLocking(0);
	
	//do we start by directly loading a level? (needed for things like multiplayer)
	//only do this the first time.
	if ( UIT_RanOnce == 0 )
	{
		string mapName;
		UI_GetStartMap(mapName);
		if ( mapName[0] != 0 )			//if mapName[0] == 0 then no map name has been passed in.
		{
			selection = 0;
			Game_LoadMap( mapName );	//instead of Game_NewGame() which generates the name, we directly specify it using Game_LoadMap()
			UI_StartScreen("UI_Game");
		}
	}
}

void UI_Title_OnExit()
{
	//restore mouse lock state.
	Input_EnableMouseLocking( UIT_mouseLockPrev );
	
	for (int i=0; i<26; i++)
	{
		UI_FreeImage( UIT_Font[i] );
	}
	for (int i=0; i<8; i++)
	{
		UI_FreeImage( UIT_Blood[i] );
	}
	UI_FreeImage( UIT_Box );
	UI_FreeImage( UIT_Help[0] );
	UI_FreeImage( UIT_Help[1] );
	UI_FreeImage( UIT_Help[2] );
	UI_FreeImage( UIT_Credits[0] );
	UI_FreeImage( UIT_Credits[1] );
	
	UI_SetImageRenderProp( 0, 0 );
	UIT_RanOnce = 1;
}

void UIT_Print(string str, int x, int y, float bright)
{
	int w, h;
	int l = str.length();
	if ( bright > 1.0f ) bright = 1.0f;
	for (int c=0; c<l; c++)
	{
		int cc = str[c];
		if ( cc == 32 )
		{
			x += 4;
		}
		else
		{
			int index = cc - 65;
			UI_RenderImage(UIT_Font[index], x, y, bright, UI_Align_Left, UI_Align_Bottom);
			UI_GetImageSize(UIT_Font[index], w, h); 
			x+=w+2;
		}
	}
}

void UI_Title_OnRender(int state)
{
	UI_EnableImageFilter(0);
	if ( UIT_State == 0 )
	{
		//Render box.
		UI_SetImageRenderProp(0, 1);
		UI_RenderImage(UIT_Box, 89+UIT_offset, 10, 1.0f, UI_Align_Left, UI_Align_Bottom);
		UI_SetImageRenderProp(0, 0);
		int w, h;
		UIT_Print("BLOOD", 130+UIT_offset, 14, 1.0f);
		
		UIT_Print("NEW GAME",    116+UIT_offset,  48, selection == 0 ? selAnim : 0.5f);
		UIT_Print("PLAY ONLINE", 108+UIT_offset,  68, selection == 1 ? selAnim : 0.5f);
		UIT_Print("OPTIONS",     120+UIT_offset,  88, selection == 2 ? selAnim : 0.5f);
		UIT_Print("LOAD GAME",   112+UIT_offset, 108, selection == 3 ? selAnim : 0.5f);
		UIT_Print("HELP",        138+UIT_offset, 128, selection == 4 ? selAnim : 0.5f);
		UIT_Print("CREDITS",     122+UIT_offset, 148, selection == 5 ? selAnim : 0.5f);
		UIT_Print("QUIT",        138+UIT_offset, 168, selection == 6 ? selAnim : 0.5f);
		
		int bloodFrame = bloodAnim/8;
		UI_RenderImage(UIT_Blood[bloodFrame], UIT_offset, 0, 1.0f, UI_Align_Left, UI_Align_Bottom);
		if ( UIT_offset > 0 )	//widescreen...
		{
			UI_RenderImage(UIT_Blood[bloodFrame], UIT_offset-320, 0, 1.0f, UI_Align_Left, UI_Align_Bottom);
			UI_RenderImage(UIT_Blood[bloodFrame], UIT_offset+320, 0, 1.0f, UI_Align_Left, UI_Align_Bottom);
		}

		UI_EnableImageFilter(1);
		//Render version text only when on the title, not when brought up during gameplay.
		if ( UIT_RanOnce == 0 )
		{
			UI_RenderString("Pre-Alpha Build 0.01", 250+UIT_offset, 230, 16, 0.8f, 0.8f, 0.8f, 1.0f);
		}
	}
	else if ( UIT_State >= 1 && UIT_State <= 3 )	//HELP
	{
		UI_RenderImage(UIT_Help[UIT_State-1], UIT_offset, 0, 1.0f, UI_Align_Left, UI_Align_Bottom);
	}
	else if ( UIT_State == 4 )	//CREDITS
	{
		UI_RenderImage(UIT_Credits[0], 91+UIT_offset, 29, 1.0f, UI_Align_Left, UI_Align_Bottom);
		UI_RenderImage(UIT_Credits[1], UIT_offset, 0, 1.0f, UI_Align_Left, UI_Align_Bottom);
	}
	else if ( UIT_State == 5 ) //Episodes
	{
		//Render box.
		UI_SetImageRenderProp(0, 1);
		UI_RenderImage(UIT_Box, 89+UIT_offset, 10, 1.0f, UI_Align_Left, UI_Align_Bottom);
		UI_SetImageRenderProp(0, 0);
		int w, h;
		UIT_Print("EPISODES", 126+UIT_offset, 14, 1.0f);
		
		UIT_Print("THE WAY OF ALL FLESH", 76+UIT_offset,  58, selection == 0 ? selAnim : 0.5f);
		UIT_Print("EVEN DEATH MAY DIE",   86+UIT_offset,  82, selection == 1 ? selAnim : 0.5f);
		UIT_Print("FAREWELL TO ARMS",     88+UIT_offset, 106, selection == 2 ? selAnim : 0.5f);
		UIT_Print("DEAD RECKONING",       98+UIT_offset, 130, selection == 3 ? selAnim : 0.5f);
		UIT_Print("POST MORTEM",         106+UIT_offset, 154, selection == 4 ? selAnim : 0.5f);
		
		int bloodFrame = bloodAnim/8;
		UI_RenderImage(UIT_Blood[bloodFrame], UIT_offset, 0, 1.0f, UI_Align_Left, UI_Align_Bottom);
		if ( UIT_offset > 0 )	//widescreen...
		{
			UI_RenderImage(UIT_Blood[bloodFrame], UIT_offset-320, 0, 1.0f, UI_Align_Left, UI_Align_Bottom);
			UI_RenderImage(UIT_Blood[bloodFrame], UIT_offset+320, 0, 1.0f, UI_Align_Left, UI_Align_Bottom);
		}
	}
	else if ( UIT_State == 6 ) //Difficulty
	{
		//Render box.
		UI_SetImageRenderProp(0, 1);
		UI_RenderImage(UIT_Box, 89+UIT_offset, 10, 1.0f, UI_Align_Left, UI_Align_Bottom);
		UI_SetImageRenderProp(0, 0);
		int w, h;
		UIT_Print("DIFFICULTY", 121+UIT_offset, 14, 1.0f);
		
		UIT_Print("STILL KICKING",     108+UIT_offset,  58, selection == 0 ? selAnim : 0.5f);
		UIT_Print("PINK ON THE INSIDE", 86+UIT_offset,  78, selection == 1 ? selAnim : 0.5f);
		UIT_Print("LIGHTLY BROILED",    96+UIT_offset,  98, selection == 2 ? selAnim : 0.5f);
		UIT_Print("WELL DONE",         118+UIT_offset, 118, selection == 3 ? selAnim : 0.5f);
		UIT_Print("EXTRA CRISPY",      110+UIT_offset, 138, selection == 4 ? selAnim : 0.5f);
		
		int bloodFrame = bloodAnim/8;
		UI_RenderImage(UIT_Blood[bloodFrame], UIT_offset, 0, 1.0f, UI_Align_Left, UI_Align_Bottom);
		if ( UIT_offset > 0 )	//widescreen...
		{
			UI_RenderImage(UIT_Blood[bloodFrame], UIT_offset-320, 0, 1.0f, UI_Align_Left, UI_Align_Bottom);
			UI_RenderImage(UIT_Blood[bloodFrame], UIT_offset+320, 0, 1.0f, UI_Align_Left, UI_Align_Bottom);
		}
	}
}

void UI_Title_OnUpdate()
{
	bloodAnim++;
	if ( bloodAnim >= 64 )
		 bloodAnim = 0;
		
	selAnim += 0.03f;
	if ( selAnim > 1.6f )
		 selAnim = 0.5f;
}

void UI_Title_OnKey(int key)
{
	if ( UIT_State == 0 )
	{
		if ( key == KEY_DOWN )
		{
			selection++;
			if ( selection > 6 ) selection = 0;	
		}
		else if ( key == KEY_UP )
		{
			selection--;
			if ( selection < 0 ) selection = 6;
		}
		
		if ( key == KEY_ENTER )
		{
			if ( selection == 0 )
			{
				UIT_State = 5;
			}
			else if ( selection == 4 )
			{
				UIT_State = 1;
			}
			else if ( selection == 5 )
			{
				UIT_State = 4;
			}
			else if ( selection == 6 )
			{
				Game_Exit();
			}
		}
	}
	else if ( UIT_State >= 1 && UIT_State <= 3 )
	{
		if ( key == KEY_DOWN )
		{
			UIT_State++;
			if ( UIT_State > 3 ) UIT_State = 1;	
		}
		else if ( key == KEY_UP )
		{
			UIT_State--;
			if ( UIT_State < 1 ) UIT_State = 3;
		}
	}
	else if ( UIT_State == 5 )
	{
		if ( key == KEY_DOWN )
		{
			selection++;
			if ( selection > 4 ) selection = 0;	
		}
		else if ( key == KEY_UP )
		{
			selection--;
			if ( selection < 0 ) selection = 4;
		}
		
		if ( key == KEY_ENTER )
		{
			UIT_State = 6;
			UIT_Episode = selection;
			if ( UIT_Episode == 4 ) UIT_Episode++;
			selection = 2;
		}
	}
	else if ( UIT_State == 6 )
	{
		if ( key == KEY_DOWN )
		{
			selection++;
			if ( selection > 4 ) selection = 0;	
		}
		else if ( key == KEY_UP )
		{
			selection--;
			if ( selection < 0 ) selection = 4;
		}
		
		if ( key == KEY_ENTER )
		{
			UIT_Difficulty = selection;
			selection = 0;
			
			Game_NewGame( UIT_Episode, UIT_Difficulty );
			//startup the main game...
			if ( UIT_RanOnce == 1 )	//if this is ran from within the game, then we're already in game mode, we just need to pop this screen off the stack.
			{
				UI_PopScreen();
			}
			else	//otherwise we need to change to the game screen.
			{
				UI_StartScreen("UI_Game");
			}
		}
	}
	
	if ( key == KEY_ESC && UIT_State == 0 && UIT_RanOnce == 1 )
	{
		UI_PopScreen();
	}
	else if ( key == KEY_ESC && UIT_State < 6 )
	{
		UIT_State = 0;
	}
	else if ( key == KEY_ESC && UIT_State == 6 )
	{
		UIT_State = 5;
	}
}

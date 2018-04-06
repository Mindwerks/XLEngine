//////////////////////////////////
/// DarkXL Level Select Screen ///
//////////////////////////////////

int UIL_LFD_Menu;

void UI_LevelSelect_OnEnter()
{
	//Set the virtual screen size.
	//The virtual screen is 1024x768 base modified by the aspect ratio of the real resolution.
	int scrW, scrH, virtW, virtH, offset;
	UI_GetScreenSize(scrW, scrH);
	virtW = 768*scrW/scrH;
	virtH = 768;
	UI_SetVirtualScreenSize(virtW, virtH);
	
	//in widescreen resolutions the UI is pillar boxed to 
	//maintain the correct aspect ratio.
	//the left offset is computed here.
	offset  = (virtW - 1024) / 2;
	
	//LFD Anim.
	UIL_LFD_Menu = UI_CreateLFD_Anim("AGENTMNU.LFD", "AGENTMNU.ANIM", "AGENTMNU.PLTT");
	
	//Buttons
	UI_CreateWindow("UIL_New",    "", 0, offset+ 66, 628, 194, 97, UIWinFlag_None);
	UI_CreateWindow("UIL_Remove", "", 0, offset+286, 628, 194, 97, UIWinFlag_None);
	UI_CreateWindow("UIL_Exit",   "", 0, offset+524, 628, 204, 97, UIWinFlag_None);
	UI_CreateWindow("UIL_Begin",  "", 0, offset+765, 628, 204, 97, UIWinFlag_None);
	
	//Start the Intro cutscene after allocating resources that we'll need.
	Game_StartCutscene( 10 );
}

void UI_LevelSelect_OnExit()
{
	UI_DestroyLFD_Anim( UIL_LFD_Menu );
}

void UI_LevelSelect_OnRender(int state)
{
	if ( Game_IsCutscenePlaying() == 0 )
	{
		//Render the background.
		UI_RenderLFD_Anim( UIL_LFD_Menu, 0, 0, 0 );
		
		//UI_PrintMousePos( 0, 0, 16 );
	}
}

void UI_LevelSelect_OnUpdate()
{
}

void UI_LevelSelect_OnKey(int key, int msg)
{
}

//New Agent
void UIL_New_OnRender(int state)
{
	if ( Game_IsCutscenePlaying() == 0 )
	{
		if ( state == 2 ) state = 0;	//no mouse over support in the original data.
		state = 1-state;
		UI_RenderLFD_Anim( UIL_LFD_Menu, 1+state, 0, -9 );
	}
}

void UIL_New_OnRelease()
{
	//New agent.
}

//Remove Agent
void UIL_Remove_OnRender(int state)
{
	if ( Game_IsCutscenePlaying() == 0 )
	{
		if ( state == 2 ) state = 0;	//no mouse over support in the original data.
		state = 1-state;
		UI_RenderLFD_Anim( UIL_LFD_Menu, 3+state, 0, -9 );
	}
}

void UIL_Remove_OnRelease()
{
	//New agent.
}

//Exit game
void UIL_Exit_OnRender(int state)
{
	if ( Game_IsCutscenePlaying() == 0 )
	{
		if ( state == 2 ) state = 0;	//no mouse over support in the original data.
		state = 1-state;
		UI_RenderLFD_Anim( UIL_LFD_Menu, 5+state, 0, -9 );
	}
}

void UIL_Exit_OnRelease()
{
	Game_Exit();
}

//Start game
void UIL_Begin_OnRender(int state)
{
	if ( Game_IsCutscenePlaying() == 0 )
	{
		if ( state == 2 ) state = 0;	//no mouse over support in the original data.
		state = 1-state;
		UI_RenderLFD_Anim( UIL_LFD_Menu, 7+state, 0, -9 );
	}
}

void UIL_Begin_OnRelease()
{
	//New agent.
}
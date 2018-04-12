////////////////////////
/// DXL_GameView     ///
////////////////////////

int UIG_scrW, UIG_scrH;
int UIG_virtW, UIG_virtH;
int UIG_offset;
float UIG_scale;

void UI_GameView_OnEnter()
{
	//Set the virtual screen size.
	UI_GetScreenSize(UIG_scrW, UIG_scrH);
	UIG_virtW = 320*UIG_scrW/UIG_scrH;
	UIG_virtH = 200;
	UI_SetVirtualScreenSize(UIG_virtW, UIG_virtH);
	
	//handle widescreen resolutions.
	UIG_offset  = (UIG_virtW - 320) / 2;
	
	UI_SetPalette(7, 0);
}

void UI_GameView_OnExit()
{
}

void UI_GameView_OnRender(int state)
{
}

void UI_GameView_OnUpdate()
{
}

void UI_GameView_OnKey(int key)
{
}

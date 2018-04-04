////////////////////////
//// UI Editor Base ////
//////////////////////// 

int  Editor_scrW, Editor_scrH;
int  Editor_Cursor;
bool Editor_Initialized = false;

void XL_UI_Editor_OnEnter()
{
	//Enable mouse locking for mouse look.
	Input_EnableMouseLocking(0);
	//Set the virtual screen size.
	UI_GetScreenSize(Editor_scrW, Editor_scrH);
	UI_SetVirtualScreenSize(Editor_scrW, Editor_scrH);
	
	//load the mouse cursor.
	Editor_Cursor = UI_AddImage("Cursor.png", 0, 0);
	
	if ( Editor_Initialized == false )
	{
		//Create buttons
		UI_PushWindow("XE_Editor", "XL Engine UI Editor [CoreUI_StartMenu]", 0, 16, 16, 600, 400, UIWinFlag_None);
			UI_CreateWindow("XE_Load", "Load",  1,  28,  68,  64, 28, UIWinFlag_None);
			UI_CreateWindow("XE_Save", "Save",  1, 104,  68,  64, 28, UIWinFlag_None);
			UI_CreateWindow("XE_SaveAs", "Save As", 1, 180,  68,  88, 28, UIWinFlag_None);
			UI_CreateWindow("XE_CreateButton", "Create Button", 1,  28, 108, 140, 28, UIWinFlag_None);
			UI_CreateWindow("XE_EditButton",  "Edit Button", 1, 180, 108, 120, 28, UIWinFlag_None);
			UI_CreateWindow("XE_EditorClose", "X", 1, 560, 8, 36, 28, UIWinFlag_None);		
		UI_PopWindow();
		
		//Button editor
		UI_PushWindow("XE_BEdit", "Button Editor", 0, 16, 432, 300, 300, UIWinFlag_None);
			UI_CreateWindow("XE_BEditClose", "X", 1, 260, 8, 36, 28, UIWinFlag_None);		
		UI_PopWindow();
		
		//default button editor to off.
		UI_EnableWindow("XE_BEdit", 0);
		
		Editor_Initialized = true;
	}
}

void XL_UI_Editor_OnExit()
{
	UI_FreeImage( Editor_Cursor );
}

void RenderMouseCursor()
{
	//Get the current mouse position.	
	int mouse_x, mouse_y;
	UI_GetMousePos(mouse_x, mouse_y);
	
	UI_SetImageUV_RangeI(Editor_Cursor, 0, 0, 10, 10);
	UI_RenderImageRect(Editor_Cursor, mouse_x, mouse_y, 20, 20, 1.0f, UI_Align_Left, UI_Align_Bottom);
}

void XL_UI_Editor_OnRender(int state)
{
	//No custom rendering.
}

void XL_UI_Editor_OnPostRender(int state)
{
	UI_EnableImageFilter(0);

	RenderMouseCursor();
		
	UI_EnableImageFilter(1);
}

void XL_UI_Editor_OnUpdate()
{
}

void XL_UI_Editor_OnKey(int key)
{
}

//Buttons
void XE_Load_OnRelease()
{
}

void XE_Save_OnRelease()
{
}

void XE_SaveAs_OnRelease()
{
}

void XE_CreateButton_OnRelease()
{
	UI_EnableWindow("XE_BEdit", 1);
}

void XE_EditButton_OnRelease()
{
	UI_EnableWindow("XE_BEdit", 1);
}

void XE_EditorClose_OnRelease()
{
	UI_PopScreen();
}

//Button Editor
void XE_BEditClose_OnRelease()
{
	UI_EnableWindow("XE_BEdit", 0);
}
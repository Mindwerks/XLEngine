////////////////////////
/// BXL_Title Screen ///
////////////////////////

int UIL_scrW, UIL_scrH;

int[] UIL_Tile(4573);
int imageCount = 4573;
int offsetY=0;

void UI_TexList_OnEnter()
{
	//Set the virtual screen size.
	UI_GetScreenSize(UIL_scrW, UIL_scrH);
	UI_SetVirtualScreenSize(UIL_scrW, UIL_scrH);
	
	for (int i=0; i<imageCount; i++)
	{
		int archiveNum = (i/256);
		string archive;
		if ( archiveNum < 10 )
		{
			archive = "TILES00" + archiveNum + ".ART";
		}
		else
		{
			archive = "TILES0" + archiveNum + ".ART";
		}
		string num = i;
		UIL_Tile[i] = UI_AddGameImage(IMAGE_TYPE_ART, archive, num);
	}
}

void UI_TexList_OnExit()
{
	for (int i=0; i<imageCount; i++)
	{
		UI_FreeImage( UIL_Tile[i] );
	}
}

void UI_TexList_OnRender(int state)
{
	int x=4;
	int y=10 + offsetY;
	int w, h, maxH=0;
	int wN, hN;
	int mX, mY;
	string img;
	int sx=-1, sy=-1;
	UI_GetMousePos(mX, mY);
	UI_GetImageSize(UIL_Tile[0], wN, hN);
	for (int i=0; i<imageCount; i++)
	{
		w = wN; h = hN;
		if ( i < imageCount-1 )
		{
			UI_GetImageSize(UIL_Tile[i+1], wN, hN);
		}
		if ( y+h > 0 )
		{
			UI_RenderImage(UIL_Tile[i], x, y, 1.0f, UI_Align_Left, UI_Align_Bottom);
			if ( mX >= x && mX < x+w && mY >= y && mY < y+h )
			{
				img = i + "  " + w + "x" + h;
				sx = x+1; sy = y+1;
			}
		}
		if ( h > maxH ) maxH = h;
		x += w+4;
		if ( x+wN+4 > UIL_scrW )
		{
			x = 4;
			y += maxH+4;
			maxH = 0;
		}
		if ( y >= UIL_scrH )
		{
			break;
		}
	}
	
	if ( sx > -1 && sy > -1 )
	{
		UI_RenderString(img, sx+2, sy+2, 16, 0.0f, 0.0f, 0.0f, 1.0f);
		UI_RenderString(img, sx, sy, 16, 0.8f, 0.8f, 0.2f, 1.0f);
	}

	//Render version text.
	UI_RenderString("Pre-Alpha Build 0.01", UIL_scrW-200, UIL_scrH-20, 16, 0.8f, 0.8f, 0.8f, 1.0f);
}

void UI_TexList_OnUpdate()
{
	//Mouse over eventually?
}

void UI_TexList_OnKey(int key)
{
	if ( key == KEY_DOWN )
	{
		offsetY -= 16;
	}
	else if ( key == KEY_UP )
	{
		offsetY += 16;
	}
	
	if ( key == KEY_PGDN )
	{
		offsetY -= 128;
	}
	else if ( key == KEY_PGUP )
	{
		offsetY += 128;
	}
	
	if ( offsetY > 0 )
		offsetY = 0;
}

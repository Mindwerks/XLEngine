#include "Font.h"
#include "FontManager.h"
#include "TextureCache.h"
#include "IDriver3D.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

XLFont::XLFont(void)
{
	m_hTex = XL_INVALID_TEXTURE;
}

XLFont::~XLFont(void)
{
}

bool XLFont::Load( const string& szFile, IDriver3D *pDriver )
{
	bool bResult = false;

	FILE *fntFile = fopen( szFile.c_str(), "rb" );
	if ( fntFile )
	{
		u8 version[4];
		fread(version, 1, 4, fntFile);
		if ( version[0] != 'B' || version[1] != 'M' || version[2] != 'F' || version[3] != 0x03 )
		{
		    fclose(fntFile);
			return false;
		}

		//final texture name.
		char szTextureName[64];

		//zero out all the characters.
		memset(m_CharSet.Chars, 0, sizeof(CharDescriptor)*256);

		bool bDone = false;
		while ( feof(fntFile) == 0 && bDone == false )
		{
			//read the block type and size.
			u8  uBlockType;
			u32 uBlockSize;

			fread(&uBlockType, 1, 1, fntFile);
			fread(&uBlockSize, 4, 1, fntFile);

			switch (uBlockType)
			{
				case 1:	//INFO block
				{
					u16 fontSize, stretchH;
					u8  flags, charSet, aa, spacingHoriz, spacingVert, outline;
					u8  padding[4];
					char szName[64];
					fread(&fontSize,	 2, 1, fntFile);
					fread(&flags,		 1, 1, fntFile);
					fread(&charSet,		 1, 1, fntFile);
					fread(&stretchH,	 2, 1, fntFile);
					fread(&aa,			 1, 1, fntFile);
					fread(&padding[0],   1, 1, fntFile);
					fread(&padding[1],   1, 1, fntFile);
					fread(&padding[2],   1, 1, fntFile);
					fread(&padding[3],   1, 1, fntFile);
					fread(&spacingHoriz, 1, 1, fntFile);
					fread(&spacingVert,  1, 1, fntFile);
					fread(&outline,		 1, 1, fntFile);
					fread(szName,		 1, uBlockSize-14, fntFile);
				}
				break;
				case 2:
				{
					u8 flags;
					u8 channels[4];
					fread(&m_CharSet.LineHeight, 2, 1, fntFile);
					fread(&m_CharSet.Base,		 2, 1, fntFile);
					fread(&m_CharSet.Width,		 2, 1, fntFile);
					fread(&m_CharSet.Height,	 2, 1, fntFile);
					fread(&m_CharSet.Pages,		 2, 1, fntFile);
					fread(&flags,				 1, 1, fntFile);
					fread(&channels[0],			 1, 1, fntFile);
					fread(&channels[1],			 1, 1, fntFile);
					fread(&channels[2],			 1, 1, fntFile);
					fread(&channels[3],			 1, 1, fntFile);
				}
				break;
				case 3:
				{
					fread(szTextureName, 1, uBlockSize, fntFile);
				}
				break;
				case 4:
				{
					u32 ID;
					u8 page, channel;
					s32 nBlockSize = (s32)uBlockSize;
					while ( nBlockSize > 0 )
					{
						fread(&ID, 4, 1, fntFile);

                        if ( ID > 255 )
                        {
                            ID = 255;
                        }
						fread(&m_CharSet.Chars[ID].x,		 2, 1, fntFile);
						fread(&m_CharSet.Chars[ID].y,		 2, 1, fntFile);
						fread(&m_CharSet.Chars[ID].Width,    2, 1, fntFile);
						fread(&m_CharSet.Chars[ID].Height,   2, 1, fntFile);
						fread(&m_CharSet.Chars[ID].XOffset,  2, 1, fntFile);
						fread(&m_CharSet.Chars[ID].YOffset,  2, 1, fntFile);
						fread(&m_CharSet.Chars[ID].XAdvance, 2, 1, fntFile);
						fread(&page,						 1, 1, fntFile);
						fread(&channel,						 1, 1, fntFile);
						m_CharSet.Chars[ID].Page = page;

						nBlockSize -= 20;
					};
				}
				break;
				case 5:	//Kerning pairs - currently unused.
				{
					u32 first, second;
					u16 amount;
					fread(&first,  4, 1, fntFile);
					fread(&second, 4, 1, fntFile);
					fread(&amount, 2, 1, fntFile);
				}
				break;
				default:
				{
					bDone = true;
				}
			};
		};

		fclose(fntFile);
		bResult = true;

		//now load the texture.
		m_hTex = TextureCache::LoadTexture( szTextureName, false );
	}

	return bResult;
}

u32 XLFont::ComputePixelPos(const string& szString, u32 uPos)
{
	const char *pszText = szString.c_str();
	s32 nCurX = 0;
	for (u32 i=0; i<uPos; i++)
	{
		nCurX += m_CharSet.Chars[ pszText[i] ].XAdvance;
	}
	return (u32)nCurX;
}

s32 XLFont::FillVB(s32 x, s32 y, const string& szString, FontVertex *pVB_Data)
{
	s32 nCurX = x;
	s32 nCurY = y;

	f32 texelX  = 1.0f / (f32)m_CharSet.Width;
	f32 texelY  = 1.0f / (f32)m_CharSet.Height;

	size_t l = szString.size();

	assert(l < MAX_STRING_COUNT);

	const char *pszText = szString.c_str();
	for (size_t i=0, i4=0; i<l; i++, i4+=4)
	{
		u16 CharX   = m_CharSet.Chars[ pszText[i] ].x;
		u16 CharY   = m_CharSet.Chars[ pszText[i] ].y;
		u16 Width   = m_CharSet.Chars[ pszText[i] ].Width;
		u16 Height  = m_CharSet.Chars[ pszText[i] ].Height;
		s16 OffsetX = m_CharSet.Chars[ pszText[i] ].XOffset;
		s16 OffsetY = m_CharSet.Chars[ pszText[i] ].YOffset;

		//upper left
		pVB_Data[i4+0].uv.x  = (float)CharX * texelX;
		pVB_Data[i4+0].uv.y  = (float)CharY * texelY;
		pVB_Data[i4+0].pos.x = (float)(nCurX + OffsetX);
		pVB_Data[i4+0].pos.y = (float)(nCurY + OffsetY);
		pVB_Data[i4+0].pos.z = -1.0f;

		//upper right
		pVB_Data[i4+1].uv.x  = (float)(CharX+Width) * texelX;
		pVB_Data[i4+1].uv.y  = (float)CharY * texelY;
		pVB_Data[i4+1].pos.x = (float)(Width + nCurX + OffsetX);
		pVB_Data[i4+1].pos.y = (float)(nCurY + OffsetY);
		pVB_Data[i4+1].pos.z = -1.0f;

		//lower right
		pVB_Data[i4+2].uv.x  = (float)(CharX+Width) * texelX;
		pVB_Data[i4+2].uv.y  = (float)(CharY+Height) * texelY;
		pVB_Data[i4+2].pos.x = (float)(Width + nCurX + OffsetX);
		pVB_Data[i4+2].pos.y = (float)(nCurY + Height + OffsetY);
		pVB_Data[i4+2].pos.z = -1.0f;

		//lower left
		pVB_Data[i4+3].uv.x  = (float)CharX * texelX;
		pVB_Data[i4+3].uv.y  = (float)(CharY+Height) * texelY;
		pVB_Data[i4+3].pos.x = (float)(nCurX + OffsetX);
		pVB_Data[i4+3].pos.y = (float)(nCurY + Height + OffsetY);
		pVB_Data[i4+3].pos.z = -1.0f;

		nCurX += m_CharSet.Chars[ pszText[i] ].XAdvance;
	}

	return (s32)l;
}

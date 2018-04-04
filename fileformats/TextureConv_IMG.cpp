#include "TextureConv_IMG.h"
#include "ArchiveManager.h"
#include "../memory/ScratchPad.h"
#include "../fileformats/TextureLoader.h"
#include "../math/Math.h"
#include <assert.h>

enum
{
	PAL_MAP=0,
	PAL_OLDMAP,
	PAL_OLDPAL,
	PAL_ART,
	PAL_DANKBMAP,
	PAL_FMAP,
	PAL_NIGHTSKY,
	PAL_PAL,
	PAL_COUNT
};

#pragma pack(push)
#pragma pack(1)

struct ImageHeader
{
	s16 XOffset;
	s16 YOffset;
	s16 Width;
	s16 Height;
	u16 Compression;
	u16 DataLength;
};

#pragma pack(pop)

TextureConv_IMG::TextureConv_IMG() : TextureConverter()
{
}

TextureConv_IMG::~TextureConv_IMG()
{
}

u32 TextureConv_IMG::GetHackID(const char *pszImage)
{
	u32 uHackID = 0;
	if ( stricmp(pszImage, "COMPASS.IMG") == 0 )
		uHackID = 1;
	else if ( stricmp(pszImage, "PICK03I0.IMG") == 0 || stricmp(pszImage, "CHGN00I0.IMG") == 0 || stricmp(pszImage, "DIE_00I0.IMG") == 0 || 
		      stricmp(pszImage, "PICK02I0.IMG") == 0 || stricmp(pszImage, "PRIS00I0.IMG") == 0 || stricmp(pszImage, "TITL00I0.IMG") == 0 ||
			  stricmp(pszImage, "CHGN00I0.IMG") == 0 )
		uHackID = 2;
	else if ( stricmp(pszImage, "INFO00I0.IMG") == 0 || stricmp(pszImage, "INVE00I0.IMG") == 0 || stricmp(pszImage, "INVE01I0.IMG") == 0 ||
			  stricmp(pszImage, "TRAV0I00.IMG") == 0 || stricmp(pszImage, "CNFG00I0.IMG") == 0 || stricmp(pszImage, "LOAD00I0.IMG") == 0 || 
			  stricmp(pszImage, "AMAP00I0.IMG") == 0 || stricmp(pszImage, "MAP100I0.IMG") == 0 || stricmp(pszImage, "TAMRIEL2.IMG") == 0 ||
			  stricmp(pszImage, "CUST00I0.IMG") == 0 || stricmp(pszImage, "CHAR00I0.IMG") == 0 || stricmp(pszImage, "CHAR01I0.IMG") == 0 || 
			  stricmp(pszImage, "CHAR02I0.IMG") == 0 || stricmp(pszImage, "CHAR03I0.IMG") == 0 || stricmp(pszImage, "CHAR04I0.IMG") == 0 ||
			  stricmp(pszImage, "CHAR05I0.IMG") == 0 || stricmp(pszImage, "BIOG00I0.IMG") == 0 )
	{
		if ( strnicmp(pszImage, "MAP100I0", 8)  == 0 )
			uHackID = 3;
		else
			uHackID = 4;
	}
	else if ( stricmp(pszImage,  "TRAV0I03.IMG") == 0 )
		uHackID = 5;
	else if ( stricmp(pszImage,  "TRAV01I0.IMG") == 0 || stricmp(pszImage, "TRAV01I1.IMG") == 0 )
		uHackID = 6;
	else if ( stricmp(pszImage,  "TRAV0I00.IMG") == 0 || stricmp(pszImage, "TRAV0I01.IMG") == 0 )
		uHackID = 7;
	else if ( stricmp(pszImage,  "BUTN01I0.IMG") == 0 )
		uHackID = 8;
	else if ( strnicmp(pszImage, "NITE", 4)		 == 0 )
		uHackID = 9;
	else if ( stricmp(pszImage,  "SPOP.RCI")	 == 0 )
		uHackID = 10;
	else if ( stricmp(pszImage,  "BUTTONS.RCI")	 == 0 )
		uHackID = 11;

	return uHackID;
}

bool TextureConv_IMG::ConvertTexture_Pal8(u8 *pConvertedData, s32& nOffsX, s32& nOffsY, u32& uWidth, u32& uHeight, const u8 *pSourceData, u32 uLen, const u8 *pPalette, bool bCopyPal, u32 uHackID/*=0*/)
{
	ImageHeader header;
	u8 *pImageData=NULL;
	u8 *pCustomPal=NULL;
	int nPal = PAL_PAL;
	nOffsX = 0;
	nOffsY = 0;

	switch (uHackID)
	{
		case 0:
		break;
		case 1:
		{
			header.Width = 322;
			header.Height = 14;
			header.Compression = 0;
			header.XOffset = header.YOffset = 0;
			header.DataLength = 322*14;
			pImageData = new u8[header.Width*header.Height];
			memcpy(pImageData, pSourceData, header.DataLength);
		}
		break;
		case 2:
		{
			//Paletted RCI files.
			header.Width = 320;
			header.Height = 200;
			header.Compression = 0;
			header.XOffset = header.YOffset = 0;
			header.DataLength = 320*200;
			pImageData = new u8[header.Width*header.Height];
			memcpy(pImageData, pSourceData, header.DataLength);

			pCustomPal = new u8[768];
			memcpy(pCustomPal, &pSourceData[header.DataLength], 768);
			for (int i=0; i<768; i++)
			{
				pCustomPal[i] <<= 2;
			}
		}
		break;
		case 3:
			nPal = PAL_MAP;
		case 4:
		{
			header.Width = 320;
			header.Height = 200;
			header.Compression = 0;
			header.XOffset = header.YOffset = 0;
			header.DataLength = 320*200;
			pImageData = new u8[header.Width*header.Height];
			memcpy(pImageData, pSourceData, header.DataLength);
		}
		break;
		case 5:
		{
			header.Width = 45;
			header.Height = 22;
			header.Compression = 0;
			header.XOffset = header.YOffset = 0;
			header.DataLength = 45*22;
			pImageData = new u8[header.Width*header.Height];
			memcpy(pImageData, pSourceData, header.DataLength);
		}
		break;
		case 6:
		{
			header.Width = 179;
			header.Height = 22;
			header.Compression = 0;
			header.XOffset = header.YOffset = 0;
			header.DataLength = header.Width*header.Height;
			pImageData = new u8[header.Width*header.Height];
			memcpy(pImageData, pSourceData, header.DataLength);
		}
		break;
		case 7:
		{
			header.Width = 320;
			header.Height = 200;
			header.Compression = 0;
			header.XOffset = header.YOffset = 0;
			header.DataLength = header.Width*header.Height;
			pImageData = new u8[header.Width*header.Height];
			memcpy(pImageData, pSourceData, header.DataLength);
		}
		break;
		case 8:
		{
			header.Width = 184;
			header.Height = 144;
			header.Compression = 0;
			header.XOffset = header.YOffset = 0;
			header.DataLength = header.Width*header.Height;
			pImageData = new u8[header.Width*header.Height];
			memcpy(pImageData, pSourceData, header.DataLength);
		}
		break;
		case 9:
		{
			header.Width = 512;
			header.Height = 219;
			header.Compression = 0;
			header.XOffset = header.YOffset = 0;
			header.DataLength = header.Width*header.Height;
			pImageData = new u8[header.Width*header.Height];
			memcpy(pImageData, pSourceData, header.DataLength);
		}
		break;
		case 10:
		{
			header.Width = 22;
			header.Height = 22;
			header.Compression = 0;
			header.XOffset = header.YOffset = 0;
			header.DataLength = 22*22;

			pImageData = new u8[header.Width*header.Height];
			int index = 0;
			int offset = (22*22)*index;
			memcpy(pImageData, &pSourceData[offset], header.DataLength);
		}
		break;
		case 11:
		{
			header.Width = 32*21;
			header.Height = 16;
			header.Compression = 0;
			header.XOffset = header.YOffset = 0;
			header.DataLength = 32*21*16;

			pImageData = new u8[header.Width*header.Height];
			u8 *pTmp = new u8[32*16];
			for (int i=0; i<21; i++)
			{
				int offset = (32*16)*i;
				memcpy(pTmp, &pSourceData[offset], 32*16);

				for (int y=0; y<16; y++)
				{
					memcpy( &pImageData[y*header.Width+i*32], &pTmp[y*32], 32 );
				}
			}
			delete [] pTmp;
		}
		break;
	};

	if ( header.Compression == 0x0002 )
	{
		//compressed.
	}
	else
	{
		//uncompressed.
		int pitch = header.Width;

		//now we'll create an RGBA texture and create a hardware texture out of it.
		u8 *pal = NULL;
		u32 palStride = 3;
		if ( pCustomPal )
		{
			pal = pCustomPal;
		}
		else
		{
			palStride = 4;
			pal = TextureLoader::GetPaletteData(nPal);
		}
		for (int h=0; h<header.Width*header.Height; h++)
		{
			unsigned int r, g, b, a=0x80;
			int index = pImageData[h]*palStride;
			r = pal[ index+0 ];
			g = pal[ index+1 ];
			b = pal[ index+2 ];

			if ( nPal == PAL_MAP )
			{
				r <<= 2;
				g <<= 2;
				b <<= 2;

				if ( r > 255 ) r = 255;
				if ( g > 255 ) g = 255;
				if ( b > 255 ) b = 255;
			}

			if ( pImageData[h] == 0 )
			{
				a = 0x00;
			}
			
			int hh = h*4;
			if (nPal == PAL_NIGHTSKY && (h%header.Width)==header.Width-1)
			{
				pConvertedData[hh+0] = pConvertedData[(h-header.Width+1)*4+0];
				pConvertedData[hh+1] = pConvertedData[(h-header.Width+1)*4+1];
				pConvertedData[hh+2] = pConvertedData[(h-header.Width+1)*4+2];
				pConvertedData[hh+3] = pConvertedData[(h-header.Width+1)*4+3];
			}
			else
			{
				pConvertedData[hh+0] = r;
				pConvertedData[hh+1] = g;
				pConvertedData[hh+2] = b;
				pConvertedData[hh+3] = a;
			}
		}
		uWidth  = header.Width;
		uHeight = header.Height;
	}

	SafeDeleteArr( pImageData );
	SafeDeleteArr( pCustomPal );

	return true;
}

struct TexRecordF
{
	s16 w, h;
	s16 offsetX, offsetY;
	TextureHandle hTex;
};

struct TextureFile
{
	s32 nRecordCnt;
	TexRecordF *pRecords;
};

#pragma pack(push)
#pragma pack(1)

struct TexRecordHeader
{
	s16 type1;
	s32 recordPos;
	s16 type2;
	s32 unknown;
	s64 NullValue;
};

struct TexRecord
{
	s16 offsetX;
	s16 offsetY;
	s16 width;
	s16 height;
	u16 compression;
	u32 recordSize;
	u32 dataOffs;
	s16 IsNormal;
	u16 frameCount;
	u16 unknown1;
	s16 xscale;
	s16 yscale;
};

#pragma pack(pop)

const u16 Ctx_RleCompressed = 0x0002;
const u16 Ctx_Uncompressed  = 0x0000;
const u16 Ctx_ImageRle      = 0x0108;
const u16 Ctx_RecordRle     = 0x1108;

void _ReadFromRow_MFU(u8 *row, int w, int& index, const u8 *pData)
{
	u8 c = pData[index]; index++;
	bool isZero = true;
	int p = 0;
	do
	{
		for (int i=0; i<c; i++)
		{
			if ( isZero )
			{
				row[ p++ ] = 0;
			}
			else
			{
				row[p] = pData[index]; 
				index++; p++; 
			}
		}
		if ( p < w || (p == w && isZero) )
		{ 
			c = pData[index]; index++; 
		}
		isZero = !isZero;
	} while ( p < w );
}

bool TextureConv_IMG::ConvertTexture_Pal8_TexList(u8 *pConvertedData, s32& nOffsX, s32& nOffsY, u32& uWidth, u32& uHeight, const u8 *pSourceData, u32 uLen, const u8 *pPalette, int nRecord, u32 uHackID)
{
	s32 index = 0;
	s16 nRecordCount = *((s16 *)&pSourceData[index]); index +=  2;
	char *pszName    =  (char *)&pSourceData[index];  index += 24;

	if ( nRecord < 0 || (nRecord) >= nRecordCount )
		 nRecord = 0;

	TexRecordHeader *pHeaders = (TexRecordHeader *)&pSourceData[index];

	index = pHeaders[ nRecord ].recordPos;
	TexRecord *pRec = (TexRecord *)&pSourceData[index];
	bool bCompressed = (pRec->compression == Ctx_RleCompressed || pRec->compression == Ctx_ImageRle || pRec->compression == Ctx_RecordRle);
	index = pHeaders[ nRecord ].recordPos + (s32)pRec->dataOffs;
	u8 *buffer = NULL;

	nOffsX  = (s32)pRec->offsetX;
	nOffsY  = (s32)pRec->offsetY;
	uWidth  = (u32)pRec->width;
	uHeight = (u32)pRec->height;

	m_aExtraData[0] = pRec->xscale;
	m_aExtraData[1] = pRec->yscale;

	if ( pRec->dataOffs == 0 || pRec->frameCount < 1 ) //this is a special file.
	{
		u8 color = ((u16)pHeaders[ nRecord ].type1)>>8;
		int pal_index = color*4;
		u8 r, g, b, a=0xff;
		r = MIN(pPalette[ pal_index+0 ], 255);
		g = MIN(pPalette[ pal_index+1 ], 255);
		b = MIN(pPalette[ pal_index+2 ], 255);
		
		nOffsX  = 0;
		nOffsY  = 0;
		uWidth  = 4;
		uHeight = 4;

		for (int h=0; h<16; h++)
		{
			pConvertedData[(h<<2)+0] = r;
			pConvertedData[(h<<2)+1] = g;
			pConvertedData[(h<<2)+2] = b;
			pConvertedData[(h<<2)+3] = a;
		}
	}
	else if ( pRec->frameCount == 1 && bCompressed == false )
	{
		assert( pRec->width > 0 && pRec->width < 2048 );
		assert( pRec->height > 0 && pRec->height < 2048 );

		buffer = new u8[pRec->width*pRec->height];
		int pitch = 256 - pRec->width;
		int buf_idx = 0;
		for (int h=0; h<pRec->height; h++)
		{
			memcpy(&buffer[buf_idx], &pSourceData[index], pRec->width);
			buf_idx += pRec->width;

			index += 256;
		}

		//now we'll create an RGBA texture and create a hardware texture out of it.
		for (int h=0; h<pRec->width*pRec->height; h++)
		{
			assert( buffer[h] < 256 );
			int pal_index = buffer[h]*4;
			int alpha = 0xff;
			if ( pal_index == 0 ) alpha = 0;
			pConvertedData[(h<<2)+0] = MIN(pPalette[ pal_index+0 ], 255);
			pConvertedData[(h<<2)+1] = MIN(pPalette[ pal_index+1 ], 255);
			pConvertedData[(h<<2)+2] = MIN(pPalette[ pal_index+2 ], 255);
			pConvertedData[(h<<2)+3] = alpha;
		}
	}
	else if ( pRec->frameCount > 0 && bCompressed == false )
	{
		int index = pHeaders[ nRecord ].recordPos+pRec->dataOffs;
		int frameOffsStart = index;
		int *pFrameOffsetList = (int *)&pSourceData[index]; index += sizeof(int)*pRec->frameCount;

		//for now just grab frame 0...
		index = frameOffsStart + pFrameOffsetList[0];
		s16 fw = *((s16 *)&pSourceData[index]); index += 2;
		s16 fh = *((s16 *)&pSourceData[index]); index += 2;
		uWidth  = (u32)fw;
		uHeight = (u32)fh;

		assert( fw > 0 && fw < 2048 );
		assert( fh > 0 && fh < 2048 );

		buffer = new u8[fw*fh];
		for (int h=0; h<fh; h++)
		{
			_ReadFromRow_MFU(&buffer[h*fw], fw, index, pSourceData);
		}
		
		//now we'll create an RGBA texture and create a hardware texture out of it.
		for (int h=0; h<fw*fh; h++)
		{
			assert( buffer[h] < 256 );
			int pal_index = buffer[h]*4;
			int alpha = 0xff;
			if ( pal_index == 0 ) alpha = 0;
			pConvertedData[(h<<2)+0] = MIN(pPalette[ pal_index+0 ], 255);
			pConvertedData[(h<<2)+1] = MIN(pPalette[ pal_index+1 ], 255);
			pConvertedData[(h<<2)+2] = MIN(pPalette[ pal_index+2 ], 255);
			pConvertedData[(h<<2)+3] = alpha;
		}
	}
	else
	{
		assert( pRec->width  > 0 && pRec->width  < 2048 );
		assert( pRec->height > 0 && pRec->height < 2048 );

		struct RleHeader
		{
			s16 RowOffset;
			u16 RowEncoding;
		};

		buffer = new u8[pRec->width*pRec->height];
		index = pHeaders[ nRecord ].recordPos+pRec->dataOffs;
		RleHeader *pRleHeaders = (RleHeader *)&pSourceData[index]; 

		int buf_idx = 0;
		for (int h=0; h<pRec->height; h++)
		{
			index = pHeaders->recordPos + pRleHeaders[h].RowOffset;
			if ( pRleHeaders[h].RowEncoding == 0x8000 )
			{
				s16 row_w;
				s32 p = 0;
				s16 probe;
				u8  pixel;
				row_w = *((s16 *)&pSourceData[index]); index += 2;
				do
				{
					probe = *((s16 *)&pSourceData[index]); index += 2;
					if ( probe < 0 )
					{
						probe = -probe;
						pixel = *((u8 *)&pSourceData[index]); index++;
						for (int pp=0; pp<probe; pp++)
						{
							buffer[buf_idx + p] = pixel;
							p++;
						}
					}
					else if ( probe > 0 )
					{
						for (int pp=0; pp<probe; pp++)
						{
							buffer[buf_idx + p] = *((u8 *)&pSourceData[index]); index++;
							p++;
						}
					}
				} while (p < row_w);
				buf_idx += pRec->width;
			}
			else
			{
				memcpy(&buffer[buf_idx], &pSourceData[index], pRec->width);
				index   += pRec->width;
				buf_idx += pRec->width;
			}
		}

		//now we'll create an RGBA texture and create a hardware texture out of it.
		for (int h=0; h<pRec->width*pRec->height; h++)
		{
			assert( buffer[h] < 256 );
			int pal_index = buffer[h]*4;
			int alpha = 0xff;
			if ( pal_index == 0 ) alpha = 0;
			pConvertedData[(h<<2)+0] = MIN(pPalette[ pal_index+0 ], 255);
			pConvertedData[(h<<2)+1] = MIN(pPalette[ pal_index+1 ], 255);
			pConvertedData[(h<<2)+2] = MIN(pPalette[ pal_index+2 ], 255);
			pConvertedData[(h<<2)+3] = alpha;
		}
	}
	delete [] buffer;

	return true;
}

bool TextureConv_IMG::ConvertTexture_8bpp(u8 *pConvertedData, s32& nOffsX, s32& nOffsY, u32& uWidth, u32& uHeight, const u8 *pSourceData, u32 uLen, u32 uHackID/*=0*/)
{
	ImageHeader header;
	u8 *pImageData=NULL;
	u8 *pCustomPal=NULL;
	int nPal = PAL_PAL;
	nOffsX = 0;
	nOffsY = 0;

	switch (uHackID)
	{
		case 0:
		break;
		case 1:
		{
			header.Width = 322;
			header.Height = 14;
			header.Compression = 0;
			header.XOffset = header.YOffset = 0;
			header.DataLength = 322*14;
			pImageData = new u8[header.Width*header.Height];
			memcpy(pImageData, pSourceData, header.DataLength);
		}
		break;
		case 2:
		{
			//Paletted RCI files.
			header.Width = 320;
			header.Height = 200;
			header.Compression = 0;
			header.XOffset = header.YOffset = 0;
			header.DataLength = 320*200;
			pImageData = new u8[header.Width*header.Height];
			memcpy(pImageData, pSourceData, header.DataLength);

			pCustomPal = new u8[768];
			memcpy(pCustomPal, &pSourceData[header.DataLength], 768);
			for (int i=0; i<768; i++)
			{
				pCustomPal[i] <<= 2;
			}
			TextureLoader::SetPalette(31, pCustomPal, 768, 0);
		}
		break;
		case 3:
			nPal = PAL_MAP;
		case 4:
		{
			header.Width = 320;
			header.Height = 200;
			header.Compression = 0;
			header.XOffset = header.YOffset = 0;
			header.DataLength = 320*200;
			pImageData = new u8[header.Width*header.Height];
			memcpy(pImageData, pSourceData, header.DataLength);
		}
		break;
		case 5:
		{
			header.Width = 45;
			header.Height = 22;
			header.Compression = 0;
			header.XOffset = header.YOffset = 0;
			header.DataLength = 45*22;
			pImageData = new u8[header.Width*header.Height];
			memcpy(pImageData, pSourceData, header.DataLength);
		}
		break;
		case 6:
		{
			header.Width = 179;
			header.Height = 22;
			header.Compression = 0;
			header.XOffset = header.YOffset = 0;
			header.DataLength = header.Width*header.Height;
			pImageData = new u8[header.Width*header.Height];
			memcpy(pImageData, pSourceData, header.DataLength);
		}
		break;
		case 7:
		{
			header.Width = 320;
			header.Height = 200;
			header.Compression = 0;
			header.XOffset = header.YOffset = 0;
			header.DataLength = header.Width*header.Height;
			pImageData = new u8[header.Width*header.Height];
			memcpy(pImageData, pSourceData, header.DataLength);
		}
		break;
		case 8:
		{
			header.Width = 184;
			header.Height = 144;
			header.Compression = 0;
			header.XOffset = header.YOffset = 0;
			header.DataLength = header.Width*header.Height;
			pImageData = new u8[header.Width*header.Height];
			memcpy(pImageData, pSourceData, header.DataLength);
		}
		break;
		case 9:
		{
			header.Width = 512;
			header.Height = 219;
			header.Compression = 0;
			header.XOffset = header.YOffset = 0;
			header.DataLength = header.Width*header.Height;
			pImageData = new u8[header.Width*header.Height];
			memcpy(pImageData, pSourceData, header.DataLength);
		}
		break;
		case 10:
		{
			header.Width = 22;
			header.Height = 22;
			header.Compression = 0;
			header.XOffset = header.YOffset = 0;
			header.DataLength = 22*22;

			pImageData = new u8[header.Width*header.Height];
			int index = 0;
			int offset = (22*22)*index;
			memcpy(pImageData, &pSourceData[offset], header.DataLength);
		}
		break;
		case 11:
		{
			header.Width = 32*21;
			header.Height = 16;
			header.Compression = 0;
			header.XOffset = header.YOffset = 0;
			header.DataLength = 32*21*16;

			pImageData = new u8[header.Width*header.Height];
			u8 *pTmp = new u8[32*16];
			for (int i=0; i<21; i++)
			{
				int offset = (32*16)*i;
				memcpy(pTmp, &pSourceData[offset], 32*16);

				for (int y=0; y<16; y++)
				{
					memcpy( &pImageData[y*header.Width+i*32], &pTmp[y*32], 32 );
				}
			}
			delete [] pTmp;
		}
		break;
	};

	if ( header.Compression == 0x0002 )
	{
		//compressed.
	}
	else
	{
		uWidth  = header.Width;
		uHeight = header.Height;
		memcpy(pConvertedData, pImageData, uWidth*uHeight);
	}

	SafeDeleteArr( pImageData );
	SafeDeleteArr( pCustomPal );

	return true;
}

u32 TextureConv_IMG::ConvertTexture_8bpp_TexList(u8 *pConvertedData, s32& nOffsX, s32& nOffsY, u32& uWidth, u32& uHeight, const u8 *pSourceData, u32 uLen, int nRecord, u32 uHackID/*=0*/)
{
	s32 index = 0;
	s16 nRecordCount = *((s16 *)&pSourceData[index]); index +=  2;
	char *pszName    =  (char *)&pSourceData[index];  index += 24;

	if ( nRecord < 0 || nRecord >= nRecordCount )
		 nRecord = 0;

	TexRecordHeader *pHeaders = (TexRecordHeader *)&pSourceData[index];

	index = pHeaders[ nRecord ].recordPos;
	TexRecord *pRec = (TexRecord *)&pSourceData[index];
	bool bCompressed = (pRec->compression == Ctx_RleCompressed || pRec->compression == Ctx_ImageRle || pRec->compression == Ctx_RecordRle);
	index = pHeaders[ nRecord ].recordPos + (s32)pRec->dataOffs;
	u8 *buffer = NULL;

	nOffsX  = (s32)pRec->offsetX;
	nOffsY  = (s32)pRec->offsetY;
	uWidth  = (u32)pRec->width;
	uHeight = (u32)pRec->height;

	m_aExtraData[0] = pRec->xscale;
	m_aExtraData[1] = pRec->yscale;

	u32 uFrameCnt = 1;
	if ( pRec->dataOffs == 0 || pRec->frameCount < 1 ) //this is a special file.
	{
		nOffsX  = 0;
		nOffsY  = 0;
		uWidth  = 4;
		uHeight = 4;

		u8 color = ((u16)pHeaders[ nRecord ].type1)>>8;
		for (int h=0; h<16; h++)
		{
			pConvertedData[h] = color;
		}
	}
	else if ( pRec->frameCount == 1 && bCompressed == false )
	{
		assert( pRec->width  > 0 && pRec->width  < 2048 );
		assert( pRec->height > 0 && pRec->height < 2048 );

		buffer = new u8[pRec->width*pRec->height];
		int pitch = 256 - pRec->width;
		int buf_idx = 0;
		for (int h=0; h<pRec->height; h++)
		{
			memcpy(&buffer[buf_idx], &pSourceData[index], pRec->width);
			buf_idx += pRec->width;

			index += 256;
		}

		memcpy(pConvertedData, buffer, pRec->width*pRec->height);
	}
	else if ( pRec->frameCount > 0 && bCompressed == false )
	{
		int index = pHeaders[ nRecord ].recordPos+pRec->dataOffs;
		int frameOffsStart = index;
		int *pFrameOffsetList = (int *)&pSourceData[index]; index += sizeof(int)*pRec->frameCount;

		uFrameCnt = pRec->frameCount;

		//read the frames.
		u32 uFrameOffset = 0;
		s32 frame0_w = 0;
		s32 frame0_h = 0;
		for (u32 f=0; f<uFrameCnt; f++)
		{
			index = frameOffsStart + pFrameOffsetList[f];
			s16 fw = *((s16 *)&pSourceData[index]); index += 2;
			s16 fh = *((s16 *)&pSourceData[index]); index += 2;
	
			if ( f == 0 )
			{
				frame0_w = fw;
				frame0_h = fh;
				buffer = new u8[fw*fh*uFrameCnt];
			}
			else
			{
				assert( frame0_w == fw && frame0_h == fh );
			}

			uWidth  = (u32)fw;
			uHeight = (u32)fh;

			assert( fw > 0 && fw < 2048 );
			assert( fh > 0 && fh < 2048 );

			for (int h=0; h<fh; h++)
			{
				_ReadFromRow_MFU(&buffer[h*fw+uFrameOffset], fw, index, pSourceData);
			}

			uFrameOffset += fw*fh;
		}
		
		memcpy(pConvertedData, buffer, uWidth*uHeight*uFrameCnt);
	}
	else
	{
		assert( pRec->width  > 0 && pRec->width  < 2048 );
		assert( pRec->height > 0 && pRec->height < 2048 );

		struct RleHeader
		{
			s16 RowOffset;
			u16 RowEncoding;
		};

		buffer = new u8[pRec->width*pRec->height];
		index = pHeaders[ nRecord ].recordPos+pRec->dataOffs;
		RleHeader *pRleHeaders = (RleHeader *)&pSourceData[index]; 

		int buf_idx = 0;
		for (int h=0; h<pRec->height; h++)
		{
			index = pHeaders->recordPos + pRleHeaders[h].RowOffset;
			if ( pRleHeaders[h].RowEncoding == 0x8000 )
			{
				s16 row_w;
				s32 p = 0;
				s16 probe;
				u8  pixel;
				row_w = *((s16 *)&pSourceData[index]); index += 2;
				do
				{
					probe = *((s16 *)&pSourceData[index]); index += 2;
					if ( probe < 0 )
					{
						probe = -probe;
						pixel = *((u8 *)&pSourceData[index]); index++;
						for (int pp=0; pp<probe; pp++)
						{
							buffer[buf_idx + p] = pixel;
							p++;
						}
					}
					else if ( probe > 0 )
					{
						for (int pp=0; pp<probe; pp++)
						{
							buffer[buf_idx + p] = *((u8 *)&pSourceData[index]); index++;
							p++;
						}
					}
				} while (p < row_w);
				buf_idx += pRec->width;
			}
			else
			{
				memcpy(&buffer[buf_idx], &pSourceData[index], pRec->width);
				index   += pRec->width;
				buf_idx += pRec->width;
			}
		}

		memcpy(pConvertedData, buffer, pRec->width*pRec->height);
	}

	delete [] buffer;

	return uFrameCnt;
}
#include "TextureConv_PCX.h"
#include "ArchiveManager.h"
#include "../memory/ScratchPad.h"
#include "../fileformats/TextureLoader.h"
#include <assert.h>
#include <cstring>

TextureConv_PCX::TextureConv_PCX() : TextureConverter()
{
}

TextureConv_PCX::~TextureConv_PCX()
{
}

struct PCX_Header
{
	u8 Manufacturer;	
	u8 Version;
	u8 Encoding;
	u8 BitsPerPixel;	//4
	u16 XMin;
	u16 YMin;
	u16 XMax;
	u16 YMax;
	u16 VertDPI;	    //14
	u8 Palette[48];		//62
	u8 Reserved;		
	u8 ColorPlanes;		//64
	u16 BytesPerLine;	
	u16 PaletteType;
	u16 HScrSize;
	u16 VScrSize;		//72
	u8 Filler[56];		//128
};

bool TextureConv_PCX::ConvertTexture_Pal8(u8 *pConvertedData, s32& nOffsX, s32& nOffsY, u32& uWidth, u32& uHeight, const u8 *pSourceData, u32 uLen, const u8 *pPalette, bool bCopyPal, u32 uHackID/*=0*/)
{
	PCX_Header *pHeader = (PCX_Header *)pSourceData;
	const u8 *pImageData = &pSourceData[sizeof(PCX_Header)];

	assert(pHeader->BitsPerPixel == 8);
	assert(pHeader->Version == 5);

	uWidth  = (u32)pHeader->XMax - (u32)pHeader->XMin + 1;
	uHeight = (u32)pHeader->YMax - (u32)pHeader->YMin + 1;
	nOffsX  = 0;
	nOffsY  = 0;

	//for now just read the palette from the end of the file...
	u8 *pIndexData = (u8 *)ScratchPad::AllocMem( uWidth*uHeight );
	u32 destIdx=0;
	u32 uImgDataIdx = 0;
	u32 uPixelIdx = 0;
	while (uPixelIdx < uWidth*uHeight)
	{
		u8 uPixel = pImageData[uImgDataIdx];
		uImgDataIdx++;
		s32 nRunLength = 0;
		if ( (uPixel&64) && (uPixel&128) )
		{
			nRunLength = (s32)( uPixel&63 );
		}
		if ( nRunLength == 0 )
		{
			pIndexData[uPixelIdx] = uPixel;
			uPixelIdx++;
		}
		else
		{
			uPixel = pImageData[uImgDataIdx];
			uImgDataIdx++;
			while (nRunLength)
			{
				pIndexData[uPixelIdx] = uPixel;
				uPixelIdx++;

				nRunLength--;
			};
		}
	};
	assert( uPixelIdx == uWidth*uHeight );

	static u8 pal[768];
	assert( pImageData[uImgDataIdx] == 12 );
	memcpy(pal, &pImageData[uImgDataIdx+1], 768);

	if ( bCopyPal )
	{
		TextureLoader::SetPalette(0, pal, 768, 0);
	}

	for (u32 y=0; y<uHeight; y++)
	{
		for (u32 x=0; x<uWidth; x++)
		{
			u32 uPalIndex = pIndexData[y*uWidth+x]*3;
			assert( uPalIndex < 768*3 );

			pConvertedData[destIdx++] = uPalIndex == 0 ? 0 : pal[uPalIndex+0];
			pConvertedData[destIdx++] = uPalIndex == 0 ? 0 : pal[uPalIndex+1];
			pConvertedData[destIdx++] = uPalIndex == 0 ? 0 : pal[uPalIndex+2];
			pConvertedData[destIdx++] = uPalIndex == 0 ? 0x00 : 0xff;
		}
	}

	//dilation process for transparent pixels...
	u32 yIndex = 0;
	u32 w4 = uWidth*4;
	u32 uDilationPassCnt = 4;
	for (u32 p=0; p<uDilationPassCnt; p++)
	{
		u32 uTransCnt = 0;
		for (u32 y=0; y<uHeight; y++)
		{
			for (u32 x=0, x4=0; x<uWidth; x++, x4+=4)
			{
				if ( pConvertedData[yIndex + x4 + 3 ] == 0 )
				{
					//are any of the neighbors not transparent?
					if ( x < uWidth-1 && pConvertedData[yIndex + x4+4 + 3 ] > 0 )
					{
						pConvertedData[yIndex + x4 + 0 ] = pConvertedData[yIndex + x4+4 + 0 ];
						pConvertedData[yIndex + x4 + 1 ] = pConvertedData[yIndex + x4+4 + 1 ];
						pConvertedData[yIndex + x4 + 2 ] = pConvertedData[yIndex + x4+4 + 2 ];
					}
					else if ( x > 0 && pConvertedData[yIndex + x4-4 + 3 ] > 0 )
					{
						pConvertedData[yIndex + x4 + 0 ] = pConvertedData[yIndex + x4-4 + 0 ];
						pConvertedData[yIndex + x4 + 1 ] = pConvertedData[yIndex + x4-4 + 1 ];
						pConvertedData[yIndex + x4 + 2 ] = pConvertedData[yIndex + x4-4 + 2 ];
					}
					else if ( y < uHeight-1 && pConvertedData[yIndex+w4 + x4 + 3 ] > 0 )
					{
						pConvertedData[yIndex + x4 + 0 ] = pConvertedData[yIndex+w4 + x4 + 0 ];
						pConvertedData[yIndex + x4 + 1 ] = pConvertedData[yIndex+w4 + x4 + 1 ];
						pConvertedData[yIndex + x4 + 2 ] = pConvertedData[yIndex+w4 + x4 + 2 ];
					}
					else if ( y > 0 && pConvertedData[yIndex-w4 + x4 + 3 ] > 0 )
					{
						pConvertedData[yIndex + x4 + 0 ] = pConvertedData[yIndex-w4 + x4 + 0 ];
						pConvertedData[yIndex + x4 + 1 ] = pConvertedData[yIndex-w4 + x4 + 1 ];
						pConvertedData[yIndex + x4 + 2 ] = pConvertedData[yIndex-w4 + x4 + 2 ];
					}
					uTransCnt++;
				}
			}
			yIndex += uWidth*4;
		}
		if ( uTransCnt == 0 )
			break;
	}

	return true;
}

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
	uint8_t Manufacturer;
	uint8_t Version;
	uint8_t Encoding;
	uint8_t BitsPerPixel;	//4
	uint16_t XMin;
	uint16_t YMin;
	uint16_t XMax;
	uint16_t YMax;
	uint16_t VertDPI;	    //14
	uint8_t Palette[48];		//62
	uint8_t Reserved;
	uint8_t ColorPlanes;		//64
	uint16_t BytesPerLine;
	uint16_t PaletteType;
	uint16_t HScrSize;
	uint16_t VScrSize;		//72
	uint8_t Filler[56];		//128
};

bool TextureConv_PCX::ConvertTexture_Pal8(uint8_t *pConvertedData, int32_t& nOffsX, int32_t& nOffsY, uint32_t& uWidth, uint32_t& uHeight, const uint8_t *pSourceData, uint32_t uLen, const uint8_t *pPalette, bool bCopyPal, uint32_t uHackID/*=0*/)
{
	PCX_Header *pHeader = (PCX_Header *)pSourceData;
	const uint8_t *pImageData = &pSourceData[sizeof(PCX_Header)];

	assert(pHeader->BitsPerPixel == 8);
	assert(pHeader->Version == 5);

	uWidth  = (uint32_t)pHeader->XMax - (uint32_t)pHeader->XMin + 1;
	uHeight = (uint32_t)pHeader->YMax - (uint32_t)pHeader->YMin + 1;
	nOffsX  = 0;
	nOffsY  = 0;

	//for now just read the palette from the end of the file...
	uint8_t *pIndexData = (uint8_t *)ScratchPad::AllocMem( uWidth*uHeight );
	uint32_t destIdx=0;
	uint32_t uImgDataIdx = 0;
	uint32_t uPixelIdx = 0;
	while (uPixelIdx < uWidth*uHeight)
	{
		uint8_t uPixel = pImageData[uImgDataIdx];
		uImgDataIdx++;
		int32_t nRunLength = 0;
		if ( (uPixel&64) && (uPixel&128) )
		{
			nRunLength = (int32_t)( uPixel&63 );
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

	static uint8_t pal[768];
	assert( pImageData[uImgDataIdx] == 12 );
	memcpy(pal, &pImageData[uImgDataIdx+1], 768);

	if ( bCopyPal )
	{
		TextureLoader::SetPalette(0, pal, 768, 0);
	}

	for (uint32_t y=0; y<uHeight; y++)
	{
		for (uint32_t x=0; x<uWidth; x++)
		{
			uint32_t uPalIndex = pIndexData[y*uWidth+x]*3;
			assert( uPalIndex < 768*3 );

			pConvertedData[destIdx++] = uPalIndex == 0 ? 0 : pal[uPalIndex+0];
			pConvertedData[destIdx++] = uPalIndex == 0 ? 0 : pal[uPalIndex+1];
			pConvertedData[destIdx++] = uPalIndex == 0 ? 0 : pal[uPalIndex+2];
			pConvertedData[destIdx++] = uPalIndex == 0 ? 0x00 : 0xff;
		}
	}

	//dilation process for transparent pixels...
	uint32_t yIndex = 0;
	uint32_t w4 = uWidth*4;
	uint32_t uDilationPassCnt = 4;
	for (uint32_t p=0; p<uDilationPassCnt; p++)
	{
		uint32_t uTransCnt = 0;
		for (uint32_t y=0; y<uHeight; y++)
		{
			for (uint32_t x=0, x4=0; x<uWidth; x++, x4+=4)
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

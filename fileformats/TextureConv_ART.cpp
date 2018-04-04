#include "TextureConv_ART.h"
#include "ArchiveManager.h"

TextureConv_ART::TextureConv_ART() : TextureConverter()
{
}

TextureConv_ART::~TextureConv_ART()
{
}

bool TextureConv_ART::ConvertTexture_Pal8(u8 *pConvertedData, s32& nOffsX, s32& nOffsY, u32& uWidth, u32& uHeight, const u8 *pSourceData, u32 uLen, const u8 *pPalette, bool bCopyPal, u32 uHackID/*=0*/)
{
	u16 *pSizeInfo = (u16 *)ArchiveManager::GameFile_GetFileInfo();

	uWidth  = pSizeInfo[0];
	uHeight = pSizeInfo[1];
	nOffsX  = 0;
	nOffsY  = 0;

	u32 destIdx=0;
	for (u32 y=0; y<uHeight; y++)
	{
		for (u32 x=0; x<uWidth; x++)
		{
			//ART images are stored by columns rather then lines.
			u32 uPalIndex = (pSourceData[x*uHeight+y]<<2);

			pConvertedData[destIdx++] = pPalette[uPalIndex+0];
			pConvertedData[destIdx++] = pPalette[uPalIndex+1];
			pConvertedData[destIdx++] = pPalette[uPalIndex+2];
			pConvertedData[destIdx++] = pPalette[uPalIndex+3];
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

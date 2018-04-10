#include "TextureConv_ART.h"
#include "ArchiveManager.h"

TextureConv_ART::~TextureConv_ART()
{
}

bool TextureConv_ART::ConvertTexture_Pal8(uint8_t *pConvertedData, int32_t& nOffsX, int32_t& nOffsY, uint32_t& uWidth, uint32_t& uHeight, const uint8_t *pSourceData, uint32_t uLen, const uint8_t *pPalette, bool bCopyPal, uint32_t uHackID/*=0*/)
{
    uint16_t *pSizeInfo = (uint16_t *)ArchiveManager::GameFile_GetFileInfo();

    uWidth  = pSizeInfo[0];
    uHeight = pSizeInfo[1];
    nOffsX  = 0;
    nOffsY  = 0;

    uint32_t destIdx=0;
    for (uint32_t y=0; y<uHeight; y++)
    {
        for (uint32_t x=0; x<uWidth; x++)
        {
            //ART images are stored by columns rather then lines.
            uint32_t uPalIndex = (pSourceData[x*uHeight+y]<<2);

            pConvertedData[destIdx++] = pPalette[uPalIndex+0];
            pConvertedData[destIdx++] = pPalette[uPalIndex+1];
            pConvertedData[destIdx++] = pPalette[uPalIndex+2];
            pConvertedData[destIdx++] = pPalette[uPalIndex+3];
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

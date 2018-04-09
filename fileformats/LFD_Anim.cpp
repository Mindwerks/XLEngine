#include "LFD_Anim.h"
#include "../math/Math.h"
#include "../render/IDriver3D.h"
#include "../render/TextureCache.h"
#include <cmath>
#include <cstring>
#include <cassert>

PLTT_File LFD_Anim::m_PalFile;
uint32_t _ImgBuffer[2048*2048];

LFD_Anim::LFD_Anim(IDriver3D *pDriver)
{
    m_nNumDelts = 0;

    memset(m_hTex, 0, sizeof(TextureHandle)*256);

    m_fScaleX = 1.0f;
    m_fScaleY = 1.0f;

    m_pDriver = pDriver;
}

LFD_Anim::~LFD_Anim(void)
{
    Destroy();
}

void LFD_Anim::Destroy()
{
    for (int32_t i=0; i<m_nNumDelts; i++)
    {
        TextureCache::FreeTexture( m_hTex[i] );
    }
    m_nNumDelts = 0;
}

bool LFD_Anim::LoadPLTT(char *pData, int len)
{
    m_PalFile.First = *((uint8_t *)&pData[0]);
    m_PalFile.Last  = *((uint8_t *)&pData[1]);
    m_PalFile.num_colors = m_PalFile.Last - m_PalFile.First + 1;
    memset(m_PalFile.colors, 0, 256*3);
    memcpy(m_PalFile.colors, &pData[2], m_PalFile.num_colors*3);

    return true;
}

bool LFD_Anim::SetPLTT(PLTT_File *pal, bool bCopyFullPal/*=true*/)
{
    if ( bCopyFullPal )
    {
        memcpy(&m_PalFile, pal, sizeof(PLTT_File));
    }
    else
    {
        m_PalFile.First = 0; m_PalFile.Last = 255;
        memcpy(&m_PalFile.colors[pal->First], pal->colors, ( (pal->Last-pal->First)+1 )*sizeof(RGB_Color));
    }
    return true;
}

PLTT_File *LFD_Anim::GetPLTT()
{
    return &m_PalFile;
}

bool LFD_Anim::LoadDELT(char *pData, int32_t len, bool bUseProperOffs)
{
    int32_t nIdx=0, pIdx;
    int32_t Offs[2], Size[2], SizeAndType, StartX, StartY;
    uint32_t texSize[2];
    int32_t num_pixels, count, i=0;
    bool bRLE;
    uint8_t pixel;

    m_nNumDelts = 1;
    int32_t size = len;

    Offs[0] = *((int16_t *)&pData[nIdx]); nIdx+=2;
    Offs[1] = *((int16_t *)&pData[nIdx]); nIdx+=2;
    Size[0] = *((int16_t *)&pData[nIdx]); nIdx+=2;
    Size[1] = *((int16_t *)&pData[nIdx]); nIdx+=2;
    size -= 8;

    Size[0]++;
    Size[1]++;

    m_Width[i]  = (float)Size[0]/320.0f;
    m_Height[i] = (float)Size[1]/200.0f;
    if ( bUseProperOffs )
    {
        m_OffsX[i] = (float)Offs[0]/320.0f;
        m_OffsY[i] = /*1.0f - m_Height[i] - */(float)Offs[1]/200.0f;
    }
    else
    {
        m_OffsX[i]  = 0.0f;
        m_OffsY[i]  = ((float)Offs[1]*0.125f*0.45f/200.0f);
    }

    memset(_ImgBuffer, 0, 2048*2048);
    assert( Size[0]*Size[1] <= 2048*2048 );

    int32_t y=0;
    while (size > 6)
    {
        SizeAndType = *((int16_t *)&pData[nIdx]); nIdx+=2;
        StartX = *((int16_t *)&pData[nIdx]); nIdx+=2;
        StartY = *((int16_t *)&pData[nIdx]); nIdx+=2;
        size -= 6;

        if ( bUseProperOffs )
        {
            StartX -= Offs[0];
            StartY -= Offs[1];
        }

        pIdx = StartX + StartY*Size[0];

        num_pixels = (SizeAndType>>1)&0x3FFF;
        bRLE = (SizeAndType&1) ? true : false;

        uint8_t num_bytes = 0;
        int32_t offs;
        if ( 0 )//!bRLE )   --? I should look into this
        {
            num_bytes = (uint8_t)pData[nIdx]; nIdx++;
            if ( num_bytes > num_pixels )
            {
                offs = num_bytes - num_pixels;
            }
            size--;
        }
        uint8_t *pImgData = (uint8_t *)pData;
        while (num_pixels > 0)
        {
            if ( bRLE )
            {
                //read count byte...
                count = pImgData[nIdx]; 
                nIdx++; size--;
                if ( !(count&1) ) //direct
                {
                    count >>= 1;
                    for (int32_t p=0; p<count; p++)
                    {
                        pixel = pImgData[nIdx]; nIdx++;
                        _ImgBuffer[pIdx++] = (0xff<<24) | (m_PalFile.colors[pixel].b<<16) | (m_PalFile.colors[pixel].g<<8) | m_PalFile.colors[pixel].r;
                        size--;
                    }
                    num_pixels -= count;
                }
                else    //rle
                {
                    count >>= 1;
                    pixel = pImgData[nIdx]; nIdx++;
                    size--;
                    //copy all the pixels over...
                    for (int32_t p=0; p<count; p++)
                    {
                        _ImgBuffer[pIdx++] = (0xff<<24) | (m_PalFile.colors[pixel].b<<16) | (m_PalFile.colors[pixel].g<<8) | m_PalFile.colors[pixel].r;
                    }
                    num_pixels -= count;
                }
            }
            else
            {
                for (int32_t p=0; p<num_pixels; p++)
                {
                    pixel = pImgData[nIdx]; nIdx++;
                    _ImgBuffer[pIdx++] = (0xff<<24) | (m_PalFile.colors[pixel].b<<16) | (m_PalFile.colors[pixel].g<<8) | m_PalFile.colors[pixel].r;
                    size--;
                }
                num_pixels = 0;
            }
        };
        /*
        if ( !bRLE )    //--? Weird again.
        {
            if ( offs > 0 )
            {
                //nIdx += offs;
                size -= offs;
            }
        }
        */
    };
    //Do the RGBA texture creation...
    texSize[0] = (uint32_t)Size[0];
    texSize[1] = (uint32_t)Size[1];
    m_hTex[i] = TextureCache::LoadTextureFromMem((uint8_t *)_ImgBuffer, texSize[0], texSize[1], false);
    int32_t nOffsX, nOffsY;
    TextureCache::GetTextureSize(nOffsX, nOffsY, texSize[0], texSize[1], m_u1[i], m_v1[i]);

    return true;
}

bool LFD_Anim::Load(char *pData, int32_t len, bool bUseProperOffs)
{   
    int32_t nIdx=0, pIdx;
    m_nNumDelts = *((uint16_t *)pData);
    assert( m_nNumDelts < 256 );
    nIdx+=2;
    int32_t Offs[2], Size[2], SizeAndType, StartX, StartY;
    uint32_t texSize[2];
    int32_t num_pixels, count;
    bool bRLE;
    uint8_t pixel;
    for (int32_t i=0; i<m_nNumDelts; i++)
    {
        int32_t size = *((int32_t *)&pData[nIdx]);
        nIdx+=4;
        Offs[0] = *((int16_t *)&pData[nIdx]); nIdx+=2;
        Offs[1] = *((int16_t *)&pData[nIdx]); nIdx+=2;
        Size[0] = *((int16_t *)&pData[nIdx]); nIdx+=2;
        Size[1] = *((int16_t *)&pData[nIdx]); nIdx+=2;
        size -= 8;

        Size[0]++;
        Size[1]++;

        m_Width[i]  = (float)Size[0]/320.0f;
        m_Height[i] = (float)Size[1]/200.0f;
        if ( bUseProperOffs )
        {
            m_OffsX[i] = (float)Offs[0]/320.0f;
            m_OffsY[i] = /*1.0f - m_Height[i] - */(float)Offs[1]/200.0f;
        }
        else
        {
            m_OffsX[i]  = 0.0f;
            m_OffsY[i]  = ((float)Offs[1]*0.125f*0.45f/200.0f);
        }

        memset(_ImgBuffer, 0, 2048*2048);
        float afAveClr[3]={0.0f, 0.0f, 0.0f};
        int32_t nTotalVisPixels=0;

        while (size > 6)
        {
            SizeAndType = *((int16_t *)&pData[nIdx]); nIdx+=2;
            StartX = *((int16_t *)&pData[nIdx]); nIdx+=2;
            StartY = *((int16_t *)&pData[nIdx]); nIdx+=2;
            size -= 6;

            if ( bUseProperOffs )
            {
                StartX -= Offs[0];
                StartY -= Offs[1];
            }

            pIdx = StartX + StartY*Size[0];

            num_pixels = (SizeAndType>>1)&0x3FFF;
            bRLE = (SizeAndType&1) ? true : false;

            uint8_t num_bytes = 0;
            int32_t offs;
            if ( 0 )//!bRLE )
            {
                num_bytes = (uint8_t)pData[nIdx]; nIdx++;
                if ( num_bytes > num_pixels )
                {
                    offs = num_bytes - num_pixels;
                }
                size--;
            }
            uint8_t *pImgData = (uint8_t *)pData;
            while (num_pixels > 0)
            {
                if ( bRLE )
                {
                    //read count byte...
                    count = pImgData[nIdx]; 
                    nIdx++; size--;
                    if ( !(count&1) ) //direct
                    {
                        count >>= 1;
                        for (int p=0; p<count; p++)
                        {
                            pixel = pImgData[nIdx]; nIdx++;
                            _ImgBuffer[pIdx++] = (0xff<<24) | (m_PalFile.colors[pixel].b<<16) | (m_PalFile.colors[pixel].g<<8) | m_PalFile.colors[pixel].r;
                            afAveClr[0] += (float)m_PalFile.colors[pixel].r;
                            afAveClr[1] += (float)m_PalFile.colors[pixel].g;
                            afAveClr[2] += (float)m_PalFile.colors[pixel].b;
                            nTotalVisPixels++;
                            size--;
                        }
                        num_pixels -= count;
                    }
                    else    //rle
                    {
                        count >>= 1;
                        pixel = pImgData[nIdx]; nIdx++;
                        size--;
                        //copy all the pixels over...
                        for (int32_t p=0; p<count; p++)
                        {
                            _ImgBuffer[pIdx++] = (0xff<<24) | (m_PalFile.colors[pixel].b<<16) | (m_PalFile.colors[pixel].g<<8) | m_PalFile.colors[pixel].r;
                            afAveClr[0] += (float)m_PalFile.colors[pixel].r;
                            afAveClr[1] += (float)m_PalFile.colors[pixel].g;
                            afAveClr[2] += (float)m_PalFile.colors[pixel].b;
                            nTotalVisPixels++;
                        }
                        num_pixels -= count;
                    }
                }
                else
                {
                    for (int32_t p=0; p<num_pixels; p++)
                    {
                        pixel = pImgData[nIdx]; nIdx++;
                        _ImgBuffer[pIdx++] = (0xff<<24) | (m_PalFile.colors[pixel].b<<16) | (m_PalFile.colors[pixel].g<<8) | m_PalFile.colors[pixel].r;
                        afAveClr[0] += (float)m_PalFile.colors[pixel].r;
                        afAveClr[1] += (float)m_PalFile.colors[pixel].g;
                        afAveClr[2] += (float)m_PalFile.colors[pixel].b;
                        nTotalVisPixels++;
                        size--;
                    }
                    num_pixels = 0;
                }
            };
        };

        if ( bUseProperOffs && 0 )
        {
            float fOOTPix;
            fOOTPix = (nTotalVisPixels>0) ? 1.0f / (float)nTotalVisPixels : 0.0f;
            afAveClr[0] *= fOOTPix;
            afAveClr[1] *= fOOTPix;
            afAveClr[2] *= fOOTPix;

            int32_t yOp=0, yOn;
            for (int32_t y=0, yOffs=0; y<Size[1]; y++, yOffs+=Size[0])
            {
                yOn = yOffs + Size[0];
                for (int32_t x=0; x<Size[0]; x++)
                {
                    if ( _ImgBuffer[x+yOffs] == 0x00000000  )   //spread to nearby pixels...
                    {
                        _ImgBuffer[x+yOffs] = (0x00<<24) | (((int32_t)afAveClr[0])<<16) | (((int32_t)afAveClr[1])<<8) | ((int32_t)afAveClr[2]);
                    }
                }
            }
        }

        if ( size > 0 ) { nIdx+=size; }
        //preprocess the image buffer, we want to "bleed" out pixels into black regions...
        int32_t num_bleed_pass = 1;
        int32_t yOp, yOn;
        for (int32_t b=0; b<num_bleed_pass; b++)
        {
            uint32_t uMin = (b==0)?0xff000000 : 0x00000000;
            yOp = 0;
            for (int32_t y=0, yOffs=0; y<Size[1]; y++, yOffs+=Size[0])
            {
                yOn = yOffs + Size[0];
                for (int32_t x=0; x<Size[0]; x++)
                {
                    if ( _ImgBuffer[x+yOffs] > uMin  )  //spread to nearby pixels...
                    {
                        if ( x > 0 )
                        {
                            if ( _ImgBuffer[x-1+yOffs] == 0x00000000 )  
                            { 
                                _ImgBuffer[x-1+yOffs] = _ImgBuffer[x+yOffs]; 
                                if (_ImgBuffer[x-1+yOffs]>0xff000000) _ImgBuffer[x-1+yOffs] -= 0xff000000; 
                            }
                        }
                        if ( x < Size[0]-1 )
                        {
                            if ( _ImgBuffer[x+1+yOffs] == 0x00000000 )
                            { 
                                _ImgBuffer[x+1+yOffs] = _ImgBuffer[x+yOffs]; 
                                if (_ImgBuffer[x+1+yOffs]>0xff000000) _ImgBuffer[x+1+yOffs] -= 0xff000000; 
                            }
                        }
                        if ( y > 0 )
                        {
                            if ( _ImgBuffer[x+yOp] == 0x00000000 )
                            { 
                                _ImgBuffer[x+yOp] = _ImgBuffer[x+yOffs]; 
                                if (_ImgBuffer[x+yOp]>0xff000000) _ImgBuffer[x+yOp] -= 0xff000000; 
                            }
                        }
                        if ( y < Size[1]-1 )
                        {
                            if ( _ImgBuffer[x+yOn] == 0x00000000 )
                            { 
                                _ImgBuffer[x+yOn] = _ImgBuffer[x+yOffs]; 
                                if (_ImgBuffer[x+yOn]>0xff000000) _ImgBuffer[x+yOn] -= 0xff000000; 
                            }
                        }
                    }
                }
                yOp = yOffs;
            }
        }

        //Do the RGBA texture creation...
        texSize[0] = (uint32_t)Size[0];
        texSize[1] = (uint32_t)Size[1];
        m_hTex[i] = TextureCache::LoadTextureFromMem((uint8_t *)_ImgBuffer, texSize[0], texSize[1], false);
        int32_t nOffsX, nOffsY;
        TextureCache::GetTextureSize(nOffsX, nOffsY, texSize[0], texSize[1], m_u1[i], m_v1[i]);
    }
    return true;
}

void LFD_Anim::SetOffsScale(float sx, float sy)
{
    for (int32_t i=0; i<m_nNumDelts; i++)
    {
        m_OffsX[i] *= sx;
        if ( sy != 1.0f )
        {
            m_OffsY[i] = m_OffsY[i]*sy;
        }
    }
}

void LFD_Anim::GetFrameExtents(int32_t frame, float x, float y, int32_t& frameX0, int32_t& frameY0, int32_t& frameWidth, int32_t& frameHeight)
{
    if ( frame > m_nNumDelts-1 ) { frame = m_nNumDelts-1; }

    //ok, we need to figure out the scale and offset.
    int32_t nWidth, nHeight;
    m_pDriver->GetWindowSize(nWidth, nHeight);
    //now we want to impose the 320x200 UI into the 4:3 size.
    int32_t n43_Width = 4*nHeight/3;
    float fOffsetX  = (float)( (nWidth - n43_Width)>>1 );
    float fOffsetY  = 0.0f;
    float fScaleX   = (float)n43_Width;
    float fScaleY   = (float)nHeight;

    frameX0 = (int32_t)( (m_OffsX[frame]+x)*fScaleX+fOffsetX );
    frameY0 = (int32_t)( (m_OffsY[frame]+y)*fScaleY+fOffsetY );

    frameWidth  = (int32_t)( m_Width[frame]*fScaleX*m_fScaleX );
    frameHeight = (int32_t)( m_Height[frame]*fScaleY*m_fScaleY );
}

void LFD_Anim::Render(int32_t frame, float x, float y, float maxX, float minY, float dU, float dV, bool bDistort)
{
    if ( frame > m_nNumDelts-1 ) { frame = m_nNumDelts-1; }

    m_pDriver->SetTexture(0, m_hTex[frame], IDriver3D::FILTER_POINT);
    if ( bDistort )
    {
        //m_pDriver->SetShaders(Driver3D_DX9::VS_SHADER_SCREEN_PROJ, Driver3D_DX9::PS_SHADER_SCREEN);
        //hacky adjustment for distortion - but its hacked in DF too...
        //y = y*0.28f - 1.0f;
        //m_pDriver->SetFilterMode(FILTERMODE_NORMAL);

        //y = -y;
    }

    //ok, we need to figure out the scale and offset.
    int32_t nWidth, nHeight;
    m_pDriver->GetWindowSize(nWidth, nHeight);
    //now we want to impose the 320x200 UI into the 4:3 size.
    int32_t n43_Width = 4*nHeight/3;
    float fOffsetX = (float)( (nWidth - n43_Width)>>1 );
    float fOffsetY = 0.0f;
    float fScaleX  = (float)n43_Width;
    float fScaleY  = (float)nHeight;

    float x0, y0, x1, y1, u0, u1, v0, v1;
    x0 = (m_OffsX[frame]+x)*fScaleX+fOffsetX; x1 = m_Width[frame] *m_fScaleX*fScaleX+x0;
    y0 = (m_OffsY[frame]+y)*fScaleY+fOffsetY; y1 = m_Height[frame]*m_fScaleY*fScaleY+y0;
    u0 = 0.0f; u1 = m_u1[frame];
    v0 = 0.0f; v1 = m_v1[frame];

    /*if ( minY > y0 )
    {
        float s = (minY - y0) / (y1 - y0);
        v1 = v1 + s*(v0-v1);
        y0 = minY;
    }*/

    u0 += dU; u1 += dU;
    v0 += dV; v1 += dV;

    Vector4 color(1,1,1,1);
    Vector4 posScale(x0, y0, m_Width[frame]*fScaleX*m_fScaleX, m_Height[frame]*fScaleY*m_fScaleY);
    Vector2 uvTop(u0, v0), uvBot(u1, v1);

    if ( bDistort )
    {
        /*polygon[0].Set(x0+0.05f, y1, y1*3.0f);
        polygon[1].Set(x1+0.05f, y1, y1*3.0f);
        polygon[2].Set(x1+0.05f, y0, y0*3.0f);
        polygon[3].Set(x0+0.05f, y0, y0*3.0f);
        */
        //Distorted quad.
        m_pDriver->RenderScreenQuad(posScale, uvTop, uvBot, color, color);
    }
    else
    {
        m_pDriver->RenderScreenQuad(posScale, uvTop, uvBot, color, color);
    }
}

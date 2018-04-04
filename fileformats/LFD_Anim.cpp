#include "LFD_Anim.h"
#include "../math/Math.h"
#include "../render/IDriver3D.h"
#include "../render/TextureCache.h"
#include <math.h>
#include <string.h>
#include <assert.h>

PLTT_File LFD_Anim::m_PalFile;
u32 _ImgBuffer[2048*2048];

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
	for (s32 i=0; i<m_nNumDelts; i++)
	{
		TextureCache::FreeTexture( m_hTex[i] );
	}
	m_nNumDelts = 0;
}

bool LFD_Anim::LoadPLTT(char *pData, int len)
{
	m_PalFile.First = *((u8 *)&pData[0]);
	m_PalFile.Last  = *((u8 *)&pData[1]);
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

bool LFD_Anim::LoadDELT(char *pData, s32 len, bool bUseProperOffs)
{
	s32 nIdx=0, pIdx;
	s32 Offs[2], Size[2], SizeAndType, StartX, StartY;
	u32 texSize[2];
	s32 num_pixels, count, i=0;
	bool bRLE;
	u8 pixel;

	m_nNumDelts = 1;
	s32 size = len;

	Offs[0] = *((s16 *)&pData[nIdx]); nIdx+=2;
	Offs[1] = *((s16 *)&pData[nIdx]); nIdx+=2;
	Size[0] = *((s16 *)&pData[nIdx]); nIdx+=2;
	Size[1] = *((s16 *)&pData[nIdx]); nIdx+=2;
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

	s32 y=0;
	while (size > 6)
	{
		SizeAndType = *((s16 *)&pData[nIdx]); nIdx+=2;
		StartX = *((s16 *)&pData[nIdx]); nIdx+=2;
		StartY = *((s16 *)&pData[nIdx]); nIdx+=2;
		size -= 6;

		if ( bUseProperOffs )
		{
			StartX -= Offs[0];
			StartY -= Offs[1];
		}

		pIdx = StartX + StartY*Size[0];

		num_pixels = (SizeAndType>>1)&0x3FFF;
		bRLE = (SizeAndType&1) ? true : false;

		u8 num_bytes = 0;
		s32 offs;
		if ( 0 )//!bRLE )	--? I should look into this
		{
			num_bytes = (u8)pData[nIdx]; nIdx++;
			if ( num_bytes > num_pixels )
			{
				offs = num_bytes - num_pixels;
			}
			size--;
		}
		u8 *pImgData = (u8 *)pData;
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
					for (s32 p=0; p<count; p++)
					{
						pixel = pImgData[nIdx]; nIdx++;
						_ImgBuffer[pIdx++] = (0xff<<24) | (m_PalFile.colors[pixel].b<<16) | (m_PalFile.colors[pixel].g<<8) | m_PalFile.colors[pixel].r;
						size--;
					}
					num_pixels -= count;
				}
				else	//rle
				{
					count >>= 1;
					pixel = pImgData[nIdx]; nIdx++;
					size--;
					//copy all the pixels over...
					for (s32 p=0; p<count; p++)
					{
						_ImgBuffer[pIdx++] = (0xff<<24) | (m_PalFile.colors[pixel].b<<16) | (m_PalFile.colors[pixel].g<<8) | m_PalFile.colors[pixel].r;
					}
					num_pixels -= count;
				}
			}
			else
			{
				for (s32 p=0; p<num_pixels; p++)
				{
					pixel = pImgData[nIdx]; nIdx++;
					_ImgBuffer[pIdx++] = (0xff<<24) | (m_PalFile.colors[pixel].b<<16) | (m_PalFile.colors[pixel].g<<8) | m_PalFile.colors[pixel].r;
					size--;
				}
				num_pixels = 0;
			}
		};
		/*
		if ( !bRLE )	//--? Weird again.
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
	texSize[0] = (u32)Size[0];
	texSize[1] = (u32)Size[1];
	m_hTex[i] = TextureCache::LoadTextureFromMem((u8 *)_ImgBuffer, texSize[0], texSize[1], false);
	s32 nOffsX, nOffsY;
	TextureCache::GetTextureSize(nOffsX, nOffsY, texSize[0], texSize[1], m_u1[i], m_v1[i]);

	return true;
}

bool LFD_Anim::Load(char *pData, s32 len, bool bUseProperOffs)
{	
	s32 nIdx=0, pIdx;
	m_nNumDelts = *((u16 *)pData);
	assert( m_nNumDelts < 256 );
	nIdx+=2;
	s32 Offs[2], Size[2], SizeAndType, StartX, StartY;
	u32 texSize[2];
	s32 num_pixels, count;
	bool bRLE;
	u8 pixel;
	for (s32 i=0; i<m_nNumDelts; i++)
	{
		s32 size = *((s32 *)&pData[nIdx]);
		nIdx+=4;
		Offs[0] = *((s16 *)&pData[nIdx]); nIdx+=2;
		Offs[1] = *((s16 *)&pData[nIdx]); nIdx+=2;
		Size[0] = *((s16 *)&pData[nIdx]); nIdx+=2;
		Size[1] = *((s16 *)&pData[nIdx]); nIdx+=2;
		size -= 8;

		Size[0]++;
		Size[1]++;

		m_Width[i]  = (f32)Size[0]/320.0f;
		m_Height[i] = (f32)Size[1]/200.0f;
		if ( bUseProperOffs )
		{
			m_OffsX[i] = (f32)Offs[0]/320.0f;
			m_OffsY[i] = /*1.0f - m_Height[i] - */(f32)Offs[1]/200.0f;
		}
		else
		{
			m_OffsX[i]  = 0.0f;
			m_OffsY[i]  = ((f32)Offs[1]*0.125f*0.45f/200.0f);
		}

		memset(_ImgBuffer, 0, 2048*2048);
		f32 afAveClr[3]={0.0f, 0.0f, 0.0f};
		s32 nTotalVisPixels=0;

		while (size > 6)
		{
			SizeAndType = *((s16 *)&pData[nIdx]); nIdx+=2;
			StartX = *((s16 *)&pData[nIdx]); nIdx+=2;
			StartY = *((s16 *)&pData[nIdx]); nIdx+=2;
			size -= 6;

			if ( bUseProperOffs )
			{
				StartX -= Offs[0];
				StartY -= Offs[1];
			}

			pIdx = StartX + StartY*Size[0];

			num_pixels = (SizeAndType>>1)&0x3FFF;
			bRLE = (SizeAndType&1) ? true : false;

			u8 num_bytes = 0;
			s32 offs;
			if ( 0 )//!bRLE )
			{
				num_bytes = (u8)pData[nIdx]; nIdx++;
				if ( num_bytes > num_pixels )
				{
					offs = num_bytes - num_pixels;
				}
				size--;
			}
			u8 *pImgData = (u8 *)pData;
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
							afAveClr[0] += (f32)m_PalFile.colors[pixel].r;
							afAveClr[1] += (f32)m_PalFile.colors[pixel].g;
							afAveClr[2] += (f32)m_PalFile.colors[pixel].b;
							nTotalVisPixels++;
							size--;
						}
						num_pixels -= count;
					}
					else	//rle
					{
						count >>= 1;
						pixel = pImgData[nIdx]; nIdx++;
						size--;
						//copy all the pixels over...
						for (s32 p=0; p<count; p++)
						{
							_ImgBuffer[pIdx++] = (0xff<<24) | (m_PalFile.colors[pixel].b<<16) | (m_PalFile.colors[pixel].g<<8) | m_PalFile.colors[pixel].r;
							afAveClr[0] += (f32)m_PalFile.colors[pixel].r;
							afAveClr[1] += (f32)m_PalFile.colors[pixel].g;
							afAveClr[2] += (f32)m_PalFile.colors[pixel].b;
							nTotalVisPixels++;
						}
						num_pixels -= count;
					}
				}
				else
				{
					for (s32 p=0; p<num_pixels; p++)
					{
						pixel = pImgData[nIdx]; nIdx++;
						_ImgBuffer[pIdx++] = (0xff<<24) | (m_PalFile.colors[pixel].b<<16) | (m_PalFile.colors[pixel].g<<8) | m_PalFile.colors[pixel].r;
						afAveClr[0] += (f32)m_PalFile.colors[pixel].r;
						afAveClr[1] += (f32)m_PalFile.colors[pixel].g;
						afAveClr[2] += (f32)m_PalFile.colors[pixel].b;
						nTotalVisPixels++;
						size--;
					}
					num_pixels = 0;
				}
			};
		};

		if ( bUseProperOffs && 0 )
		{
			f32 fOOTPix;
			fOOTPix = (nTotalVisPixels>0) ? 1.0f / (f32)nTotalVisPixels : 0.0f;
			afAveClr[0] *= fOOTPix;
			afAveClr[1] *= fOOTPix;
			afAveClr[2] *= fOOTPix;

			s32 yOp=0, yOn;
			for (s32 y=0, yOffs=0; y<Size[1]; y++, yOffs+=Size[0])
			{
				yOn = yOffs + Size[0];
				for (s32 x=0; x<Size[0]; x++)
				{
					if ( _ImgBuffer[x+yOffs] == 0x00000000  )	//spread to nearby pixels...
					{
						_ImgBuffer[x+yOffs] = (0x00<<24) | (((s32)afAveClr[0])<<16) | (((s32)afAveClr[1])<<8) | ((s32)afAveClr[2]);
					}
				}
			}
		}

		if ( size > 0 ) { nIdx+=size; }
		//preprocess the image buffer, we want to "bleed" out pixels into black regions...
		s32 num_bleed_pass = 1;
		s32 yOp, yOn;
		for (s32 b=0; b<num_bleed_pass; b++)
		{
			u32 uMin = (b==0)?0xff000000 : 0x00000000;
			yOp = 0;
			for (s32 y=0, yOffs=0; y<Size[1]; y++, yOffs+=Size[0])
			{
				yOn = yOffs + Size[0];
				for (s32 x=0; x<Size[0]; x++)
				{
					if ( _ImgBuffer[x+yOffs] > uMin  )	//spread to nearby pixels...
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
		texSize[0] = (u32)Size[0];
		texSize[1] = (u32)Size[1];
		m_hTex[i] = TextureCache::LoadTextureFromMem((u8 *)_ImgBuffer, texSize[0], texSize[1], false);
		s32 nOffsX, nOffsY;
		TextureCache::GetTextureSize(nOffsX, nOffsY, texSize[0], texSize[1], m_u1[i], m_v1[i]);
	}
	return true;
}

void LFD_Anim::SetOffsScale(f32 sx, f32 sy)
{
	for (s32 i=0; i<m_nNumDelts; i++)
	{
		m_OffsX[i] *= sx;
		if ( sy != 1.0f )
		{
			m_OffsY[i] = m_OffsY[i]*sy;
		}
	}
}

void LFD_Anim::GetFrameExtents(s32 frame, f32 x, f32 y, s32& frameX0, s32& frameY0, s32& frameWidth, s32& frameHeight)
{
	if ( frame > m_nNumDelts-1 ) { frame = m_nNumDelts-1; }

	//ok, we need to figure out the scale and offset.
	s32 nWidth, nHeight;
	m_pDriver->GetWindowSize(nWidth, nHeight);
	//now we want to impose the 320x200 UI into the 4:3 size.
	s32 n43_Width = 4*nHeight/3;
	f32 fOffsetX  = (f32)( (nWidth - n43_Width)>>1 );
	f32 fOffsetY  = 0.0f;
	f32 fScaleX   = (f32)n43_Width;
	f32 fScaleY   = (f32)nHeight;

	frameX0 = (s32)( (m_OffsX[frame]+x)*fScaleX+fOffsetX );
	frameY0 = (s32)( (m_OffsY[frame]+y)*fScaleY+fOffsetY );

	frameWidth  = (s32)( m_Width[frame]*fScaleX*m_fScaleX );
	frameHeight = (s32)( m_Height[frame]*fScaleY*m_fScaleY );
}

void LFD_Anim::Render(s32 frame, f32 x, f32 y, f32 maxX, f32 minY, f32 dU, f32 dV, bool bDistort)
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
	s32 nWidth, nHeight;
	m_pDriver->GetWindowSize(nWidth, nHeight);
	//now we want to impose the 320x200 UI into the 4:3 size.
	s32 n43_Width = 4*nHeight/3;
	f32 fOffsetX = (f32)( (nWidth - n43_Width)>>1 );
	f32 fOffsetY = 0.0f;
	f32 fScaleX  = (f32)n43_Width;
	f32 fScaleY  = (f32)nHeight;

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

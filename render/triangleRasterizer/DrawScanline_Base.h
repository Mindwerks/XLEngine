//DrawScanline FUNCTION BODY
//Note defines are used to enable or disable various features for current version of the function.
{
	int XStart = pLeft->X;
	int Width  = pRight->X - XStart;

//on all the time for the moment...
#ifdef fogLighting
	#undef fogLighting
#endif
#if gouraud == 0
	#define fogLighting 1
#else
	#define fogLighting 0
#endif

#if bpp8 == 1
	u8 *pDestBits = _pFrameBuffer_8;

	u32 uOffsTex = 0;
	u32 tw = _pCurTex->m_nWidth;
	u32 th = _pCurTex->m_nHeight;
	for (s32 l=0; l<_nMip; l++)
	{
		uOffsTex += (tw*th);
		tw >>= 1;
		th >>= 1;
	}

	u32 *pTexFrame = _pCurTex->m_pData[_uCurFrame];
	u8 *pTextureBits = (u8 *)&pTexFrame[uOffsTex>>2];
	u8 *pColormapFog = TextureLoader::GetColormapData(_uColormapID);
	u8 *pColormapLit = TextureLoader::GetColormapData(0);
#else
	u32 *pDestBits = _pFrameBuffer_32;
	u32 uOffsTex = 0;
	u32 tw = _pCurTex->m_nWidth;
	u32 th = _pCurTex->m_nHeight;
	for (s32 l=0; l<_nMip; l++)
	{
		uOffsTex += (tw*th);
		tw >>= 1;
		th >>= 1;
	}
	u32 *pTexFrame = _pCurTex->m_pData[_uCurFrame];
	u8 *pTextureBits = (u8 *)&pTexFrame[uOffsTex];
#endif
	u16 *pDepthBuffer = _pDepthBuffer;
	pDestBits += pLeft->Y * _nFrameWidth + XStart;
	pDepthBuffer += pLeft->Y * _nFrameWidth + XStart;
	s32 TextureDeltaScan = _pCurTex->m_nWidth>>_nMip;

	float OneOverZLeft = pLeft->OneOverZ;
	float UOverZLeft   = pLeft->UOverZ;
	float VOverZLeft   = pLeft->VOverZ;

	float dOneOverZdXAff = Gradients.dOneOverZdX * _nAffineLength;
	float dUOverZdXAff = Gradients.dUOverZdX * _nAffineLength;
	float dVOverZdXAff = Gradients.dVOverZdX * _nAffineLength;

	float OneOverZRight = OneOverZLeft + dOneOverZdXAff;
	float UOverZRight   = UOverZLeft + dUOverZdXAff;
	float VOverZRight   = VOverZLeft + dVOverZdXAff;

	float ZLeft = 1.0f / OneOverZLeft;
	float ULeft = ZLeft * UOverZLeft;
	float VLeft = ZLeft * VOverZLeft;

	//Lighting...
#if gouraud == 1
	float gOverZLeft  = pLeft->gOverZ;
	float dgOverZdXAff = Gradients.dgOverZdX * _nAffineLength;
	float gOverZRight  = gOverZLeft + dgOverZdXAff;
	float gLeft = ZLeft * gOverZLeft;
#else
	float LxOverZLeft  = pLeft->LxOverZ;
	float LyOverZLeft  = pLeft->LyOverZ;
	float LzOverZLeft  = pLeft->LzOverZ;

	float dLxOverZdXAff = Gradients.dLxOverZdX * _nAffineLength;
	float dLyOverZdXAff = Gradients.dLyOverZdX * _nAffineLength;
	float dLzOverZdXAff = Gradients.dLzOverZdX * _nAffineLength;

	float LxOverZRight  = LxOverZLeft + dLxOverZdXAff;
	float LyOverZRight  = LyOverZLeft + dLyOverZdXAff;
	float LzOverZRight  = LzOverZLeft + dLzOverZdXAff;

	float LxLeft = ZLeft * LxOverZLeft;
	float LyLeft = ZLeft * LyOverZLeft;
	float LzLeft = ZLeft * LzOverZLeft;
#endif
	//
	float ZRight, URight, VRight;
	fixed16_16 U, V, DeltaU=0, DeltaV=0;
	fixed16_16 DeltaLx=0, DeltaLy=0, DeltaLz=0;

#if pow2 == 1
	int tu_wrap = (_pCurTex->m_nWidth>>_nMip)-1;
	int tv_wrap = (_pCurTex->m_nHeight>>_nMip)-1;
#endif

#if bpp8 == 1
	unsigned int r = _Intensity>>2;
#else
	unsigned int r = _Intensity;
#endif
	unsigned int r0 = r;

#if gouraud == 1
	float gRight;
#elif affine == 0
	float LxRight, LyRight, LzRight;
#endif
#if affine == 0
	#if bilinear == 1
		int X=XStart, dY=(pLeft->Y&1)<<2;
	#endif
#endif

	if ( Width > 0 )
	{
#if affine == 1
		ZRight = 1.0f / (pRight->OneOverZ - Gradients.dOneOverZdX);
		URight = ZRight * (pRight->UOverZ - Gradients.dUOverZdX);
		VRight = ZRight * (pRight->VOverZ - Gradients.dVOverZdX);
	
		U = Fixed16_16Math::FloatToFixed(ULeft) + Gradients.dUdXModifier - 32768;
		V = Fixed16_16Math::FloatToFixed(VLeft) + Gradients.dVdXModifier - 32768;
	
		//Lighting
		u32 L0 = r, L1 = r;
	#if gouraud == 1
		gRight = ZRight * (pRight->gOverZ - Gradients.dgOverZdX);
		L0 += (int)(gLeft);
		L1 += (int)(gRight);
	#endif

		fixed16_16 Z  = Fixed16_16Math::FloatToFixed(ZLeft);
		if ( Z < 0 ) { return; }
	
		fixed16_16 dZ = 0;
		s32 dF = 0;
		#if bpp8 == 1
			s32 F0, F1;
			F0 = MIN((s32)L0, 63 - (Z>>10));
			F1=F0;
		#else
			s32 F0 = MIN((s32)L0, 255 - (Z>>8)), F1=F0;
		#endif
		s32 F  = F0;
		//Guard against div-by-0 for 1 pixel lines.
		if ( --Width )
		{
			DeltaU = Fixed16_16Math::FloatToFixed(URight - ULeft) / Width;
			DeltaV = Fixed16_16Math::FloatToFixed(VRight - VLeft) / Width;
		
			dZ = Fixed16_16Math::FloatToFixed(ZRight - ZLeft) / Width;
		
			#if bpp8 == 1
				F1 = MIN((s32)L1, 63 - ((Z+dZ*Width)>>10));
			#else
				F1 = MIN((s32)L1, 255 - ((Z+dZ*Width)>>8));
			#endif
			dF = (F1 - F0) / Width;
		}
						
		for (int Counter=0; Counter <= Width; Counter++)
		{
			//
			u16 Zd = *pDepthBuffer;
			if ( Z <= Zd )
			{
				#if pow2 == 1
					int UInt = (U>>16)&tu_wrap;
					int VInt = (V>>16)&tv_wrap;
				#else
					int UInt = MAX(U>>16,0);	
					int VInt = MAX(V>>16,0);
				#endif
			
				#if bpp8==1
					u8 color = *( pTextureBits + UInt + (VInt*TextureDeltaScan) );
					*pDestBits = pColormapFog[ color + (F<<8) ];
					*pDepthBuffer = (u16)Z;
				#else
					u32 color = _pCurPal[ *( pTextureBits + UInt + (VInt*TextureDeltaScan) ) ];
						color = _colorMap32[0][((color>>16)&0xff) + (F<<8)] | _colorMap32[1][((color>>8)&0xff) + (F<<8)] |
							    _colorMap32[2][(color&0xff) + (F<<8)] | (color&0xff000000);
					#if alpha != 0
					if ((color&0xff000000) > 0)
					{
					#endif
						*pDestBits = color;
						*pDepthBuffer = (u16)Z;
					#if alpha != 0
					}
					#endif
				#endif
			}
			pDestBits++;
			pDepthBuffer++;	
		
			Z += dZ;
			U += DeltaU;
			V += DeltaV;
			F += dF;
		}
#else
		int Subdivisions   = Width / _nAffineLength;
		int WidthModLength = Width & (_nAffineLength-1);
	
		if ( !WidthModLength )
		{
			Subdivisions--;
			WidthModLength = _nAffineLength;
		}
	
		while (Subdivisions-- > 0)
		{
			ZRight = 1.0f / OneOverZRight;
			URight = ZRight * UOverZRight;
			VRight = ZRight * VOverZRight;
		
			U = Fixed16_16Math::FloatToFixed(ULeft) + Gradients.dUdXModifier - 32768;
			V = Fixed16_16Math::FloatToFixed(VLeft) + Gradients.dVdXModifier - 32768;
			DeltaU = Fixed16_16Math::FloatToFixed(URight - ULeft) / _nAffineLength;
			DeltaV = Fixed16_16Math::FloatToFixed(VRight - VLeft) / _nAffineLength;
		
			fixed16_16 Z  = Fixed16_16Math::FloatToFixed(ZLeft);
			if ( Z < 0 ) { return; }
		
			fixed16_16 dZ = Fixed16_16Math::FloatToFixed(ZRight - ZLeft) / _nAffineLength;
			//Lighting
			u32 L0 = r, L1 = r;
		#if gouraud == 1
			gRight = ZRight * gOverZRight;
			L0 += (int)(gLeft);
			L1 += (int)(gRight);
		#else
			if ( _nLightCnt )
			{
				LxRight = ZRight * LxOverZRight;
				LyRight = ZRight * LyOverZRight;
				LzRight = ZRight * LzOverZRight;
			
				for (int l=0; l<_nLightCnt; l++)
				{
					float dx = (LxLeft - _avLightPos[l].x)*_fLightScale;
					float dy = (LyLeft - _avLightPos[l].y)*_fLightScale;
					float dz = (LzLeft - _avLightPos[l].z)*_fLightScale;
					float D  = _sqrtTable[ MIN(Fixed16_16Math::FloatToFixed( dx*dx + dy*dy + dz*dz),65536) ];
					float A  = MAX( 1.0f - D, 0 );
					float ooD = 1.0f/D;
					Vector3 LVec(dx*ooD, dy*ooD, dz*ooD);
					float NDotL = MAX( LVec.Dot(_N)*_afIntens[l], 0 );
				#if bpp8 == 1
					L0 += (int)(NDotL*A*63);
				#else
					L0 += (int)(NDotL*A*255);
				#endif
				
					dx = (LxRight - _avLightPos[l].x)*_fLightScale;
					dy = (LyRight - _avLightPos[l].y)*_fLightScale;
					dz = (LzRight - _avLightPos[l].z)*_fLightScale;
					D  = _sqrtTable[ MIN(Fixed16_16Math::FloatToFixed( dx*dx + dy*dy + dz*dz),65536) ];
					A  = MAX( 1.0f - D, 0 );
					ooD = 1.0f/D;
					LVec.Set(dx*ooD, dy*ooD, dz*ooD);
					NDotL = MAX( LVec.Dot(_N)*_afIntens[l], 0 );
				#if bpp8 == 1
					L1 += (int)(NDotL*A*63);
				#else
					L1 += (int)(NDotL*A*255);
				#endif
				}
			}
		#endif
		
			//lighting
			#if bpp8 == 1
				s32 F0, F1;
			#if fogLighting == 0
				if ( _useFog )
				{
					F0 = MIN((s32)L0, 63 - (Z>>10));
					F1 = MIN((s32)L1, 63 - ((Z+dZ*_nAffineLength)>>10));
				}
				else
			#endif
				{
					F0 = MIN((s32)L0, 63);
					F1 = MIN((s32)L1, 63);
				}
			#else
				s32 F0 = MIN((s32)L0, 255 - (Z>>8));
				s32 F1 = MIN((s32)L1, 255 - ((Z+dZ*_nAffineLength)>>8));
			#endif
			s32 F  = F0;
			s32 dF = (F1 - F0) / _nAffineLength;
		
			for (int Counter = 0; Counter < _nAffineLength; Counter++)
			{
				u16 Zd = *pDepthBuffer;
				if ( Z <= Zd )
				{
				#if bilinear == 1
					int dU = _ditherTable[ ((X&1)<<1)+dY ];
					int dV = _ditherTable[ ((X&1)<<1)+dY+1 ];
					#if pow2 == 1
						int UInt = ((U+dU)>>16)&tu_wrap;
						int VInt = ((V+dV)>>16)&tv_wrap;
					#else
						int UInt = MAX((U+dU)>>16,0);	
						int VInt = MAX((V+dV)>>16,0);
					#endif
				#else
					#if pow2 == 1
						int UInt = (U>>16)&tu_wrap;
						int VInt = (V>>16)&tv_wrap;
					#else
						int UInt = MAX(U>>16,0);	
						int VInt = MAX(V>>16,0);
					#endif
				#endif
				#if bpp8==1
					u8 color = *( pTextureBits + UInt + (VInt*TextureDeltaScan) );
					#if alpha != 0
					if (color)
					{
					#endif
					#if fogLighting == 1
						color = (F < 63 && color < 255) ? pColormapLit[ color + (F<<8) ] : color;
						*pDestBits = pColormapFog[ color + ((63 - (Z>>10))<<8) ];
					#else
						#if alpha == 2
							*pDestBits = _aTransTable_Blend[ pColormapFog[ color + (F<<8) ] + ((*pDestBits)<<8) ];
						#else
							*pDestBits = pColormapFog[ color + (F<<8) ];
						#endif
					#endif
						*pDepthBuffer = (u16)Z;
					#if alpha != 0
					}
					#endif
				#else
					u32 color = _pCurPal[ *( pTextureBits + UInt + (VInt*TextureDeltaScan) ) ];
						color = _colorMap32[0][((color>>16)&0xff) + (F<<8)] | _colorMap32[1][((color>>8)&0xff) + (F<<8)] |
								_colorMap32[2][(color&0xff) + (F<<8)] | (color&0xff000000);
						//color = ((((color&0x00ff0000)*F)>>8)&0x00ff0000) | ((((color&0x0000ff00)*F)>>8)&0x0000ff00) |
						//((((color&0x000000ff)*F)>>8)&0x000000ff) | (color&0xff000000);
					#if alpha != 0
					if ((color&0xff000000) > 0)
					{
					#endif
						*pDestBits = color;
						*pDepthBuffer = (u16)Z;
					#if alpha != 0
					}
					#endif
				#endif
				}
				pDestBits++;
				pDepthBuffer++;
			
				Z += dZ;
				U += DeltaU;
				V += DeltaV;
				F += dF;
			#if bilinear == 1
				X++;
			#endif
			}
		
			ZLeft = ZRight;
			ULeft = URight;
			VLeft = VRight;
		
			//Lighting.
		#if gouraud == 1
			gLeft = gRight;
			gOverZRight += dgOverZdXAff;
		#else
			if ( _nLightCnt )
			{
				LxLeft = LxRight;
				LyLeft = LyRight;
				LzLeft = LzRight;
				LxOverZRight += dLxOverZdXAff;
				LyOverZRight += dLyOverZdXAff;
				LzOverZRight += dLzOverZdXAff;
			}
		#endif
		
			//
			OneOverZRight += dOneOverZdXAff;
			UOverZRight += dUOverZdXAff;
			VOverZRight += dVOverZdXAff;
		}
	
		if ( WidthModLength )
		{
			ZRight = 1.0f / (pRight->OneOverZ - Gradients.dOneOverZdX);
			URight = ZRight * (pRight->UOverZ - Gradients.dUOverZdX);
			VRight = ZRight * (pRight->VOverZ - Gradients.dVOverZdX);
		
			U = Fixed16_16Math::FloatToFixed(ULeft) + Gradients.dUdXModifier - 32768;
			V = Fixed16_16Math::FloatToFixed(VLeft) + Gradients.dVdXModifier - 32768;
		
			//Lighting
			u32 L0 = r, L1 = r;
		#if gouraud == 1
			gRight = ZRight * (pRight->gOverZ - Gradients.dgOverZdX);
			L0 += (int)(gLeft);
			L1 += (int)(gRight);
		#else
			if ( _nLightCnt )
			{
				float LxRight = ZRight * (pRight->LxOverZ - Gradients.dLxOverZdX);
				float LyRight = ZRight * (pRight->LyOverZ - Gradients.dLyOverZdX);
				float LzRight = ZRight * (pRight->LzOverZ - Gradients.dLzOverZdX);
			
				for (int l=0; l<_nLightCnt; l++)
				{
					float dx = (LxLeft - _avLightPos[l].x)*_fLightScale;
					float dy = (LyLeft - _avLightPos[l].y)*_fLightScale;
					float dz = (LzLeft - _avLightPos[l].z)*_fLightScale;
					float D  = _sqrtTable[ MIN(Fixed16_16Math::FloatToFixed( dx*dx + dy*dy + dz*dz),65536) ];
					float A  = MAX( 1.0f - D, 0 );
					float ooD = 1.0f/D;
					Vector3 LVec(dx*ooD, dy*ooD, dz*ooD);
					float NDotL = MAX( LVec.Dot(_N)*_afIntens[l], 0 );
					#if bpp8 == 1
						L0 += (int)(NDotL*A*63);
					#else
						L0 += (int)(NDotL*A*255);
					#endif
				
					dx = (LxRight - _avLightPos[l].x)*_fLightScale;
					dy = (LyRight - _avLightPos[l].y)*_fLightScale;
					dz = (LzRight - _avLightPos[l].z)*_fLightScale;
					D  = _sqrtTable[ MIN(Fixed16_16Math::FloatToFixed( dx*dx + dy*dy + dz*dz),65536) ];
					A  = MAX( 1.0f - D, 0 );
					ooD = 1.0f/D;
					LVec.Set(dx*ooD, dy*ooD, dz*ooD);
					NDotL = MAX( LVec.Dot(_N)*_afIntens[l], 0 );
					#if bpp8 == 1
						L1 += (int)(NDotL*A*63);
					#else
						L1 += (int)(NDotL*A*255);
					#endif
				}
			}
		#endif
		
			fixed16_16 Z  = Fixed16_16Math::FloatToFixed(ZLeft);
			if ( Z < 0 ) { return; }
		
			fixed16_16 dZ = 0;
			s32 dF = 0;
			#if bpp8 == 1
				s32 F0, F1;
				#if fogLighting == 0
				if ( _useFog )
					F0 = MIN((s32)L0, 63 - (Z>>10));
				else
				#endif
					F0 = MIN((s32)L0, 63);

				F1=F0;
			#else
				s32 F0 = MIN((s32)L0, 255 - (Z>>8)), F1=F0;
			#endif
			s32 F  = F0;
			//Guard against div-by-0 for 1 pixel lines.
			if ( --WidthModLength )
			{
				DeltaU = Fixed16_16Math::FloatToFixed(URight - ULeft) / WidthModLength;
				DeltaV = Fixed16_16Math::FloatToFixed(VRight - VLeft) / WidthModLength;
			
				dZ = Fixed16_16Math::FloatToFixed(ZRight - ZLeft) / WidthModLength;
			
				#if bpp8 == 1
				#if fogLighting == 0
					if ( _useFog )
						F1 = MIN((s32)L1, 63 - ((Z+dZ*WidthModLength)>>10));
					else
				#endif
						F1 = MIN((s32)L1, 63);
				#else
					F1 = MIN((s32)L1, 255 - ((Z+dZ*WidthModLength)>>8));
				#endif
				dF = (F1 - F0) / WidthModLength;
			}
							
			for (int Counter=0; Counter <= WidthModLength; Counter++)
			{
				//
				u16 Zd = *pDepthBuffer;
				if ( Z <= Zd )
				{
					#if bilinear == 1
						int dU = _ditherTable[ ((X&1)<<1)+dY ];
						int dV = _ditherTable[ ((X&1)<<1)+dY+1 ];
						#if pow2 == 1
							int UInt = ((U+dU)>>16)&tu_wrap;
							int VInt = ((V+dV)>>16)&tv_wrap;
						#else
							int UInt = MAX((U+dU)>>16,0);	
							int VInt = MAX((V+dV)>>16,0);
						#endif
					#else
						#if pow2 == 1
							int UInt = (U>>16)&tu_wrap;
							int VInt = (V>>16)&tv_wrap;
						#else
							int UInt = MAX(U>>16,0);	
							int VInt = MAX(V>>16,0);
						#endif
					#endif
				
					#if bpp8==1
						u8 color = *( pTextureBits + UInt + (VInt*TextureDeltaScan) );
						#if alpha != 0
						if (color)
						{
						#endif
						#if fogLighting == 1
							color = (F < 63 && color < 255) ? pColormapLit[ color + (F<<8) ] : color;
							*pDestBits = pColormapFog[ color + ((63 - (Z>>10))<<8) ];
						#else
							#if alpha == 2
								*pDestBits = _aTransTable_Blend[ pColormapFog[ color + (F<<8) ] + ((*pDestBits)<<8) ];
							#else
								*pDestBits = pColormapFog[ color + (F<<8) ];
							#endif
						#endif
							*pDepthBuffer = (u16)Z;
						#if alpha != 0
						}
						#endif
					#else
						u32 color = _pCurPal[ *( pTextureBits + UInt + (VInt*TextureDeltaScan) ) ];
							//color = ((((color&0x00ff0000)*F)>>8)&0x00ff0000) | ((((color&0x0000ff00)*F)>>8)&0x0000ff00) |
							//((((color&0x000000ff)*F)>>8)&0x000000ff) | (color&0xff000000);
							color = _colorMap32[0][((color>>16)&0xff) + (F<<8)] | _colorMap32[1][((color>>8)&0xff) + (F<<8)] |
								    _colorMap32[2][(color&0xff) + (F<<8)] | (color&0xff000000);
						#if alpha != 0
						if ((color&0xff000000) > 0)
						{
						#endif
							*pDestBits = color;
							*pDepthBuffer = (u16)Z;
						#if alpha != 0
						}
						#endif
					#endif
				}
				pDestBits++;
				pDepthBuffer++;	
			
				Z += dZ;
				U += DeltaU;
				V += DeltaV;
				F += dF;
			#if bilinear == 1
				X++;
			#endif
			}
		}
#endif	//affine
	}
}

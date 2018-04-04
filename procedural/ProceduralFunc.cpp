#include "ProceduralFunc.h"
#include "Noise.h"
#include "../math/Math.h"
#include <math.h>

namespace ProceduralFunc
{
	float fBm(Vector3 p, int nOctaves, float H/*=0.5f*/, float r/*=2.0f*/)
	{
		float z = p.z;
		float fRes = Noise::Noise3D(p.x, p.y, z);
		float fAmp=H;
		for (int i=1; i<nOctaves; i++)
		{
			p = p * r;
			fRes += fAmp*Noise::Noise3D(p.x, p.y, z);
			fAmp *= H;
		}
		return fRes;
	}

	float Turbulance(Vector3 p, int nOctaves, float H/*=0.5f*/, float r/*=2.0f*/)
	{
		float z = p.z;
		float fRes = fabsf( Noise::Noise3D(p.x, p.y, z) );
		float fAmp=H;

		for (int i=1; i<nOctaves; i++)
		{
			p = p * r;
			fRes += fAmp*fabsf(Noise::Noise3D(p.x, p.y, z));
			fAmp *= H;
		}
		return fRes;
	}

	float Ridged(Vector3 p, int nOctaves, float H/*=0.5f*/, float r/*=2.0f*/)
	{
		float z = p.z;
		float fRes = 1.0f - fabsf( Noise::Noise3D(p.x, p.y, z) );
		float fAmp=H;

		for (int i=1; i<nOctaves; i++)
		{
			p = p * r;
			fRes += fAmp*(1.0f - fabsf(Noise::Noise3D(p.x, p.y, z)));
			fAmp *= H;
		}
		return Math::Min(fRes,1.0f);
	}

	float RidgedMulti(Vector3 p, int nOctaves, float power/*=2.0f*/, float H/*=0.5f*/, float r/*=2.0f*/)
	{
		const float gain = 2.0f;

		float z = p.z;
		float weight = 1.0f;
		float signal = powf( 1.0f - fabsf( Noise::Noise3D(p.x, p.y, z) ), power );
		float fRes = signal;
		float fAmp=H;

		for (int i=1; i<nOctaves; i++)
		{
			weight = Math::clamp( signal * gain, 0.0f, 1.0f );

			p = p * r;

			signal = weight*powf( 1.0f - fabsf( Noise::Noise3D(p.x, p.y, z) ), power );
			fRes += signal * fAmp;

			fAmp *= H;
		}
		return Math::Min(fRes,1.0f);
	}
};
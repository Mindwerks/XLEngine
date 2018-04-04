#include "Vector4.h"
#include <math.h>

Vector4 Vector4::One(1.0f, 1.0f, 1.0f, 1.0f);
Vector4 Vector4::Half(0.5f, 0.5f, 0.5f, 0.5f);
Vector4 Vector4::Zero(0.0f, 0.0f, 0.0f, 0.0f);

f32 Vector4::Normalize()
{
	f32 mag2 = x*x + y*y + z*z + w*w;
	f32 mag = 0.0f;
	if ( mag2 > 0.0001f )
	{
		mag = sqrtf(mag2);
		f32 oomag = 1.0f / mag;
		x *= oomag;
		y *= oomag;
		z *= oomag;
		w *= oomag;
	}
	return mag;
}

f32 Vector4::Normalize3()
{
	f32 mag2 = x*x + y*y + z*z;
	f32 mag = 0.0f;
	if ( mag2 > 0.0001f )
	{
		mag = sqrtf(mag2);
		f32 oomag = 1.0f / mag;
		x *= oomag;
		y *= oomag;
		z *= oomag;
	}
	return mag;
}

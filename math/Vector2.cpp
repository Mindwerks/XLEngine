#include "Vector2.h"
#include <math.h>

Vector2 Vector2::One(1.0f, 1.0f);
Vector2 Vector2::Half(0.5f, 0.5f);
Vector2 Vector2::Zero(0.0f, 0.0f);

f32 Vector2::Normalize()
{
	f32 mag2 = x*x + y*y;
	f32 mag = 0.0f;
	if ( mag2 > 0.0001f )
	{
		mag = sqrtf(mag2);
		f32 oomag = 1.0f / mag;
		x *= oomag;
		y *= oomag;
	}
	return mag;
}

#include "Vector4.h"
#include <math.h>

Vector4 Vector4::One(1.0f, 1.0f, 1.0f, 1.0f);
Vector4 Vector4::Half(0.5f, 0.5f, 0.5f, 0.5f);
Vector4 Vector4::Zero(0.0f, 0.0f, 0.0f, 0.0f);

float Vector4::Normalize()
{
    float mag2 = x*x + y*y + z*z + w*w;
    float mag = 0.0f;
    if ( mag2 > 0.0001f )
    {
        mag = sqrtf(mag2);
        float oomag = 1.0f / mag;
        x *= oomag;
        y *= oomag;
        z *= oomag;
        w *= oomag;
    }
    return mag;
}

float Vector4::Normalize3()
{
    float mag2 = x*x + y*y + z*z;
    float mag = 0.0f;
    if ( mag2 > 0.0001f )
    {
        mag = sqrtf(mag2);
        float oomag = 1.0f / mag;
        x *= oomag;
        y *= oomag;
        z *= oomag;
    }
    return mag;
}

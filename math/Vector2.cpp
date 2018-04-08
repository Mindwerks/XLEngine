#include "Vector2.h"

Vector2 Vector2::One(1.0f, 1.0f);
Vector2 Vector2::Half(0.5f, 0.5f);
Vector2 Vector2::Zero(0.0f, 0.0f);

float Vector2::Normalize() {
    float mag2 = x * x + y * y;
    float mag = 0.0f;
    if (mag2 > 0.0001f)
    {
        mag = sqrtf(mag2);
        float oomag = 1.0f / mag;
        x *= oomag;
        y *= oomag;
    }
    return mag;
}

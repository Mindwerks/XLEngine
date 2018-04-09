#include "Plane.h"
#include <cassert>

float Plane::Normalize()
{
    float mag, oomag;
    mag = sqrtf(a*a + b*b + c*c);
    if ( mag )
    {
        oomag = 1.0f/mag;
        a *= oomag;
        b *= oomag;
        c *= oomag;
        d *= oomag;
    }
    return mag;
}

HalfSpace_e Plane::ClassifyPoint(const Vector3& vPt) const
{
    float dist;
    dist = a*vPt.x + b*vPt.y + c*vPt.z + d;
    if ( dist < 0.0f ) return NEGATIVE;
    if ( dist > 0.0f ) return POSITIVE;
    return ON_PLANE;
}

void Plane::Build(Vector3 *vPoints)
{
    Vector3 vA, vB, vN;
    vA.Set( vPoints[1].x-vPoints[0].x, vPoints[1].y-vPoints[0].y, vPoints[1].z-vPoints[0].z );
    vB.Set( vPoints[2].x-vPoints[0].x, vPoints[2].y-vPoints[0].y, vPoints[2].z-vPoints[0].z );

    vN.Cross(vA, vB);
    vN.Normalize();

    a =  vN.x; b = vN.y; c = vN.z;
    d = -vN.Dot( vPoints[0] );
}

bool Plane::Build(const Vector3& v0, const Vector3& v1, const Vector3& v2)
{
    Vector3 vA, vB, vN;
    vA.Set(v1.x-v0.x, v1.y-v0.y, v1.z-v0.z);
    vB.Set(v2.x-v0.x, v2.y-v0.y, v2.z-v0.z);

    vN.Cross(vA, vB);
    float m = vN.Normalize();

    a =  vN.x; b = vN.y; c = vN.z;
    d = -vN.Dot(v0);

    if ( m < 1.192092896e-07F )
    {
        //assert(0);
        return false;
    }

    return true;
}

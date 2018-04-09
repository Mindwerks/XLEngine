#ifndef MATH_H
#define MATH_H

#include <cmath>
#include "../CommonTypes.h"
#include "Vector2.h"
#include "Vector3.h"

#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif

#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif

#define MATH_PI 3.1415926535897932384626433832795f
#define MATH_TWO_PI 6.283185307179586476925286766559f
#define MATH_PI_OVER_2 1.5707963267948966192313216916398f

class Math
{
public:
    static inline unsigned int RoundNextPow2(unsigned int x)
    {
        x = x-1;
        x = x | (x>>1);
        x = x | (x>>2);
        x = x | (x>>4);
        x = x | (x>>8);
        x = x | (x>>16);
        return x + 1;
    }

    static inline float saturate(float x)
    {
        return x > 0.0f ? ((x < 1.0f) ? x : 1.0f) : 0.0f;
    }

    static inline float sign(float x)
    {
        return x >= 0.0f ? 1.0f : -1.0f;
    }

    static inline int32_t abs(int32_t x)
    {
        return (x < 0) ? -x : x;
    }

    static inline float abs(float x)
    {
        return (x < 0) ? -x : x;
    }

    static inline float signZero(float x, float e)
    {
        float r = 0.0f;
        if ( fabsf(x) > e )
        {
            r = (x > 0.0f) ? 1.0f : -1.0f;
        }
        return r;
    }

    static inline float clamp(float x, float a, float b)
    {
        float c = (x>a) ? x : a;
        c = (c<b) ? c : b;

        return c;
    }

    static inline int32_t clamp(int32_t x, int32_t a, int32_t b)
    {
        int32_t c = (x>a) ? x : a;
        c = (c<b) ? c : b;

        return c;
    }

    static void ClosestPointToLine2D(float x, float y, float x0, float y0, float x1, float y1, float& ix, float& iy)
    {
        float d2 = (x1-x0)*(x1-x0) + (y1-y0)*(y1-y0);
        float lu;
        if ( d2 <= 0.00000001f ) { ix = x; iy = y; return; }

        lu = ( (x - x0)*(x1-x0) + (y-y0)*(y1-y0) ) / d2;
        lu = Math::clamp(lu, 0.0f, 1.0f);
        ix = x0 + lu*(x1-x0);
        iy = y0 + lu*(y1-y0);

        float dx, dy;
        dx = ix - x; dy = iy - y;
        d2 = dx*dx + dy*dy;
    }

    static float DistPointLine2D_Sqr(float x, float y, float x0, float y0, float x1, float y1)
    {
        float d2 = (x1-x0)*(x1-x0) + (y1-y0)*(y1-y0);
        float lu;
        if ( d2 <= 0.00000001f ) { return 0.0f; }

        lu = ( (x - x0)*(x1-x0) + (y-y0)*(y1-y0) ) / d2;
        lu = Math::clamp(lu, 0.0f, 1.0f);
        float ix = x0 + lu*(x1-x0);
        float iy = y0 + lu*(y1-y0);

        float dx, dy;
        dx = ix - x; dy = iy - y;
        d2 = dx*dx + dy*dy;

        return d2;
    }

    static bool LineIntersect2D(const Vector2& p0, const Vector2& p1, const Vector2& p2, const Vector2& p3, float& s, float& t)
    {
        float denom = (p3.y-p2.y)*(p1.x-p0.x) - (p3.x-p2.x)*(p1.y-p0.y);
        if ( fabsf(denom) < 0.000001f )
            return false;

        float ooDenom = 1.0f / denom;
        s = ( (p3.x-p2.x)*(p0.y-p2.y) - (p3.y-p2.y)*(p0.x-p2.x) ) * ooDenom;
        t = ( (p1.x-p0.x)*(p0.y-p2.y) - (p1.y-p0.y)*(p0.x-p2.x) ) * ooDenom;

        if ( s >= -0.0001f && s <= 1.0001f && t >= -0.0001f && t <= 1.0001f )
            return true;

        return false;
    }

    static bool IntervalOverlap(float i0a, float i0b, float i1a, float i1b)
    {
        if ( i1b < i0a || i1a > i0b )
        {
            return false;
        }
        return true;
    }

    static bool IntervalOverlapEq(float i0a, float i0b, float i1a, float i1b)
    {
        if ( i1b <= i0a || i1a >= i0b )
        {
            return false;
        }
        return true;
    }

    static bool IntervalOccluded(float occA, float occB, float i1a, float i1b)
    {
        //is i1(a,b) occluded by occ(A,B)?
        if ( i1a >= occA && i1b <= occB )
        {
            return true;
        }
        return false;
    }

    static bool AABB_Overlap3D(const Vector3& minA, const Vector3& maxA, const Vector3& minB, const Vector3& maxB)
    {
        if ( maxA.x < minB.x || minA.x > maxB.x ) return false;
        if ( maxB.y < minA.y || minB.y > maxA.y ) return false;
        if ( maxB.z < minA.z || minB.z > maxA.z ) return false;

        return true;
    }

    static bool AABB_Overlap2D(const Vector2& minA, const Vector2& maxA, const Vector2& minB, const Vector2& maxB)
    {
        if ( maxA.x < minB.x || minA.x > maxB.x ) return false;
        if ( maxB.y < minA.y || minB.y > maxA.y ) return false;

        return true;
    }

    static bool EdgeIntersectsBox2D(const Vector2& vAABB_Min, const Vector2& vAABB_Max, const Vector2& v0, const Vector2& v1)
    {
        const Vector2 vEdgeMin( MIN(v0.x, v1.x), MIN(v0.y, v1.y) );
        const Vector2 vEdgeMax( MAX(v0.x, v1.x), MAX(v0.y, v1.y) );

        return AABB_Overlap2D(vAABB_Min, vAABB_Max, vEdgeMin, vEdgeMax);
    }

    static float IsLeft(const Vector2& p0, const Vector2& p1, const Vector2& p2)
    {
        return (p1.x - p0.x) * (p2.y - p0.y) - (p2.x - p0.x) * (p1.y - p0.y);
    }

    static float Min(float a, float b)
    {
        return (a<b) ? a : b;
    }

    static float Max(float a, float b)
    {
        return (a>b) ? a : b;
    }

    static int32_t Min(int32_t a, int32_t b)
    {
        return (a<b) ? a : b;
    }

    static int32_t Max(int32_t a, int32_t b)
    {
        return (a>b) ? a : b;
    }

    static float log2( float n )
    {
        // log(n)/log(2) is log2.
        return logf( n ) / logf( 2.0f );
    }

    static int32_t FloatToInt(float x)
    {
        uint32_t e = (0x7F + 31) - ((*(uint32_t*) &x & 0x7F800000) >> 23);
        uint32_t m = 0x80000000 | (*(uint32_t*) &x << 8);

        return int32_t((m >> e) & -(e < 32));
    }

    static float BiCubic_Heightfield(float p[4][4], float x, float y)
    {
        float a00 = p[1][1];
        float a01 = p[1][2] - p[1][1]/2 - p[1][0]/3 - p[1][3]/6;
        float a02 = p[1][0]/2 - p[1][1] + p[1][2]/2;
        float a03 = p[1][1]/2 - p[1][0]/6 - p[1][2]/2 + p[1][3]/6;
        float a10 = p[2][1] - p[1][1]/2 - p[0][1]/3 - p[3][1]/6;
        float a11 = p[0][0]/9 + p[0][1]/6 - p[0][2]/3 + p[0][3]/18 + p[1][0]/6 + p[1][1]/4 - p[1][2]/2 + p[1][3]/12 - p[2][0]/3 - p[2][1]/2 + p[2][2] - p[2][3]/6 + p[3][0]/18 + p[3][1]/12 - p[3][2]/6 + p[3][3]/36;
        float a12 = p[0][1]/3 - p[0][0]/6 - p[0][2]/6 - p[1][0]/4 + p[1][1]/2 - p[1][2]/4 + p[2][0]/2 - p[2][1] + p[2][2]/2 - p[3][0]/12 + p[3][1]/6 - p[3][2]/12;
        float a13 = p[0][0]/18 - p[0][1]/6 + p[0][2]/6 - p[0][3]/18 + p[1][0]/12 - p[1][1]/4 + p[1][2]/4 - p[1][3]/12 - p[2][0]/6 + p[2][1]/2 - p[2][2]/2 + p[2][3]/6 + p[3][0]/36 - p[3][1]/12 + p[3][2]/12 - p[3][3]/36;
        float a20 = p[0][1]/2 - p[1][1] + p[2][1]/2;
        float a21 = p[0][2]/2 - p[0][1]/4 - p[0][0]/6 - p[0][3]/12 + p[1][0]/3 + p[1][1]/2 - p[1][2] + p[1][3]/6 - p[2][0]/6 - p[2][1]/4 + p[2][2]/2 - p[2][3]/12;
        float a22 = p[0][0]/4 - p[0][1]/2 + p[0][2]/4 - p[1][0]/2 + p[1][1] - p[1][2]/2 + p[2][0]/4 - p[2][1]/2 + p[2][2]/4;
        float a23 = p[0][1]/4 - p[0][0]/12 - p[0][2]/4 + p[0][3]/12 + p[1][0]/6 - p[1][1]/2 + p[1][2]/2 - p[1][3]/6 - p[2][0]/12 + p[2][1]/4 - p[2][2]/4 + p[2][3]/12;
        float a30 = p[1][1]/2 - p[0][1]/6 - p[2][1]/2 + p[3][1]/6;
        float a31 = p[0][0]/18 + p[0][1]/12 - p[0][2]/6 + p[0][3]/36 - p[1][0]/6 - p[1][1]/4 + p[1][2]/2 - p[1][3]/12 + p[2][0]/6 + p[2][1]/4 - p[2][2]/2 + p[2][3]/12 - p[3][0]/18 - p[3][1]/12 + p[3][2]/6 - p[3][3]/36;
        float a32 = p[0][1]/6 - p[0][0]/12 - p[0][2]/12 + p[1][0]/4 - p[1][1]/2 + p[1][2]/4 - p[2][0]/4 + p[2][1]/2 - p[2][2]/4 + p[3][0]/12 - p[3][1]/6 + p[3][2]/12;
        float a33 = p[0][0]/36 - p[0][1]/12 + p[0][2]/12 - p[0][3]/36 - p[1][0]/12 + p[1][1]/4 - p[1][2]/4 + p[1][3]/12 + p[2][0]/12 - p[2][1]/4 + p[2][2]/4 - p[2][3]/12 - p[3][0]/36 + p[3][1]/12 - p[3][2]/12 + p[3][3]/36;

        float x2 = x * x;
        float x3 = x2 * x;
        float y2 = y * y;
        float y3 = y2 * y;

        return a00 + a01 * y + a02 * y2 + a03 * y3 +
               a10 * x + a11 * x * y + a12 * x * y2 + a13 * x * y3 +
               a20 * x2 + a21 * x2 * y + a22 * x2 * y2 + a23 * x2 * y3 +
               a30 * x3 + a31 * x3 * y + a32 * x3 * y2 + a33 * x3 * y3;
    }
};

#endif //MATH_H

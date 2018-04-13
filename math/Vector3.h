#ifndef VECTOR3_H
#define VECTOR3_H

#include "../Engine.h"
#include <cmath>

#ifndef VEC_EPS
#define VEC_EPS 0.0001f
#define VEC_EPS_DOUBLE 0.000000001
#endif

#ifndef MINS
#define MINS(s, t) (s)<(t)?(s):(t)
#define MAXS(s, t) (s)>(t)?(s):(t)
#endif

#define SAFE_RCP(a) a = ( (a!=0)?(1.0f/a):(a) )
#define SAFE_RCP_DBL(a) a = ( (a!=0)?(1.0/a):(a) )

class Vector3
{
public:
    Vector3() { x = 0.0f; y = 0.0f; z = 0.0f; }
    Vector3(float _x, float _y, float _z) { x = _x; y = _y; z = _z; }
    ~Vector3() = default;

    float Length()
    {
        float d2 = x*x + y*y + z*z;
        if ( d2 > 0.000001f )
        {
            return sqrtf(d2);
        }
        return 0.0f;
    }
    float Normalize()
    {
        float mag2 = x*x + y*y + z*z;
        float mag = 0.0f;
        if ( mag2 > 0.00000001f )
        {
            mag = sqrtf(mag2);
            float oomag = 1.0f / mag;
            x *= oomag;
            y *= oomag;
            z *= oomag;
        }
        return mag;
    }

    void Cross(Vector3& A, Vector3& B)
    {
        this->x = A.y*B.z - A.z*B.y;
        this->y = A.z*B.x - A.x*B.z;
        this->z = A.x*B.y - A.y*B.x;
    }

    void CrossAndNormalize(Vector3& A, Vector3& B)
    {
        this->x = A.y*B.z - A.z*B.y;
        this->y = A.z*B.x - A.x*B.z;
        this->z = A.x*B.y - A.y*B.x;

        Normalize();
    }
    float Dot(Vector3& A) { return this->x*A.x + this->y*A.y + this->z*A.z; }
    float Dot(const Vector3& A) const { return this->x*A.x + this->y*A.y + this->z*A.z; }
    void Set(float _x, float _y, float _z) { x = _x; y = _y; z = _z; }

    inline Vector3 operator+(Vector3& other) { return Vector3(x+other.x, y+other.y, z+other.z); }
    inline Vector3 operator+(Vector3* other) { return Vector3(x+other->x, y+other->y, z+other->z); }
    inline Vector3 operator+(Vector3& other) const { return Vector3(x+other.x, y+other.y, z+other.z); }
    inline Vector3 operator+(Vector3* other) const { return Vector3(x+other->x, y+other->y, z+other->z); }
    inline Vector3 operator+(const Vector3& other) { return Vector3(x+other.x, y+other.y, z+other.z); }
    inline Vector3 operator+(const Vector3* other) { return Vector3(x+other->x, y+other->y, z+other->z); }
    inline Vector3 operator+(const Vector3& other) const { return Vector3(x+other.x, y+other.y, z+other.z); }
    inline Vector3 operator+(const Vector3* other) const { return Vector3(x+other->x, y+other->y, z+other->z); }
    inline Vector3 operator+(float other) { return Vector3(x+other, y+other, z+other); }
    inline Vector3 operator+=(Vector3& other) { return Vector3(x+other.x, y+other.y, z+other.z); }
    inline Vector3 operator+=(Vector3* other) { return Vector3(x+other->x, y+other->y, z+other->z); }
    inline Vector3 operator-(Vector3& other) { return Vector3(x-other.x, y-other.y, z-other.z); }
    inline Vector3 operator-(Vector3* other) { return Vector3(x-other->x, y-other->y, z-other->z); }
    inline Vector3 operator-(const Vector3& other) { return Vector3(x-other.x, y-other.y, z-other.z); }
    inline Vector3 operator-(const Vector3* other) { return Vector3(x-other->x, y-other->y, z-other->z); }
    inline Vector3 operator-(Vector3& other) const { return Vector3(x-other.x, y-other.y, z-other.z); }
    inline Vector3 operator-(Vector3* other) const { return Vector3(x-other->x, y-other->y, z-other->z); }
    inline Vector3 operator-(const Vector3& other) const { return Vector3(x-other.x, y-other.y, z-other.z); }
    inline Vector3 operator-(const Vector3* other) const { return Vector3(x-other->x, y-other->y, z-other->z); }
    inline Vector3 operator-(float other) { return Vector3(x-other, y-other, z-other); }
    inline Vector3 operator-=(Vector3& other) { return Vector3(x-other.x, y-other.y, z-other.z); }
    inline Vector3 operator-=(Vector3* other) { return Vector3(x-other->x, y-other->y, z-other->z); }
    inline Vector3 operator*(float scale) { return Vector3(x*scale, y*scale, z*scale); }
    inline Vector3 operator*(float scale) const { return Vector3(x*scale, y*scale, z*scale); }
    inline Vector3 operator*(Vector3& other) { return Vector3(x*other.x, y*other.y, z*other.z); }
    inline Vector3 operator*(Vector3* other) { return Vector3(x*other->x, y*other->y, z*other->z); }
    inline Vector3 operator*(const Vector3& other) { return Vector3(x*other.x, y*other.y, z*other.z); }
    inline Vector3 operator*(const Vector3* other) { return Vector3(x*other->x, y*other->y, z*other->z); }
    inline Vector3 operator*=(Vector3& other) { return Vector3(x*other.x, y*other.y, z*other.z); }
    inline Vector3 operator*=(Vector3* other) { return Vector3(x*other->x, y*other->y, z*other->z); }
    inline Vector3 operator/(float scale) { return Vector3(x/scale, y/scale, z/scale); }
    inline Vector3 operator-() { return Vector3(-x, -y, -z); }

    inline bool operator==(const Vector3& other) const { return ( fabsf(x-other.x)<VEC_EPS && fabsf(y-other.y)<VEC_EPS && fabsf(z-other.z)<VEC_EPS )?(true):(false); }
    inline bool operator!=(const Vector3& other) const { return ( fabsf(x-other.x)>VEC_EPS || fabsf(y-other.y)>VEC_EPS || fabsf(z-other.z)>VEC_EPS )?(true):(false); }

    inline float Mag2() { return (x*x+y*y+z*z); }
    inline float Max() { return ( (x>y)?(x>z?x:z):(y>z?y:z) ); }
    inline float Min() { return ( (x<y)?(x<z?x:z):(y<z?y:z) ); }
    inline void Reciprocal() { SAFE_RCP(x); SAFE_RCP(y); SAFE_RCP(z); }
    inline void Lerp(Vector3& v0, Vector3& v1, float fU)
    {
        x = v0.x + fU*(v1.x - v0.x);
        y = v0.y + fU*(v1.y - v0.y);
        z = v0.z + fU*(v1.z - v0.z);
    }
    inline Vector3 MinVec(Vector3& a, Vector3& b) { return Vector3(MINS(a.x, b.x), MINS(a.y, b.y), MINS(a.z, b.z)); }
    inline Vector3 MaxVec(Vector3& a, Vector3& b) { return Vector3(MAXS(a.x, b.x), MAXS(a.y, b.y), MAXS(a.z, b.z)); }

    inline void MinVec(Vector3& b) { this->x = MINS(this->x, b.x); this->y = MINS(this->y, b.y); this->z = MINS(this->z, b.z); }
    inline void MaxVec(Vector3& b) { this->x = MAXS(this->x, b.x); this->y = MAXS(this->y, b.y); this->z = MAXS(this->z, b.z); }

    inline float Distance(Vector3& vec)
    { 
        Vector3 diff = Vector3(x-vec.x, y-vec.y, z-vec.z); 
        return (sqrtf(diff.x*diff.x+diff.y*diff.y+diff.z*diff.z));
    }

    float x, y, z;

    static const Vector3 One;
    static const Vector3 Half;
    static const Vector3 Zero;
    static const Vector3 UnitX;
    static const Vector3 UnitY;
    static const Vector3 UnitZ;
};

#endif //VECTOR3_H
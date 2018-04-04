#ifndef VECTOR2_H
#define VECTOR2_H

#include "../Engine.h"
#include <math.h>

#ifndef VEC_EPS
#define VEC_EPS 0.0001f
#endif

#ifndef MINS
#define MINS(s, t) (s)<(t)?(s):(t)
#define MAXS(s, t) (s)>(t)?(s):(t)
#endif

#ifndef SAFE_RCP
#define SAFE_RCP(a) a = ( (a!=0)?(1.0f/a):(a) )
#endif

#define VEC_EQ_EPS 0.00001f

class Vector2
{
public:
	Vector2(void) { x = 0.0f; y = 0.0f; }
	Vector2(f32 _x, f32 _y) { x = _x; y = _y; }
	~Vector2(void) {;}

	f32 Normalize();
	f32 Length()
	{
		f32 d2 = x*x + y*y;
		if ( d2 > 0.000001f )
		{
			return sqrtf(d2);
		}
		return 0.0f;
	}
	f32 Dot(Vector2& A) { return this->x*A.x + this->y*A.y; }
	void Set(f32 _x, f32 _y) { x = _x; y = _y; }

	inline Vector2 operator+(Vector2& other) { return Vector2(x+other.x, y+other.y); }
	inline Vector2 operator+(Vector2* other) { return Vector2(x+other->x, y+other->y); }
	inline Vector2 operator+(const Vector2& other) { return Vector2(x+other.x, y+other.y); }
	inline Vector2 operator+(const Vector2* other) { return Vector2(x+other->x, y+other->y); }
	inline Vector2 operator+(const Vector2& other) const { return Vector2(x+other.x, y+other.y); }
	inline Vector2 operator+(const Vector2* other) const { return Vector2(x+other->x, y+other->y); }
	inline Vector2 operator+=(Vector2& other) { return Vector2(x+other.x, y+other.y); }
	inline Vector2 operator+=(Vector2* other) { return Vector2(x+other->x, y+other->y); }
	inline Vector2 operator-(Vector2& other) { return Vector2(x-other.x, y-other.y); }
	inline Vector2 operator-(Vector2* other) { return Vector2(x-other->x, y-other->y); }
	inline Vector2 operator-(const Vector2& other) { return Vector2(x-other.x, y-other.y); }
	inline Vector2 operator-(const Vector2* other) { return Vector2(x-other->x, y-other->y); }
	inline Vector2 operator-(const Vector2& other) const { return Vector2(x-other.x, y-other.y); }
	inline Vector2 operator-(const Vector2* other) const { return Vector2(x-other->x, y-other->y); }
	inline Vector2 operator-=(Vector2& other) { return Vector2(x-other.x, y-other.y); }
	inline Vector2 operator-=(Vector2* other) { return Vector2(x-other->x, y-other->y); }
	inline Vector2 operator*(f32 scale) { return Vector2(x*scale, y*scale); }
	inline Vector2 operator*(Vector2& other) { return Vector2(x*other.x, y*other.y); }
	inline Vector2 operator*(Vector2* other) { return Vector2(x*other->x, y*other->y); }
	inline Vector2 operator*(const Vector2& other) { return Vector2(x*other.x, y*other.y); }
	inline Vector2 operator*(const Vector2* other) { return Vector2(x*other->x, y*other->y); }
	inline Vector2 operator*=(Vector2& other) { return Vector2(x*other.x, y*other.y); }
	inline Vector2 operator*=(Vector2* other) { return Vector2(x*other->x, y*other->y); }
	inline Vector2 operator/(f32 scale) { return Vector2(x/scale, y/scale); }
	inline Vector2 operator-() { return Vector2(-x, -y); }

	inline bool operator==(Vector2& other) { return ( fabsf(x-other.x)<VEC_EPS && fabsf(y-other.y)<VEC_EPS )?(true):(false); }
	inline bool operator!=(Vector2& other) { return ( fabsf(x-other.x)>VEC_EPS || fabsf(y-other.y)>VEC_EPS )?(true):(false); }

	inline f32 Mag2() { return (x*x+y*y); }
	inline f32 Max() { return ( (x>y)?(x):(y) ); }
	inline f32 Min() { return ( (x<y)?(x):(y) ); }
	inline void Reciprocal() { SAFE_RCP(x); SAFE_RCP(y); }
	inline void Lerp(Vector2& v0, Vector2& v1, f32 fU)
	{
		x = v0.x + fU*(v1.x - v0.x);
		y = v0.y + fU*(v1.y - v0.y);
	}
	inline Vector2 MinVec(Vector2& a, Vector2& b) { return Vector2(MINS(a.x, b.x), MINS(a.y, b.y)); }
	inline Vector2 MaxVec(Vector2& a, Vector2& b) { return Vector2(MAXS(a.x, b.x), MAXS(a.y, b.y)); }

	inline f32 Distance(Vector2& vec) 
	{ 
		Vector2 diff = Vector2(x-vec.x, y-vec.y); 
		return (sqrtf(diff.x*diff.x+diff.y*diff.y));
	}

	inline bool IsEqual(Vector2& vec)
	{
		f32 dx = fabsf(x-vec.x), dy = fabsf(y-vec.y);
		if ( dx < VEC_EQ_EPS && dy < VEC_EQ_EPS )
			return true;
		return false;
	}

	f32 x, y;

	static Vector2 One;
	static Vector2 Half;
	static Vector2 Zero;
};

#endif //VECTOR2_H
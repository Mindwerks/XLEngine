#ifndef MATRIX_H
#define MATRIX_H

#include "../Engine.h"
#include "Vector3.h"
#include "Vector4.h"

class Matrix
{
public:

	enum MATRIX_VEC
	{
		VRIGHT=0,
		VFORWARD=1,
		VUP=2,
		VTRANSLATE=3,

		VX=0,
		VY=1,
		VZ=2,
		VW=3
	};

	f32 m[16];

	Matrix(void) {;}
	Matrix(s32 identity)
	{
		if ( identity )
		{
			Identity();
		}
	}
	~Matrix(void) {;}

	void ProjPersp(f32 fov, f32 aspect, f32 fZNear=0.1f, f32 fZFar=1000.0f, f32 fSkew=0.0f);
	void ProjOrtho(f32 w, f32 h, f32 zMin=0.1f, f32 zMax=1000.0f);
	void LookAt(Vector3& eye, Vector3& at, Vector3& up);
	void Identity();
	void AxisAngle(Vector3& axis, f32 angle);
	void EulerToMatrix(f32 yaw, f32 pitch, f32 roll);
	void Scale(Vector3& scale);
	void Scale(f32 x, f32 y, f32 z);
	void AffineInverse();
	void Transpose();
	void Transpose33();
	void ApplyScale(Vector3& rvScale);
	void RotateX(f32 angle);
	void RotateY(f32 angle);
	void RotateZ(f32 angle);
	Matrix MatMul(Matrix& b);

	Vector3 TransformVector(Vector3& v);
	Vector3 TransformNormal(Vector3& v);
	Vector4 TransformVector(Vector4& v);

	f32 *GetFloatPtr() { return &m[0]; }

public:
	static Matrix s_Identity;
};

#endif //MATRIX_H
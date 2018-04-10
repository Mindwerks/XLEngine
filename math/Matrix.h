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

    float m[16];

    Matrix() {;}
    Matrix(int32_t identity)
    {
        if ( identity )
        {
            Identity();
        }
    }
    ~Matrix() {;}

    void ProjPersp(float fov, float aspect, float fZNear=0.1f, float fZFar=1000.0f, float fSkew=0.0f);
    void ProjOrtho(float w, float h, float zMin=0.1f, float zMax=1000.0f);
    void LookAt(Vector3& eye, Vector3& at, Vector3& up);
    void Identity();
    void AxisAngle(Vector3& axis, float angle);
    void EulerToMatrix(float yaw, float pitch, float roll);
    void Scale(Vector3& scale);
    void Scale(float x, float y, float z);
    void AffineInverse();
    void Transpose();
    void Transpose33();
    void ApplyScale(Vector3& rvScale);
    void RotateX(float angle);
    void RotateY(float angle);
    void RotateZ(float angle);
    Matrix MatMul(Matrix& b);

    Vector3 TransformVector(Vector3& v);
    Vector3 TransformNormal(Vector3& v);
    Vector4 TransformVector(Vector4& v);

    float *GetFloatPtr() { return &m[0]; }

public:
    static Matrix s_Identity;
};

#endif //MATRIX_H
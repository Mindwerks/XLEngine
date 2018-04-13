#include "Matrix.h"

Matrix Matrix::s_Identity(1);

void Matrix::ProjPersp(float fov, float aspect, float fZNear, float fZFar, float fSkew)
{
    float yScale = 1.0f / tanf(fov*0.5f);
    float xScale = yScale / aspect;

    m[0] = xScale;
    m[1] = m[2] = m[3] = 0.0f;

    m[4] = 0.0f;
    m[5] = yScale;
    m[6] = 0.0f;
    m[7] = 0.0f;

    m[8] = 0.0f;
    m[9] = fSkew;
    m[10] = (fZFar + fZNear) / (fZNear - fZFar);
    m[11] = -1.0f;

    m[12] = m[13] = 0.0f;
    m[14] = 2.0f * fZFar * fZNear / (fZNear - fZFar);
    m[15] = 0.0f;
}

void Matrix::ProjOrtho(float w, float h, float zMin, float zMax)
{
    m[0] = 2.0f/w;
    m[1] = 0.0f;
    m[2] = 0.0f;
    m[3] = 0.0f;

    m[4] = 0.0f;
    m[5] = -2.0f/h;
    m[6] = 0.0f;
    m[7] = 0.0f;

    m[8] = 0.0f;
    m[9] = 0.0f;
    m[10] = -2.0f / (zMax - zMin);
    m[11] =  0.0f;

    m[12] = -1.0f;
    m[13] =  1.0f;
    m[14] = (zMax + zMin) / (zMin - zMax);
    m[15] = 1.0f;
}

void Matrix::LookAt(Vector3& eye, Vector3& at, Vector3& up)
{
    this->Identity();

    Vector3 F;
    F.Set(at.x - eye.x, at.y - eye.y, at.z - eye.z);
    F.Normalize();

    Vector3 R;
    R.Cross(up, F);
    R.Normalize();

    Vector3 U;
    U.Cross(F, R);
    U.Normalize();

    Vector3 P;
    P.Set( R.Dot(eye), -U.Dot(eye), F.Dot(eye) );

    m[0] = -R.x; m[4] = -R.y; m[ 8] = -R.z; m[12] = P.x;
    m[1] =  U.x; m[5] =  U.y; m[ 9] =  U.z; m[13] = P.y;
    m[2] = -F.x; m[6] = -F.y; m[10] = -F.z; m[14] = P.z;
    m[3] = 0.0f; m[7] = 0.0f; m[11] = 0.0f; m[15] = 1.0f;
}

void Matrix::Identity()
{
    for (int i=0; i<16; i++) { m[i] = 0.0f; }
    m[0] = m[5] = m[10] = m[15] = 1.0f;
}

void Matrix::AxisAngle(Vector3& axis, float angle)
{
    Matrix rot;
    rot.Identity();

    float ca = cosf(angle);
    float sa = sinf(angle);

    rot.m[0] = ca + axis.x*axis.x*(1.0f - ca);        rot.m[4] = axis.x*axis.y*(1.0f - ca) - axis.z*sa; rot.m[8]  = axis.x*axis.z*(1.0f - ca) + axis.y*sa;
    rot.m[1] = axis.y*axis.x*(1.0f - ca) + axis.z*sa; rot.m[5] = ca + axis.y*axis.y*(1.0f - ca);        rot.m[9]  = axis.y*axis.z*(1.0f - ca) - axis.x*sa;
    rot.m[2] = axis.z*axis.x*(1.0f - ca) - axis.y*sa; rot.m[6] = axis.z*axis.y*(1.0f - ca) + axis.x*sa; rot.m[10] = ca + axis.z*axis.z*(1.0f - ca);

    *this = this->MatMul(rot);
}

//RYP X
//RPY X
//YRP X
//YPR X
//PRY 
//PYR

void Matrix::EulerToMatrix(float yaw, float pitch, float roll)
{
    Vector3 vPitch = Vector3::UnitX;
    Vector3 vYaw = Vector3::UnitY;
    Vector3 vRoll = Vector3::UnitZ;
    this->AxisAngle( vPitch, -pitch );
    this->AxisAngle( vYaw,  yaw );
    this->AxisAngle( vRoll,  roll );
}

void Matrix::Scale(Vector3& scale)
{
    Matrix sMtx;
    sMtx.Identity();

    sMtx.m[0]  = scale.x;
    sMtx.m[5]  = scale.y;
    sMtx.m[10] = scale.z;

    *this = this->MatMul(sMtx);
}

void Matrix::Scale(float x, float y, float z)
{
    Matrix sMtx;
    sMtx.Identity();

    sMtx.m[0]  = x;
    sMtx.m[5]  = y;
    sMtx.m[10] = z;

    *this = this->MatMul(sMtx);
}

void Matrix::AffineInverse()
{
    //assumes an affine matrix with no scale or shear.
    Matrix in = *this;

    Vector3 transl;
    transl.Set(in.m[12], in.m[13], in.m[14]);

    Transpose33();
    Vector3 X, Y, Z;
    X.Set(m[0], m[4], m[ 8]);
    Y.Set(m[1], m[5], m[ 9]);
    Z.Set(m[2], m[6], m[10]);
    Vector3 newTransl;
    newTransl.x = -transl.Dot( X );
    newTransl.y = -transl.Dot( Y );
    newTransl.z = -transl.Dot( Z );

    m[12] = newTransl.x;
    m[13] = newTransl.y;
    m[14] = newTransl.z;
    m[15] = 1.0f;
}

void Matrix::Transpose()
{
    Matrix in = *this;
    for (int y=0; y<4; y++)
    {
        for (int x=0; x<4; x++)
        {
            m[y*4+x] = in.m[x*4+y];
        }
    }
}

void Matrix::Transpose33()
{
    Matrix in = *this;
    m[1+0*4] = in.m[0+1*4];
    m[0+1*4] = in.m[1+0*4];
    m[2+0*4] = in.m[0+2*4];
    m[0+2*4] = in.m[2+0*4];
    m[1+2*4] = in.m[2+1*4];
    m[2+1*4] = in.m[1+2*4];
}

void Matrix::ApplyScale(Vector3& rvScale)
{
}

void Matrix::RotateX(float angle)
{
    Matrix rot;
    rot.Identity();
    float ca = cosf(angle), sa = sinf(angle);
    rot.m[5] =  ca; rot.m[ 6] = sa;
    rot.m[9] = -sa; rot.m[10] = ca;

    *this = this->MatMul(rot);
}

void Matrix::RotateY(float angle)
{
    Matrix rot;
    rot.Identity();
    float ca = cosf(angle), sa = sinf(angle);
    rot.m[0] = ca; rot.m[1] = 0.0f; rot.m[2]  = -sa;
    rot.m[8] = sa; rot.m[9] = 0.0f; rot.m[10] =  ca;

    *this = this->MatMul( rot );
}

void Matrix::RotateZ(float angle)
{
    Matrix rot;
    rot.Identity();
    float ca = cosf(angle), sa = sinf(angle);
    rot.m[0] =  ca; rot.m[1] = sa;
    rot.m[4] = -sa; rot.m[5] = ca;

    *this = this->MatMul( rot );
}

Matrix Matrix::MatMul(Matrix& b)
{
    Matrix temp;

    temp.m[ 0] = this->m[0] * b.m[ 0] + this->m[4] * b.m[ 1] + this->m[ 8] * b.m[ 2] + this->m[12] * b.m[ 3];
    temp.m[ 1] = this->m[1] * b.m[ 0] + this->m[5] * b.m[ 1] + this->m[ 9] * b.m[ 2] + this->m[13] * b.m[ 3];
    temp.m[ 2] = this->m[2] * b.m[ 0] + this->m[6] * b.m[ 1] + this->m[10] * b.m[ 2] + this->m[14] * b.m[ 3];
    temp.m[ 3] = this->m[3] * b.m[ 0] + this->m[7] * b.m[ 1] + this->m[11] * b.m[ 2] + this->m[15] * b.m[ 3];

    temp.m[ 4] = this->m[0] * b.m[ 4] + this->m[4] * b.m[ 5] + this->m[ 8] * b.m[ 6] + this->m[12] * b.m[ 7];
    temp.m[ 5] = this->m[1] * b.m[ 4] + this->m[5] * b.m[ 5] + this->m[ 9] * b.m[ 6] + this->m[13] * b.m[ 7];
    temp.m[ 6] = this->m[2] * b.m[ 4] + this->m[6] * b.m[ 5] + this->m[10] * b.m[ 6] + this->m[14] * b.m[ 7];
    temp.m[ 7] = this->m[3] * b.m[ 4] + this->m[7] * b.m[ 5] + this->m[11] * b.m[ 6] + this->m[15] * b.m[ 7];

    temp.m[ 8] = this->m[0] * b.m[ 8] + this->m[4] * b.m[ 9] + this->m[ 8] * b.m[10] + this->m[12] * b.m[11];
    temp.m[ 9] = this->m[1] * b.m[ 8] + this->m[5] * b.m[ 9] + this->m[ 9] * b.m[10] + this->m[13] * b.m[11];
    temp.m[10] = this->m[2] * b.m[ 8] + this->m[6] * b.m[ 9] + this->m[10] * b.m[10] + this->m[14] * b.m[11];
    temp.m[11] = this->m[3] * b.m[ 8] + this->m[7] * b.m[ 9] + this->m[11] * b.m[10] + this->m[15] * b.m[11];

    temp.m[12] = this->m[0] * b.m[12] + this->m[4] * b.m[13] + this->m[ 8] * b.m[14] + this->m[12] * b.m[15];
    temp.m[13] = this->m[1] * b.m[12] + this->m[5] * b.m[13] + this->m[ 9] * b.m[14] + this->m[13] * b.m[15];
    temp.m[14] = this->m[2] * b.m[12] + this->m[6] * b.m[13] + this->m[10] * b.m[14] + this->m[14] * b.m[15];
    temp.m[15] = this->m[3] * b.m[12] + this->m[7] * b.m[13] + this->m[11] * b.m[14] + this->m[15] * b.m[15];

    return temp;
}

Vector3 Matrix::TransformVector(Vector3& v)
{
    Vector3 out;
    out.x = v.x*this->m[0+0] + v.y*this->m[4+0] + v.z*this->m[8+0] + this->m[12+0];
    out.y = v.x*this->m[0+1] + v.y*this->m[4+1] + v.z*this->m[8+1] + this->m[12+1];
    out.z = v.x*this->m[0+2] + v.y*this->m[4+2] + v.z*this->m[8+2] + this->m[12+2];

    return out;
}

Vector3 Matrix::TransformNormal(Vector3& v)
{
    Vector3 out;
    out.x = v.x*this->m[0+0] + v.y*this->m[4+0] + v.z*this->m[8+0];
    out.y = v.x*this->m[0+1] + v.y*this->m[4+1] + v.z*this->m[8+1];
    out.z = v.x*this->m[0+2] + v.y*this->m[4+2] + v.z*this->m[8+2];

    return out;
}

Vector4 Matrix::TransformVector(Vector4& v)
{
    Vector4 out;
    out.x = v.x*this->m[0+0] + v.y*this->m[4+0] + v.z*this->m[8+0] + v.w*this->m[12+0];
    out.y = v.x*this->m[0+1] + v.y*this->m[4+1] + v.z*this->m[8+1] + v.w*this->m[12+1];
    out.z = v.x*this->m[0+2] + v.y*this->m[4+2] + v.z*this->m[8+2] + v.w*this->m[12+2];
    out.w = v.x*this->m[0+3] + v.y*this->m[4+3] + v.z*this->m[8+3] + v.w*this->m[12+3];

    return out;
}

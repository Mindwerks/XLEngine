#ifndef PLANE_H
#define PLANE_H

#include "Vector3.h"
#include "Vector4.h"

typedef enum {
    NEGATIVE = -1,
    ON_PLANE = 0,
    POSITIVE = +1
} HalfSpace_e;

class Plane {
public:
    inline Plane(float fA = 0.0f, float fB = 0.0f, float fC = 0.0f, float fD = 0.0f) {
        a = fA;
        b = fB;
        c = fC;
        d = fD;
    }

    float Normalize();

    inline float Distance(Vector3 &vPt) const {
        return a * vPt.x + b * vPt.y + c * vPt.z + d;
    }

    inline float Dot(Vector3 &vPt) const {
        return a * vPt.x + b * vPt.y + c * vPt.z;
    }

    inline HalfSpace_e ClassifyPoint(const Vector3 &vPt) const;

    void Build(Vector3 *vPoints);

    bool Build(const Vector3 &v0, const Vector3 &v1, const Vector3 &v2);

    void FillVec4(Vector4 &rvVec) const { rvVec.Set(a, b, c, d); }

    float a, b, c, d;
};

#endif //PLANE_H
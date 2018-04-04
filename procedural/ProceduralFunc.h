#ifndef PROCEDURALFUNC_H
#define PROCEDURALFUNC_H

#include "../CommonTypes.h"
#include "../math/Vector3.h"

//Procedural Functions.
namespace ProceduralFunc
{
	float fBm(Vector3 p, int nOctaves, float H=0.5f, float r=2.0f);
	float Turbulance(Vector3 p, int nOctaves, float H=0.5f, float r=2.0f);
	float Ridged(Vector3 p, int nOctaves, float H=0.5f, float r=2.0f);
	float RidgedMulti(Vector3 p, int nOctaves, float power=2.0f, float H=0.5f, float r=2.0f);
};

#endif //PROCEDURALFUNC_H

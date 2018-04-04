#ifndef NOISE_H
#define NOISE_H

#include "../CommonTypes.h"
#include "../math/Math.h"

namespace Noise
{
	void Init();
	float Noise3D(float x, float y, float z);
};

#endif //NOISE_H

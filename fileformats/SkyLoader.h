#ifndef SKYLOADER_H_
#define SKYLOADER_H_

#include "../CommonTypes.h"

class SkyLoader
{
public:
	SkyLoader() {};
	virtual ~SkyLoader() {};

	virtual bool LoadSky(int32_t regionID) {return false;}
	virtual void *GetSkyData(int32_t regionID) {return 0;}
};

#endif //SKYLOADER_H_
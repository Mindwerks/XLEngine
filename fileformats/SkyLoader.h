#ifndef SKYLOADER_H_
#define SKYLOADER_H_

#include "../CommonTypes.h"

class SkyLoader
{
public:
	SkyLoader() {};
	virtual ~SkyLoader() {};

	virtual bool LoadSky(s32 regionID) {return false;}
	virtual void *GetSkyData(s32 regionID) {return 0;}
};

#endif //SKYLOADER_H_
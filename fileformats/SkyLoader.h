#ifndef SKYLOADER_H_
#define SKYLOADER_H_

#include "../CommonTypes.h"

class SkyLoader
{
public:
    SkyLoader() = default;
    virtual ~SkyLoader() = default;

    virtual bool LoadSky(int32_t regionID) {return false;}
    virtual void *GetSkyData(int32_t regionID) {return 0;}
};

#endif //SKYLOADER_H_
#ifndef DRIVER3D_IPLATFORM_H
#define DRIVER3D_IPLATFORM_H

#include "../Engine.h"

class Driver3D_IPlatform {
public:
    Driver3D_IPlatform() {}

    virtual ~Driver3D_IPlatform() {}

    virtual void SetWindowData(int32_t nParam, void **param) {}

    virtual void Present() {}

protected:
private:
};

#endif // DRIVER3D_IPLATFORM_H

#ifndef DRIVER3D_OGL_WIN_H
#define DRIVER3D_OGL_WIN_H

#include "../Driver3D_IPlatform.h"

class Driver3D_OGL_Win : public Driver3D_IPlatform {
public:
    Driver3D_OGL_Win();

    virtual ~Driver3D_OGL_Win();

    void SetWindowData(int32_t nParam, void **param);

    void Present();

protected:
private:
};

#endif // DRIVER3D_OGL_WIN_H

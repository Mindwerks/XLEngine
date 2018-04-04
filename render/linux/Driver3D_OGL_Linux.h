#ifndef DRIVER3D_OGL_LINUX_H
#define DRIVER3D_OGL_LINUX_H

#include "../Driver3D_OGL_IPlatform.h"

class Driver3D_OGL_Linux : public Driver3D_OGL_IPlatform
{
    public:
        Driver3D_OGL_Linux();
        virtual ~Driver3D_OGL_Linux();

        void SetWindowData(s32 nParam, void **param);

        void Present();
    protected:
    private:
};

#endif // DRIVER3D_OGL_LINUX_H

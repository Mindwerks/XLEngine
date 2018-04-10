#ifndef DRIVER3D_OGL_LINUX_H
#define DRIVER3D_OGL_LINUX_H

#include "../Driver3D_IPlatform.h"

class Driver3D_OGL_Linux : public Driver3D_IPlatform
{
    public:
        Driver3D_OGL_Linux();
        virtual ~Driver3D_OGL_Linux();

        void SetWindowData(int32_t nParam, void **param) override;

        void Present() override;
    protected:
    private:
};

#endif // DRIVER3D_OGL_LINUX_H

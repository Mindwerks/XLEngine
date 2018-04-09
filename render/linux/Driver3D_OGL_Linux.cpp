#include "Driver3D_OGL_Linux.h"

#include "SDL.h"

#include <cassert>

namespace
{

SDL_Window *m_hWnd;

} // namespace

Driver3D_OGL_Linux::Driver3D_OGL_Linux()
{
    m_hWnd = 0;
}

Driver3D_OGL_Linux::~Driver3D_OGL_Linux()
{
}

void Driver3D_OGL_Linux::Present()
{
    SDL_GL_SwapWindow(m_hWnd);  /* buffer swap does implicit glFlush */
}

void Driver3D_OGL_Linux::SetWindowData(int32_t nParam, void **param)
{
    assert(nParam == 1);
    m_hWnd = (SDL_Window*)param[0];
}

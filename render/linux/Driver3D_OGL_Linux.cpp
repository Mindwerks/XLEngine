#include "Driver3D_OGL_Linux.h"

#include <GL/glx.h>    /* this includes the necessary X headers */
#include <GL/gl.h>

#include <X11/X.h>    /* X11 constant (e.g. TrueColor) */
#include <X11/keysym.h>

#include <assert.h>

Display   *m_Display;
Window     m_hWnd;

Driver3D_OGL_Linux::Driver3D_OGL_Linux()
{
    m_Display = 0;
    m_hWnd = 0;
}

Driver3D_OGL_Linux::~Driver3D_OGL_Linux()
{

}

void Driver3D_OGL_Linux::Present()
{
    glXSwapBuffers(m_Display, m_hWnd);   /* buffer swap does implicit glFlush */
}

void Driver3D_OGL_Linux::SetWindowData(s32 nParam, void **param)
{
    m_Display = (Display *)param[0];
    m_hWnd = (Window)param[1];
}

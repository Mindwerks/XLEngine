#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>
#include "Driver3D_OGL_Win.h"
#include "../../EngineSettings.h"
#include "../../math/Math.h"

//WGL Extension crap... fortunately only on Windows.
typedef BOOL (WINAPI * PFNWGLSWAPINTERVALEXTPROC)(int interval);
typedef const char * (WINAPI * PFNWGLGETEXTENSIONSSTRINGEXTPROC)(void);
PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT = NULL;

HGLRC m_hRC;
HDC m_hDC;
HWND m_hWnd;

WORD m_GammaRamp_Default[3][256];
WORD m_GammaRamp[3][256];

bool _WGLExtensionSupported(const char *extension_name)
{
    // this is pointer to function which returns pointer to string with list of all wgl extensions
    PFNWGLGETEXTENSIONSSTRINGEXTPROC _wglGetExtensionsStringEXT = NULL;

    // determine pointer to wglGetExtensionsStringEXT function
    _wglGetExtensionsStringEXT = (PFNWGLGETEXTENSIONSSTRINGEXTPROC)wglGetProcAddress("wglGetExtensionsStringEXT");

    if (strstr(_wglGetExtensionsStringEXT(), extension_name) == NULL)
    {
        // string was not found
        return false;
    }

    // extension is supported
    return true;
}



Driver3D_OGL_Win::Driver3D_OGL_Win()
{
}

Driver3D_OGL_Win::~Driver3D_OGL_Win()
{
    //Restore the gamma ramp.
    if ( EngineSettings::get().IsFeatureEnabled(EngineSettings::FULLSCREEN) )
    {
        HDC hdc = GetDC(NULL);
        SetDeviceGammaRamp(hdc, m_GammaRamp_Default);
    }
}

void Driver3D_OGL_Win::Present()
{
    HDC hdc = GetDC(m_hWnd);
    SwapBuffers( hdc );
    ReleaseDC( m_hWnd, hdc );
}

void Driver3D_OGL_Win::SetWindowData(int32_t nParam, void **param)
{
    m_hWnd = (HWND)param[0];
    HDC m_hDC = GetDC(m_hWnd);

    BYTE bits = 32;
    static PIXELFORMATDESCRIPTOR pfd=                   // pfd Tells Windows How We Want Things To Be
    {
        sizeof(PIXELFORMATDESCRIPTOR),                  // Size Of This Pixel Format Descriptor
        1,                                              // Version Number
        PFD_DRAW_TO_WINDOW |                            // Format Must Support Window
        PFD_SUPPORT_OPENGL |                            // Format Must Support OpenGL
        PFD_DOUBLEBUFFER,                               // Must Support Double Buffering
        PFD_TYPE_RGBA,                                  // Request An RGBA Format
        bits,                                           // Select Our Color Depth
        0, 0, 0, 0, 0, 0,                               // Color Bits Ignored
        0,                                              // No Alpha Buffer
        0,                                              // Shift Bit Ignored
        0,                                              // No Accumulation Buffer
        0, 0, 0, 0,                                     // Accumulation Bits Ignored
        24,                                             // 24Bit Z-Buffer (Depth Buffer)
        8,                                              // 8Bit Stencil Buffer
        0,                                              // No Auxiliary Buffer
        PFD_MAIN_PLANE,                                 // Main Drawing Layer
        0,                                              // Reserved
        0, 0, 0                                         // Layer Masks Ignored
    };

    int32_t PixelFormat;
    if ( !(PixelFormat=ChoosePixelFormat(m_hDC,&pfd)) ) // Did Windows Find A Matching Pixel Format?
    {
        return;
    }

    if ( !SetPixelFormat(m_hDC,PixelFormat,&pfd) )      // Are We Able To Set The Pixel Format?
    {
        return;
    }

    m_hRC = wglCreateContext(m_hDC);
    if ( m_hRC == 0 )
    {
        return;
    }

    if ( !wglMakeCurrent(m_hDC,m_hRC) )                 // Try To Activate The Rendering Context
    {
        return;
    }

    //Now enable or disable VSYNC based on settings.
    //If the extension can't be found then we're forced to use whatever the driver default is...
    //but this shouldn't happen. If it does, the engine will still run at least. :)
    if ( _WGLExtensionSupported("WGL_EXT_swap_control") )
    {
        // Extension is supported, init pointer.
        // Save so that the swap interval can be changed later (to be implemented).
        wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress("wglSwapIntervalEXT");
    }

    if (wglSwapIntervalEXT)
    {
        wglSwapIntervalEXT( EngineSettings::get().IsFeatureEnabled(EngineSettings::VSYNC) ? 1 : 0 );
    }

    //Only setup the gamma ramp in fullscreen.
    if ( EngineSettings::get().IsFeatureEnabled( EngineSettings::FULLSCREEN) )
    {
        //get the current gamma ramp so it can be restored on exit.
        GetDeviceGammaRamp(m_hDC, m_GammaRamp_Default);

        float fBrightness, fContrast, fGamma;
        EngineSettings::get().GetDisplaySettings(fBrightness, fContrast, fGamma);

        //apply brightness, contrast and gamma.
        float fIntensity = 0.0f;
        float fValue;
        const float fInc = 1.0f / 255.0f;
        for (int i=0; i<256; i++)
        {
            //apply contrast
            fValue = fContrast*(fIntensity - 0.5f) + 0.5f;
            //apply brightness
            fValue = fBrightness*fValue;
            //apply gamma
            fValue = powf(fValue, fGamma);
            //clamp.
            fValue = Math::clamp(fValue, 0.0f, 1.0f);
            
            int intValue = ((int)(fValue*255.0f)) << 8;

            m_GammaRamp[0][i] = intValue;
            m_GammaRamp[1][i] = intValue;
            m_GammaRamp[2][i] = intValue;

            fIntensity += fInc;
        }
        SetDeviceGammaRamp(m_hDC, m_GammaRamp);
    }
}

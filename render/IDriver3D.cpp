#include "IDriver3D.h"
#include "../math/Vector3.h"
#include <stdio.h>
#include <malloc.h>

float LightObject::s_fAnimOffset0 = 0.0f;
float LightObject::s_fAnimOffset1 = 0.0f;

IDriver3D::IDriver3D()
{
    m_Platform    = NULL;
    m_uPaletteID  = 0;
    m_uColormapID = 0;
    m_nLightCnt   = 0;
    m_uExtensions = 0;
    m_uOverlayCount = 0;
    m_bForceMip   = false;
    m_bUpdatePal  = false;
    m_fAmbient    = 1.0f;
    //m_SunlightDir.Set(-1.0f, 1.0f, -1.0f);
    m_SunlightDir.Set(0.0f, 0.0f, -1.0f);
    m_SunlightDir.Normalize();
}

IDriver3D::~IDriver3D()
{
    //delete platform specific memory.
    if ( m_Platform )
    {
        xlDelete m_Platform;
        m_Platform = NULL;
    }
}

void IDriver3D::AddOverlay(int32_t x, int32_t y, int32_t scale, TextureHandle hTex)
{
    if ( m_uOverlayCount < MAX_OVERLAY_COUNT )
    {
        m_Overlays[m_uOverlayCount].x = x;
        m_Overlays[m_uOverlayCount].y = y;
        m_Overlays[m_uOverlayCount].scale = scale;
        m_Overlays[m_uOverlayCount].hTex = hTex;
        m_uOverlayCount++;
    }
}
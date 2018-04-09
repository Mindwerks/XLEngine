#include "FontManager.h"
#include "IDriver3D.h"
#include "Font.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "TextureCache.h"

#include <cassert>
#include <cstdlib>
#include <memory.h>

IDriver3D *FontManager::m_pDriver;
FontManager::FontMap FontManager::m_Fonts;

VertexBuffer *FontManager::m_pVB;
IndexBuffer *FontManager::m_pIB;

std::string FontManager::m_FontPath;

bool FontManager::Init(const std::string& szFontPath, IDriver3D *pDriver)
{
    m_pDriver  = pDriver;
    m_FontPath = szFontPath;

    if ( m_pDriver == NULL )
        return false;

    m_pVB = xlNew VertexBuffer(pDriver);
    if ( m_pVB->Create(sizeof(FontVertex), MAX_STRING_COUNT, true, IDriver3D::VBO_HAS_TEXCOORDS) == false )
    {
        xlDelete m_pVB;
        return false;
    }

    m_pIB = xlNew IndexBuffer(pDriver);
    if ( m_pIB->Create(MAX_STRING_COUNT*6, sizeof(uint16_t), false) == false )
    {
        xlDelete m_pVB;
        xlDelete m_pIB;
        return false;
    }
    //now fill the index buffer, since we're always rendering quads. The only difference is how many primitives that we render.
    uint16_t *pIB_Data = (uint16_t *)xlMalloc( sizeof(uint16_t)*MAX_STRING_COUNT*6 );
    if ( pIB_Data )
    {
        uint16_t vIdx=0;
        int32_t iIdx=0;
        for (int32_t q=0; q<MAX_STRING_COUNT; q++)
        {
            pIB_Data[iIdx++] = vIdx+0;
            pIB_Data[iIdx++] = vIdx+1;
            pIB_Data[iIdx++] = vIdx+2;

            pIB_Data[iIdx++] = vIdx+0;
            pIB_Data[iIdx++] = vIdx+2;
            pIB_Data[iIdx++] = vIdx+3;

            vIdx += 4;
        }
        m_pIB->Fill( (uint32_t *)pIB_Data );
        xlFree(pIB_Data);
    }
    else
    {
        xlDelete m_pVB;
        xlDelete m_pIB;
        return false;
    }

    return true;
}

void FontManager::Destroy()
{
    //delete the fonts.
    FontMap::iterator iFont = m_Fonts.begin();
    FontMap::iterator eFont = m_Fonts.end();
    for (; iFont != eFont; ++iFont)
    {
        xlDelete (*iFont).second;
    }
    m_Fonts.clear();

    if ( m_pVB )
    {
        xlDelete m_pVB;
        m_pVB = NULL;
    }
    if ( m_pIB )
    {
        xlDelete m_pIB;
        m_pIB = NULL;
    }
}

XLFont *FontManager::LoadFont(const std::string& szFile)
{
    // Don't load the same font twice
    FontMap::iterator iFont = m_Fonts.find(szFile);
    if ( iFont != m_Fonts.end() )
    {
        return iFont->second;
    }

    // Set the current TextureCache path.
    TextureCache::SavePath();
    TextureCache::SetPath( m_FontPath );
    // The font doesn't exist, we can try loading it now...
    XLFont *pFont = xlNew XLFont();
    if ( pFont && pFont->Load( m_FontPath+szFile, m_pDriver ) )
    {
        //Assign it to the map so we only load it once.
        m_Fonts[szFile] = pFont;

        TextureCache::RestorePath();

        //Return our texture handle
        return pFont;
    }
    TextureCache::RestorePath();
    return NULL;
}

void FontManager::BeginTextRendering()
{
    m_pDriver->EnableDepthRead(false);
    m_pDriver->EnableDepthWrite(false);
    m_pDriver->EnableCulling(false);
    m_pDriver->SetBlendMode( IDriver3D::BLEND_ALPHA );
}

void FontManager::EndTextRendering()
{
    m_pDriver->SetBlendMode();
    m_pDriver->SetTexture(0, XL_INVALID_TEXTURE);
    m_pDriver->SetColor( &Vector4::One );
}

void FontManager::RenderString(int32_t x, int32_t y, const std::string& szString, XLFont *pFont, Vector4 *pColor)
{
    TextureHandle hTex = pFont->GetTexture();

    FontVertex *pVB_Data = (FontVertex *)m_pVB->Lock();
    if ( pVB_Data )
    {
        int32_t nCharCount = pFont->FillVB(x, y, szString, pVB_Data);
        m_pVB->Unlock();

        if ( nCharCount > 0 )
        {
            m_pVB->Set();
            m_pDriver->SetTexture(0, hTex, IDriver3D::FILTER_NORMAL_NO_MIP);
            m_pDriver->SetColor( pColor );
            m_pDriver->RenderIndexedTriangles( m_pIB, nCharCount*2 );
        }
    }
    else
    {
        m_pVB->Unlock();
    }
}

uint32_t FontManager::GetLength(const std::string& szString, uint32_t uPosInString, XLFont *pFont)
{
    uint32_t uPos = uPosInString;
    uint32_t uStrLen = (uint32_t)szString.size();

    if ( uStrLen < 1 )
        return 0;

    if ( uStrLen < uPos ) uPos = (uint32_t)(uStrLen-1);

    return pFont->ComputePixelPos(szString, uPos);
}

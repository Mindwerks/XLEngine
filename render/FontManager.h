#ifndef FONTMANAGER_H
#define FONTMANAGER_H

#include "../CommonTypes.h"
#include "../math/Vector4.h"

#include <string>
#include <map>

class XLFont;
class IDriver3D;
class VertexBuffer;
class IndexBuffer;

#define MAX_STRING_COUNT 512

class FontManager
{
    typedef std::map<std::string, XLFont *> FontMap;

public:
    static bool Init(const std::string& szFontPath, IDriver3D *pDriver);
    static void Destroy();

    static XLFont *LoadFont(const std::string& szFile);

    //Set render states for text rendering.
    static void BeginTextRendering();
    static void EndTextRendering();
    //Render a string at location(x,y) using font pFont
    static void RenderString(int32_t x, int32_t y, const std::string& szString, XLFont *pFont, Vector4 *pColor=&Vector4::One);
    static uint32_t GetLength(const std::string& szString, uint32_t uPosInString, XLFont *pFont);

private:
    static IDriver3D *m_pDriver;
    static VertexBuffer *m_pVB;
    static IndexBuffer *m_pIB;

    static FontMap m_Fonts;
    static std::string m_FontPath;
};

#endif //FONTMANAGER_H

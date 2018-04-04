#ifndef FONTMANAGER_H
#define FONTMANAGER_H

#include "../CommonTypes.h"
#include "../math/Vector4.h"

#include <string>
#include <map>

using namespace std;

class XLFont;
class IDriver3D;
class VertexBuffer;
class IndexBuffer;

#define MAX_STRING_COUNT 512

class FontManager
{
	typedef map<string, XLFont *> FontMap;

public:
	static bool Init(const string& szFontPath, IDriver3D *pDriver);
	static void Destroy();

	static XLFont *LoadFont(const string& szFile);

	//Set render states for text rendering.
	static void BeginTextRendering();
	static void EndTextRendering();
	//Render a string at location(x,y) using font pFont
	static void RenderString(s32 x, s32 y, const string& szString, XLFont *pFont, Vector4 *pColor=&Vector4::One);
	static u32 GetLength(const string& szString, u32 uPosInString, XLFont *pFont);

private:
	static IDriver3D *m_pDriver;
	static VertexBuffer *m_pVB;
	static IndexBuffer *m_pIB;

	static FontMap m_Fonts;
	static string m_FontPath;
};

#endif //FONTMANAGER_H

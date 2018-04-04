#ifndef FONT_H
#define FONT_H

#include "../CommonTypes.h"
#include "../math/Vector2.h"
#include "../math/Vector3.h"
#include <string>

using namespace std;

class IDriver3D;

struct FontVertex
{
	Vector3 pos;
	Vector2 uv;
};

struct CharDescriptor
{
	//clean 16 bytes
	u16 x, y;
	u16 Width, Height;
	s16 XOffset, YOffset;
	s16 XAdvance;
	u16 Page;

	CharDescriptor() : x( 0 ), y( 0 ), Width( 0 ), Height( 0 ), XOffset( 0 ), YOffset( 0 ),
		XAdvance( 0 ), Page( 0 )
	{ }
};

struct Charset
{
	u16 LineHeight;
	u16 Base;
	u16 Width, Height;
	u16 Pages;
	CharDescriptor Chars[256];
};

class XLFont
{
public:
	XLFont(void);
	~XLFont(void);

	bool Load( const string& szFile, IDriver3D *pDriver );
	TextureHandle GetTexture() { return m_hTex; }
	s32 FillVB(s32 x, s32 y, const string& szString, FontVertex *pVB_Data);

	u32 ComputePixelPos(const string& szString, u32 uPos);

private:
	TextureHandle m_hTex;
	Charset m_CharSet;
};

#endif //FONT_H

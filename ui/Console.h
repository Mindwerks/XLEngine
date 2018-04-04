#ifndef CONSOLE_H
#define CONSOLE_H

#include <string>
#include <vector>
#include <list>
#include "../CommonTypes.h"
#include "../math/Vector4.h"

using namespace std;
class XLFont;
class IDriver3D;
class XL_Console;

class Console
{
public:
	enum ConsoleItemType
	{
		CTYPE_UCHAR=0,
		CTYPE_CHAR,
		CTYPE_UINT,
		CTYPE_INT,
		CTYPE_FLOAT,
		CTYPE_BOOL,
		CTYPE_STRING,
		CTYPE_CSTRING,
		CTYPE_VEC3,
		CTYPE_VEC4,
		CTYPE_FUNCTION,
		CTYPE_COUNT
	};

	typedef void (*ConsoleFunction)(const vector<string>&, void *);

public:
	Console(IDriver3D *pDriver3D);
	~Console(void);

	void SetGameInfo(const string& gameName, s32 versionMinor, s32 versionMajor);

	void AddItem(const string& itemName, void *ptr, ConsoleItemType type, const string& itemHelp, void *pUserData=NULL);
	void RemoveItem(const string& itemName);

	void SetDefaultCommand(ConsoleFunction func);
	void SetMaxCommands(int maxCmd) { m_MaxCommands = maxCmd; }
	void SetFont(XLFont *pFont) { m_pFont = pFont; }

	void Print(const string& text);
	void PrintCommandHelp(const string& cmd);

	void PassKey(char key);
	void PassEnter();
	void PassBackspace();
	void PassVirtualKey(s32 key);

	void Render();

	bool IsActive() { return m_bActive; }
	bool IsPaused() { return m_bPaused; }
	bool IsChatActive() { return m_bChatMode; }
	void EnableCommandEcho(bool bEnable) { m_bEchoCommands = bEnable; }

	void PrintCommands(const char *pszText=NULL);
	void LoadNewBackground(const char *pszBackground);

private:
	bool ParseCommandLine();

public:
	struct ConsoleItem
	{
		string name;
		string help;
		ConsoleItemType type;

		union
		{
			void *varPtr;
			ConsoleFunction func;
		};
		void *userData;
	};

	vector<string> m_CommandBuffer;
	list<ConsoleItem> m_ItemList;
	vector<string> m_TextBuffer;

	ConsoleFunction m_DefaultCommand;
	string m_CommandLine;
	char m_szGameInfo[128];

	IDriver3D *m_pDriver;

	u32 m_MaxCommands;
	u32 m_MaxTextLines;
	bool m_bEchoCommands;
	bool m_bPaused;
	s32 m_nCommandHistory;
	s32 m_nScrollOffs;
	s32 m_nBlinkFrame;
	u32 m_CaretPos;
	bool m_bActive;
	bool m_bChatMode;

	f32 m_fAnimDropDown;
	f32 m_fAnimDelta;

	TextureHandle m_hBackground;

	XLFont *m_pFont;
	Vector4 m_Color;

private:
	static bool _Compare_nocase(ConsoleItem first, ConsoleItem second);
};

#endif //CONSOLE_H

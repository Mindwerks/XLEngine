#ifndef XLCONSOLE_H
#define XLCONSOLE_H

#include "Console.h"
#include "../os/Input.h"
class Font;

class XL_Console
{
public:
	static bool Init(IDriver3D *pDriver);
	static void Destroy();

	static void Render();

	static void RegisterCmd(const string& itemName, void *ptr, Console::ConsoleItemType type, const string& itemHelp, void *pUserData);

	static void Print(const string& szMsg);
	static void PrintF(const char *pszString, ...);

	static bool IsActive();
	static bool IsPaused();
	static bool IsChatActive();

	static void SetGameInfo(const string& gameName, s32 versionMinor, s32 versionMajor) { s_pConsole->SetGameInfo(gameName, versionMinor, versionMajor); }
	static void SetConsoleColor(f32 fRed, f32 fGreen, f32 fBlue, f32 fAlpha);

private:
	static Console *s_pConsole;
	static void _DefaultConsoleFunc(const vector<string>& args, void *pUserData);
	static void _Echo(const vector<string>& args, void *pUserData);
	static void _CmdList(const vector<string>& args, void *pUserData);
	static void _Help(const vector<string>& args, void *pUserData);
	static void _ConsoleTex(const vector<string>& args, void *pUserData);

	static void _KeyDownCallback(s32 key);
	static void _CharDownCallback(s32 key);
};

#endif //XLCONSOLE_H
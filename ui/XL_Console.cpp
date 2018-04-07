#include "XL_Console.h"
#include <stdio.h>
#include <stdarg.h>

Console *XL_Console::s_pConsole;
static char _tmpStr[512];

void XL_Console::_DefaultConsoleFunc(const vector<string>& args, void *pUserData)
{
	string errorStr;
	errorStr = "^1'" + args[0] + "' is not a recognized command.";

	s_pConsole->Print(errorStr);
}

void XL_Console::_Echo(const vector<string>& args, void *pUserData)
{
	s_pConsole->Print(args[1]);
}

void XL_Console::_CmdList(const vector<string>& args, void *pUserData)
{
	if ( args.size() <= 1 )
		s_pConsole->PrintCommands();
	else
		s_pConsole->PrintCommands(args[1].c_str());
}

void XL_Console::_ConsoleTex(const vector<string>& args, void *pUserData)
{
	if ( args.size() > 1 )
	{
		s_pConsole->LoadNewBackground( args[1].c_str() );
	}
}

void XL_Console::_Help(const vector<string>& args, void *pUserData)
{
	if ( args.size() == 1 )
	{
		s_pConsole->Print("^8---------------------------- Console Help ------------------------------");
		s_pConsole->Print("^6To view the list of commands type: cmdlist");
		s_pConsole->Print("^8To get help for a specific command type: help command");
		s_pConsole->Print("^8To select previous commands, use the up and down arrows.");
		s_pConsole->Print("^8To scroll the text window, use the Page Up and Page Down keys.");
		s_pConsole->Print("^8------------------------------------------------------------------------");
	}
	else
	{
		s_pConsole->PrintCommandHelp(args[1]);
	}
}

bool XL_Console::Init(IDriver3D *pDriver)
{
	s_pConsole = xlNew Console(pDriver);
	s_pConsole->SetDefaultCommand( _DefaultConsoleFunc );
	s_pConsole->AddItem("echo", (void *)_Echo, Console::CTYPE_FUNCTION, "Echo the argument to the console.");
	s_pConsole->AddItem("cmdlist", (void *)_CmdList, Console::CTYPE_FUNCTION, "Show all console commands.");
	s_pConsole->AddItem("help", (void *)_Help, Console::CTYPE_FUNCTION,
		"Show console help. If an argument is passed, it shows help for a specific command. '>help cmdList' will show help for the 'cmdList' command");
	s_pConsole->AddItem("con_cmdecho", &s_pConsole->m_bEchoCommands, Console::CTYPE_BOOL,
		"Sets whether command echo is enabled (0/1). If enabled, the command line is echoed in the console when enter is pressed.");
	s_pConsole->AddItem("g_pause", &s_pConsole->m_bPaused, Console::CTYPE_BOOL, "Pauses the game if set to 1 (0/1).");
	s_pConsole->AddItem("con_color", &s_pConsole->m_Color, Console::CTYPE_VEC4, "Console color (RGBA).");
	s_pConsole->AddItem("con_tex", (void *)_ConsoleTex, Console::CTYPE_FUNCTION, "Load a new console background texture.");

	Input::AddKeyDownCallback( _KeyDownCallback );
	Input::AddCharDownCallback( _CharDownCallback );

	return (s_pConsole ? true : false);
}

void XL_Console::Destroy()
{
	xlDelete s_pConsole;
}

void XL_Console::_KeyDownCallback(int32_t key)
{
	if ( s_pConsole == NULL )
		return;

	if ( key == XL_BACK )
	{
		s_pConsole->PassBackspace();
	}
	else if ( key == XL_RETURN )
	{
		s_pConsole->PassEnter();
	}
	else
	{
		s_pConsole->PassVirtualKey(key);
	}
}

void XL_Console::_CharDownCallback(int32_t key)
{
	if ( s_pConsole == NULL )
		return;

	s_pConsole->PassKey((char)key);
}

void XL_Console::Render()
{
	if ( s_pConsole == NULL )
		return;

	s_pConsole->Render();
}

void XL_Console::RegisterCmd(const string& itemName, void *ptr, Console::ConsoleItemType type, const string& itemHelp, void *pUserData)
{
	s_pConsole->AddItem(itemName, ptr, type, itemHelp, pUserData);
}

bool XL_Console::IsActive()
{
	if ( s_pConsole == NULL )
		return false;

	return s_pConsole->IsActive();
}

bool XL_Console::IsChatActive()
{
	if ( s_pConsole == NULL )
		return false;

	return s_pConsole->IsChatActive();
}

bool XL_Console::IsPaused()
{
	if ( s_pConsole == NULL )
		return false;

	return s_pConsole->IsPaused();
}

void XL_Console::Print(const string& szMsg)
{
	if ( s_pConsole == NULL )
		return;

	s_pConsole->Print(szMsg);
}

void XL_Console::PrintF(const char *pszString, ...)
{
	if ( s_pConsole == NULL )
		return;

	va_list args;
	va_start(args, pszString);
#if PLATFORM_WIN
    _vsnprintf( _tmpStr, 512, pszString, args );
#else
	vsnprintf( _tmpStr, 512, pszString, args );
#endif
	va_end(args);

	s_pConsole->Print(_tmpStr);
}

/*******************************
  Plugin API
 *******************************/

void Console_RegisterCommand(const char *pszItemName, void *ptr, uint32_t type, const char *pszItemHelp, void *pUserData)
{
	XL_Console::RegisterCmd(pszItemName, ptr, (Console::ConsoleItemType)type, pszItemHelp, pUserData);
}

void XL_Console::SetConsoleColor(float fRed, float fGreen, float fBlue, float fAlpha)
{
	s_pConsole->m_Color.Set( fRed, fGreen, fBlue, fAlpha );
}
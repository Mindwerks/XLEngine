#include "Console.h"
#include "../render/IDriver3D.h"
#include "../render/FontManager.h"
#include "../render/TextureCache.h"
#include "../networking/NetworkMgr.h"
#include "../os/Input.h"
#include "../math/Math.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <sstream>
#include <iostream>

enum
{
	COLOR_BLACK=0,
	COLOR_RED,
	COLOR_GREEN,
	COLOR_DKGREEN,
	COLOR_YELLOW,
	COLOR_BLUE,
	COLOR_CYAN,
	COLOR_PURPLE,
	COLOR_ORANGE,
	COLOR_WHITE,
};

Vector4 m_aColorTable[]=
{
	Vector4(0.0f, 0.0f, 0.0f, 1.0f),
	Vector4(1.0f, 0.0f, 0.0f, 1.0f),
	Vector4(0.0f, 1.0f, 0.0f, 1.0f),
	Vector4(0.0f, 0.5f, 0.0f, 1.0f),
	Vector4(1.0f, 1.0f, 0.0f, 1.0f),
	Vector4(0.0f, 0.0f, 1.0f, 1.0f),
	Vector4(0.0f, 1.0f, 1.0f, 1.0f),
	Vector4(1.0f, 0.0f, 1.0f, 1.0f),
	Vector4(1.0f, 0.5f, 0.0f, 1.0f),
	Vector4(1.0f, 1.0f, 1.0f, 1.0f),
};

Console::Console(IDriver3D *pDriver)
{
	m_pDriver = pDriver;
	m_DefaultCommand = NULL;
	m_MaxCommands = 32;
	m_MaxTextLines = 256;
	m_pFont = NULL;
	m_nCommandHistory = -1;
	m_nScrollOffs = 0;
	m_bActive = false;
	m_bPaused = false;
	m_bChatMode = false;
	m_fAnimDropDown = 0.0f;
	m_fAnimDelta = 0.0f;
	m_nBlinkFrame = 0;
	m_CaretPos = 0;

	m_bEchoCommands = true;
	m_hBackground = XL_INVALID_TEXTURE;

	if ( pDriver )
	{
		m_pFont = FontManager::LoadFont( "Verdana16.fnt" );
	}
	m_Color.Set(0.20f, 0.20f, 0.20f, 0.85f);
}

Console::~Console(void)
{
}

bool Console::_Compare_nocase(ConsoleItem first, ConsoleItem second)
{
	int32_t cmp = stricmp( first.name.c_str(), second.name.c_str() );
	return (cmp < 0) ? true : false;
}

void Console::SetGameInfo(const string& gameName, int32_t versionMajor, int32_t versionMinor)
{
	sprintf(m_szGameInfo, "%s version %d.%03d", gameName.c_str(), versionMajor, versionMinor);
}

void Console::AddItem(const string& itemName, void *ptr, ConsoleItemType type, const string& itemHelp, void *pUserData)
{
	ConsoleItem item;
	item.name = itemName;
	item.help = itemHelp;
	item.type = type;
	if ( type == CTYPE_FUNCTION )
		item.func = (ConsoleFunction)ptr;
	else
		item.varPtr = ptr;
	item.userData = pUserData;

	m_ItemList.push_back(item);
	m_ItemList.sort( _Compare_nocase );
}

void Console::RemoveItem(const string& itemName)
{
	//add ability to remove an item, later...
}

void Console::SetDefaultCommand(ConsoleFunction func)
{
	m_DefaultCommand = func;
}

void Console::Print(const string& text)
{
	m_TextBuffer.push_back(text);
	if ( m_TextBuffer.size() > m_MaxTextLines )
	{
		m_TextBuffer.erase( m_TextBuffer.begin() );
	}
	//now print out to the console window.
	size_t l = text.size();
	const char *pszText = text.c_str();
	if ( pszText[0] == '^' )
	{
		if ( l > 2 ) printf("%s\n", &pszText[2]);
	}
	else
	{
		printf("%s\n", pszText);
	}
}

void Console::PrintCommandHelp(const string& cmd)
{
	bool bCmdFound = false;
	list<ConsoleItem>::const_iterator iter;
	for (iter = m_ItemList.begin(); iter != m_ItemList.end(); ++iter)
	{
		if ( (*iter).name == cmd )
		{
			Print("^8---------------------------------------------------");
			Print("^8" + (*iter).name + ": ");
			Print("^2" + (*iter).help);
			Print("^8---------------------------------------------------");
			bCmdFound = true;
			break;
		}
	}
	if ( bCmdFound == false )
	{
		Print("^1'" + cmd + "' Not Found, no help available.");
	}
}

void Console::PassKey(char key)
{
 	if ( m_bActive == false && m_bChatMode == false )
		return;

	if ( key >= ' ' && key <= '}' && key != '`' && m_CommandLine.size() < 90 )
	{
		if ( m_CaretPos == m_CommandLine.length() )
			m_CommandLine += key;
		else
			m_CommandLine.insert(m_CommandLine.begin()+m_CaretPos, (char)key);

		if ( m_CaretPos < m_CommandLine.length() ) m_CaretPos++;
	}
}

void Console::PassEnter()
{
	if ( m_bActive == false )
	{
		if ( Input::LockMouse() )
		{
			if ( m_bChatMode == true )
			{
				NetworkMgr::SendChatMessage(m_CommandLine.c_str());
				m_CommandLine.clear();
				m_CaretPos = 0;
			}
			m_bChatMode = !m_bChatMode;
		}
		return;
	}

	if ( m_CommandLine.length() > 0 )
	{
		ParseCommandLine();
		m_CommandLine.clear();
	}
	m_nCommandHistory = -1;
	m_CaretPos = 0;
}

void Console::PassBackspace()
{
	if ( m_bActive == false && m_bChatMode == false )
		return;

	size_t length = m_CommandLine.length();
	if ( length > 0 )
	{
		if ( m_CaretPos > 0 )
			m_CommandLine.erase( m_CaretPos-1, 1 );

		if ( m_CaretPos > 0 ) m_CaretPos--;
	}
}

void Console::PassVirtualKey(int32_t key)
{
	if ( m_bChatMode == false )
	{
		if ( key == XL_TILDE )
		{
			if ( m_bActive == false )
			{
				m_fAnimDropDown = 0.0f;
				m_fAnimDelta = 0.1f;
				m_bActive = true;
			}
			else
			{
				m_fAnimDropDown = 1.0f;
				m_fAnimDelta = -0.1f;
			}
		}
		if ( m_bActive == false )
			return;

		if ( key == XL_UP )
		{
			if ( m_nCommandHistory != 0 ) m_nCommandHistory--;
			if ( m_nCommandHistory < -1 )
			{
				m_nCommandHistory = m_CommandBuffer.size()-1;
			}
			if ( m_nCommandHistory == -1 )
			{
				m_CommandLine.clear();
			}
			else
			{
				m_CommandLine = m_CommandBuffer[m_nCommandHistory];
			}
			m_CaretPos = m_CommandLine.length();
		}
		else if ( key == XL_DOWN )
		{
			if ( m_nCommandHistory > -1 )
				m_nCommandHistory++;

			if ( m_nCommandHistory >= (int32_t)m_CommandBuffer.size() )
				m_nCommandHistory = -1;

			if ( m_nCommandHistory == -1 )
			{
				m_CommandLine.clear();
			}
			else
			{
				m_CommandLine = m_CommandBuffer[m_nCommandHistory];
			}
			m_CaretPos = m_CommandLine.length();
		}
	}
	if ( key == XL_DELETE )
	{
		size_t length = m_CommandLine.length();
		if ( length > 0 )
		{
			if ( m_CaretPos < m_CommandLine.length() )
				m_CommandLine.erase( m_CaretPos, 1 );
		}
	}

	if ( key == XL_LEFT )
	{
		if ( m_CaretPos > 0 ) m_CaretPos--;
	}
	else if ( key == XL_RIGHT )
	{
		if ( m_CaretPos < m_CommandLine.length() ) m_CaretPos++;
	}

	if ( key == XL_HOME )
	{
		m_CaretPos = 0;
	}
	else if ( key == XL_END )
	{
		m_CaretPos = m_CommandLine.length();
	}

	if ( m_bChatMode == false )
	{
		if ( key == XL_PRIOR )
		{
			m_nScrollOffs -= 3;
			if ( m_nScrollOffs < 23-(int32_t)m_TextBuffer.size() )
				m_nScrollOffs = 23-(int32_t)m_TextBuffer.size();
			if ( m_nScrollOffs > 0 )
				m_nScrollOffs = 0;
		}
		else if ( key == XL_NEXT )
		{
			m_nScrollOffs += 3;
			if ( m_nScrollOffs > 0 )
				m_nScrollOffs = 0;
		}
	}
}

void Console::LoadNewBackground(const char *pszBackground)
{
	m_hBackground = TextureCache::LoadTexture(pszBackground, false);
}

void Console::Render()
{
	if ( m_bActive == false && m_bChatMode == false )
		return;

	if ( m_hBackground == XL_INVALID_TEXTURE )
	{
		m_hBackground = TextureCache::LoadTexture("ConsoleBackground.png", false);
	}

	if ( m_bActive && m_fAnimDelta != 0.0f )
	{
		m_fAnimDropDown += m_fAnimDelta;
		if ( m_fAnimDelta < 0.0f )
		{
			if ( m_fAnimDropDown <= 0.0f )
			{
				m_fAnimDropDown = 0.0f;
				m_fAnimDelta = 0.0f;
				m_bActive = false;
				return;
			}
		}
		else
		{
			if ( m_fAnimDropDown >= 1.0f )
			{
				m_fAnimDropDown = 1.0f;
				m_fAnimDelta = 0.0f;
			}
		}
	}

	int32_t nScrWidth, nScrHeight;
	m_pDriver->GetWindowSize(nScrWidth, nScrHeight);

	//Draw background.
	static char szFinalTextLine[256];
	int32_t yOffs = 0;
	int32_t yBase = 460;
	if ( m_bActive )
	{
		m_pDriver->EnableDepthRead(false);
		m_pDriver->EnableDepthWrite(false);
		m_pDriver->EnableAlphaTest(false);
		m_pDriver->EnableCulling(false);
		m_pDriver->SetBlendMode( IDriver3D::BLEND_ALPHA );

		{
			float y2 = m_fAnimDropDown*512.0f;

			Vector4 posScale(0, 0, (float)nScrWidth, (float)y2);
			Vector2 uvTop(0, 1-m_fAnimDropDown+0.004f), uvBot(1, 1);

			m_pDriver->SetTexture(0, m_hBackground, IDriver3D::FILTER_NORMAL_NO_MIP);
			m_pDriver->RenderScreenQuad(posScale, uvTop, uvBot, m_Color, m_Color);
		}
	
		FontManager::BeginTextRendering();

		//now print stuff.
		int32_t start = 0;
		if ( m_TextBuffer.size() > 23 )
		{
			start = (int32_t)m_TextBuffer.size() - 23;
			start += m_nScrollOffs;
		}

		yOffs = (int32_t)( -(1.0f-m_fAnimDropDown)*480.0f );

		for (int32_t y=0; y<23 && y<(int32_t)m_TextBuffer.size(); y++)
		{
			const char *pszTextLine = m_TextBuffer[y+start].c_str();
			int32_t l = (int32_t)strlen(pszTextLine);
			int32_t cIdx=0;
			int32_t color = COLOR_WHITE;
			for (int32_t c=0; c<l; c++)
			{
				if ( pszTextLine[c] == '^' && c < l-1 )
				{
					char szColor[2] = { pszTextLine[c+1], 0 };
					//only take the first color for now...
					if (color==COLOR_WHITE) color = atoi(szColor);
					c++;
				}
				else
				{
					szFinalTextLine[cIdx++] = pszTextLine[c];
				}
			}
			szFinalTextLine[cIdx] = 0;
			FontManager::RenderString( 5, y*20+2+(int32_t)yOffs, szFinalTextLine, m_pFont, &m_aColorTable[color] );
		}
	}
	else
	{
		yBase = nScrHeight - (nScrHeight>>2);

		m_pDriver->EnableDepthRead(false);
		m_pDriver->EnableDepthWrite(false);
		m_pDriver->EnableAlphaTest(false);
		m_pDriver->EnableCulling(false);
		m_pDriver->SetBlendMode( IDriver3D::BLEND_ALPHA );

		uint32_t uStrLen = FontManager::GetLength( m_CommandLine, m_CommandLine.size()-1, m_pFont);

		Vector4 posScale(0, (float)(yBase-5), (float)MIN(MAX((int32_t)uStrLen+32,512),nScrWidth), 24);
		Vector4 color(0,0,0,0.75f);
		Vector2 uvTop(0, 0), uvBot(1, 1);

		m_pDriver->SetTexture(0, XL_INVALID_TEXTURE);
		m_pDriver->RenderScreenQuad(posScale, uvTop, uvBot, color, color);

		FontManager::BeginTextRendering();
	}
	FontManager::RenderString(  1, yBase+yOffs, ">", m_pFont, &m_aColorTable[COLOR_YELLOW] );
	FontManager::RenderString( 10, yBase+yOffs, m_CommandLine, m_pFont, &m_aColorTable[COLOR_YELLOW] );
	
	if ( m_CaretPos > m_CommandLine.length() )
		 m_CaretPos = m_CommandLine.length();

	strcpy(szFinalTextLine, m_CommandLine.c_str());
	if ( m_CaretPos > 0 && szFinalTextLine[m_CaretPos-1] == ' ' )
		 szFinalTextLine[m_CaretPos-1] = '_';
	uint32_t length = FontManager::GetLength(szFinalTextLine, m_CaretPos, m_pFont);
	if ( m_nBlinkFrame < 30 )
	{
		FontManager::RenderString( 10+length, yBase+yOffs, "_", m_pFont, &m_aColorTable[COLOR_YELLOW] );
	}
	else
	{
		Vector4 dkYellow(m_aColorTable[COLOR_YELLOW].x*0.5f, m_aColorTable[COLOR_YELLOW].y*0.5f, m_aColorTable[COLOR_YELLOW].z*0.5f, 1.0f);
		FontManager::RenderString( 10+length, yBase+yOffs, "_", m_pFont, &dkYellow );
	}
	m_nBlinkFrame = (m_nBlinkFrame+1)%60;

	if ( m_bActive )
	{
		int32_t nGameInfoPos = nScrWidth-200;
		FontManager::RenderString( nGameInfoPos, yBase+20+yOffs, m_szGameInfo, m_pFont, &m_aColorTable[COLOR_GREEN] );
	}
	
	FontManager::EndTextRendering();
}

void Console::PrintCommands(const char *pszText/*=NULL*/)
{
	char szCmdText[256];
	size_t l=0;
	if ( pszText )
	{
		l = strlen(pszText);
		if ( pszText[l-1] == '*' || pszText[l-1] == '?' )
			l--;
	}

	list<ConsoleItem>::const_iterator iter;
	for (iter = m_ItemList.begin(); iter != m_ItemList.end(); ++iter)
	{
		if ( pszText != NULL && l > 0 )
		{
			if ( strnicmp(pszText, (*iter).name.c_str(), l) != 0 )
				continue;
		}
		sprintf(szCmdText, "^7%s", (*iter).name.c_str());
		Print(szCmdText);
	}
}

bool Console::ParseCommandLine()
{
	ostringstream out;
	string::size_type index = 0;
	vector<string> arguments;
	list<ConsoleItem>::const_iterator iter;

	//add to text buffer - command echo.
	if ( m_bEchoCommands )
		Print("^4>" + m_CommandLine);

	//add to the command buffer.
	m_CommandBuffer.push_back( m_CommandLine );
	if ( m_CommandBuffer.size() > m_MaxCommands )
		m_CommandBuffer.erase( m_CommandBuffer.begin() );

	//tokenize
	int32_t count;
	int32_t prev_index = 0;
	bool bInQuotes = false;
	string::size_type l = m_CommandLine.length();
	for ( string::size_type c=0; c<l; c++ )
	{
		if ( (m_CommandLine.at(c) == ' ' && bInQuotes == false) || c==l-1 )
		{
			count = c-prev_index;
			if ( c == l-1 ) //this will happen for the last argument if no extra space is added.
				count++;

			//remove the quotes from the argument. this makes arguments with and without quotes functionality identical
			//if there are no spaces.
			if ( m_CommandLine.at(prev_index) == '"' )
				prev_index++;
			if ( c > 0 && m_CommandLine.at(c-1) == '"' )
				count-=2;
			else if ( m_CommandLine.at(c) == '"' )
				count-=2;

			arguments.push_back( m_CommandLine.substr(prev_index, count) );
			prev_index = c+1;
		}
		else if ( m_CommandLine.at(c) == '"' )
		{
			bInQuotes = !bInQuotes;
		}
	}

	//execute (must look for a command or variable).
	for (iter = m_ItemList.begin(); iter != m_ItemList.end(); ++iter)
	{
		if ( iter->name == arguments[0] )
		{
			switch (iter->type)
			{
				case CTYPE_UCHAR:
					if ( arguments.size() > 2) return false;
					else if ( arguments.size() == 1)
					{
						out.str("");	//clear stringstream
						out << (*iter).name << " = " << *((unsigned char *)(*iter).varPtr);
						Print(out.str());
					}
					else
					{
						*((unsigned char *)(*iter).varPtr) = (unsigned char)atoi(arguments[1].c_str());
					}
					return true;
					break;
				case CTYPE_CHAR:
					if ( arguments.size() > 2) return false;
					else if ( arguments.size() == 1)
					{
						out.str("");	//clear stringstream
						out << (*iter).name << " = " << *((char *)(*iter).varPtr);
						Print(out.str());
					}
					else
					{
						*((char *)(*iter).varPtr) = (char)atoi(arguments[1].c_str());
					}
					return true;
					break;
				case CTYPE_UINT:
					if ( arguments.size() > 2) return false;
					else if ( arguments.size() == 1)
					{
						out.str("");	//clear stringstream
						out << (*iter).name << " = " << *((uint32_t *)(*iter).varPtr);
						Print(out.str());
					}
					else
					{
						*((uint32_t *)(*iter).varPtr) = (uint32_t)atoi(arguments[1].c_str());
					}
					return true;
					break;
				case CTYPE_INT:
					if ( arguments.size() > 2) return false;
					else if ( arguments.size() == 1)
					{
						out.str("");	//clear stringstream
						out << (*iter).name << " = " << *((int32_t *)(*iter).varPtr);
						Print(out.str());
					}
					else
					{
						*((int32_t *)(*iter).varPtr) = (int32_t)atoi(arguments[1].c_str());
					}
					return true;
					break;
				case CTYPE_FLOAT:
					if ( arguments.size() > 2) return false;
					else if ( arguments.size() == 1)
					{
						out.str("");	//clear stringstream
						out << (*iter).name << " = " << *((float *)(*iter).varPtr);
						Print(out.str());
					}
					else
					{
						*((float *)(*iter).varPtr) = (float)atof(arguments[1].c_str());
					}
					return true;
					break;
				case CTYPE_BOOL:
					if ( arguments.size() > 2) return false;
					else if ( arguments.size() == 1)
					{
						out.str("");	//clear stringstream
						out << (*iter).name << " = " << ( *((bool *)(*iter).varPtr) ? "1" : "0" );
						Print(out.str());
					}
					else
					{
						*((bool *)(*iter).varPtr) = atoi(arguments[1].c_str()) ? true : false;
					}
					return true;
					break;
				case CTYPE_STRING:
					if ( arguments.size() > 2) return false;
					else if ( arguments.size() == 1)
					{
						out.str("");	//clear stringstream
						out << (*iter).name << " = " << *((string *)(*iter).varPtr);
						Print(out.str());
					}
					else
					{
						*((string *)(*iter).varPtr) = arguments[1];
					}
					return true;
					break;
				case CTYPE_CSTRING:
					if ( arguments.size() > 2) return false;
					else if ( arguments.size() == 1)
					{
						out.str("");	//clear stringstream
						out << (*iter).name << " = " << string((char *)(*iter).varPtr);
						Print(out.str());
					}
					else
					{
						strcpy((char *)(*iter).varPtr, arguments[1].c_str());
					}
					return true;
					break;
				case CTYPE_VEC3:
					if ( arguments.size() > 4 ) return false;
					else if ( arguments.size() != 4 )
					{
						out.str("");	//clear stringstream
						out << (*iter).name << " = " << ((Vector3 *)(*iter).varPtr)->x << " " << ((Vector3 *)(*iter).varPtr)->y << " " <<
							((Vector3 *)(*iter).varPtr)->z;
						Print(out.str());
					}
					else
					{
						((Vector3 *)(*iter).varPtr)->x = (float)atof(arguments[1].c_str());
						((Vector3 *)(*iter).varPtr)->y = (float)atof(arguments[2].c_str());
						((Vector3 *)(*iter).varPtr)->z = (float)atof(arguments[3].c_str());
					}
					return true;
					break;
				case CTYPE_VEC4:
					if ( arguments.size() > 5 ) return false;
					else if ( arguments.size() != 5 )
					{
						out.str("");	//clear stringstream
						out << (*iter).name << " = " << ((Vector4 *)(*iter).varPtr)->x << " " << ((Vector4 *)(*iter).varPtr)->y << " " <<
							((Vector4 *)(*iter).varPtr)->z << " " << ((Vector4 *)(*iter).varPtr)->w;
						Print(out.str());
					}
					else
					{
						((Vector4 *)(*iter).varPtr)->x = (float)atof(arguments[1].c_str());
						((Vector4 *)(*iter).varPtr)->y = (float)atof(arguments[2].c_str());
						((Vector4 *)(*iter).varPtr)->z = (float)atof(arguments[3].c_str());
						((Vector4 *)(*iter).varPtr)->w = (float)atof(arguments[4].c_str());
					}
					return true;
					break;
				case CTYPE_FUNCTION:
					(*iter).func(arguments, (*iter).userData);
					return true;
					break;
				default:
					m_DefaultCommand(arguments, NULL);
					return false;
					break;
			}
		}
	}
	m_DefaultCommand(arguments, NULL);
	m_CommandLine.clear();

	return false;
}

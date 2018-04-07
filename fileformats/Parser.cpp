#include "Parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>

const char *Parser::m_pMemory=0;
uint32_t Parser::m_uFilePtr=0;
uint32_t Parser::m_uFilePtrSaved=0;
uint32_t Parser::m_uLength=0;
uint32_t Parser::m_SeqRange[2]={0,0};

void Parser::SetData(char *pMemory, uint32_t uLength, uint32_t uFilePtr)
{
	m_pMemory = pMemory;
	m_uLength = uLength;
	m_uFilePtr = uFilePtr;
}

int32_t Parser::SearchKeyword_int32_t(const char *pszKeyword, int32_t& rnValue)
{
	bool bCaseSensitive = true;
	bool bRet = false;
	char szValue[32]="";
	bool bStartSpace=false;
	bool bFound = false;
	bool bInComment = false;

	rnValue = 0;
	for (uint32_t c=m_uFilePtr; c<m_uLength; c++)
	{
		if ( c < m_uLength-1 && m_pMemory[c] == '/' && m_pMemory[c+1] == '*' )
		{
			bInComment = true;
			c++;
			continue;
		}

		if ( bInComment == true )
		{
			if ( c < m_uLength-1 && m_pMemory[c] == '*' && m_pMemory[c+1] == '/' )
			{
				bInComment = false;
				c++;
			}
			continue;
		}

		if ( bCaseSensitive )
		{
			if ( (!pszKeyword) || strncmp(&m_pMemory[c], pszKeyword, strlen(pszKeyword)) == 0 ) 
			{ 
				if ( m_pMemory[c+strlen(pszKeyword)] != ':' )
				{
					bFound = true; 
				}
			}
		}
		else
		{
			if ( (!pszKeyword) || strnicmp(&m_pMemory[c], pszKeyword, strlen(pszKeyword)) == 0 )
			{
				if ( m_pMemory[c+strlen(pszKeyword)] != ':' )
				{
					bFound = true; 
				}
			}
		}

		if (bFound)
		{
			//now search for a number or end of the line.
			int i=0, cc;
			for (cc=c+(int)strlen(pszKeyword); m_pMemory[cc] != '\r' && m_pMemory[cc] != '#' && ((m_pMemory[cc] != ' '&&m_pMemory[cc] != 9) || !bStartSpace); cc++, i++)
			{
				szValue[i] = m_pMemory[cc];
				if ( szValue[i] != ' ' && szValue[i] != 9 ) { bStartSpace = true; }
			}
			szValue[i] = 0;
			m_uFilePtr = cc;
			bRet = true;
			break;
		}
	}
	if ( bRet )
	{
		char *tmp;
		rnValue = (int32_t)strtol(szValue, &tmp, 10);
	}

	return bRet;
}

bool Parser::SearchKeyword_Str(const char *pszKeyword, char *pszOut, bool bEndOnSpace/*=true*/, bool bPreserveSpaces/*=false*/, int32_t maxSpaceCnt/*=4*/)
{
	bool bRet = false;
	char szValue[256]="";
	pszOut[0] = 0;
	bool bStartSpace=false;

	uint32_t uStart, uEnd;
	if ( m_SeqRange[0] != 0 || m_SeqRange[1] != 0 )
	{
		uStart = m_uFilePtr;
		uEnd = m_SeqRange[1];
	}
	else
	{
		uStart = m_uFilePtr;
		uEnd = m_uLength;
	}

	int32_t nSpaceCnt=0;
	bool bCommentStarted = false;

	int32_t i, c, cc;
	for (c=(int32_t)uStart; c<(int32_t)uEnd; c++)
	{
		if (!bCommentStarted && m_pMemory[c] == '/' && m_pMemory[c+1] == '*')
		{
			bCommentStarted = true;
		}
		else if ( bCommentStarted )
		{
			if ( m_pMemory[c-1] == '*' && m_pMemory[c] == '/' )
			{
				bCommentStarted = false;
			}
		}
		else if ( bCommentStarted == false && strnicmp(&m_pMemory[c], pszKeyword, strlen(pszKeyword)) == 0 )
		{
			//now search for a number or end of the line.
			for (cc=c+(int32_t)strlen(pszKeyword), i=0; m_pMemory[cc] != '\r' && m_pMemory[cc] != '#' && ((m_pMemory[cc] != ' ' && m_pMemory[cc] != 9) || !bStartSpace || (!bEndOnSpace && nSpaceCnt<maxSpaceCnt)); cc++)
			{
				if ( m_pMemory[cc] != ' ' && szValue[i] != 9 )
				{
					szValue[i++] = m_pMemory[cc];
					nSpaceCnt = 0;
					bStartSpace = true;
				}
				else if ( m_pMemory[cc] == ' ' && bStartSpace && bPreserveSpaces )
				{
					szValue[i++] = ' ';
					nSpaceCnt++;
				}
				else if ( m_pMemory[cc] == ' ' && bStartSpace )
				{
					nSpaceCnt++;
				}
			}
			szValue[i] = 0;
			m_uFilePtr = (uint32_t)cc+1;
			bRet = true;
			break;
		}
	}
	if ( bRet )
	{
		strcpy(pszOut, szValue);
	}
	
	return bRet;
}

bool Parser::SearchKeyword_F1(const char *pszKeyword, float& rfValue)
{
	bool bRet = false;
	char szValue[32]="";
	rfValue = 0;
	bool bStartSpace=false;
	int32_t i, c, cc;
	for (c=(int32_t)m_uFilePtr; c<(int32_t)m_uLength; c++)
	{
		if ( (!pszKeyword) || strnicmp(&m_pMemory[c], pszKeyword, strlen(pszKeyword)) == 0 )
		{
			//now search for a number or end of the line.
			for (cc=c+(int)strlen(pszKeyword), i=0; m_pMemory[cc] != '\r' && m_pMemory[cc] != '#' && ((m_pMemory[cc] != ' ' && m_pMemory[cc] != 9) || !bStartSpace); cc++, i++)
			{
				szValue[i] = m_pMemory[cc];
				if ( szValue[i] != ' ' && szValue[i] != 9 ) { bStartSpace = true; }
			}
			szValue[i] = 0;
			m_uFilePtr = cc;
			bRet = true;
			break;
		}
	}
	if ( bRet )
	{
		char *tmp;
		rfValue = (float)strtod(szValue, &tmp);
	}
	
	return bRet;
}

bool Parser::SearchKeyword_Next(int32_t& rnValue)
{
	bool bRet = false;
	char szValue[32]="";
	rnValue = 0;
	bool bStartSpace=false;
	//now search for a number or end of the line.
	int i, cc;
	for (cc=m_uFilePtr, i=0; m_pMemory[cc] != '\r' && m_pMemory[cc] != '#' && ((m_pMemory[cc] != ' '&&m_pMemory[cc] != 9) || !bStartSpace); cc++, i++)
	{
		szValue[i] = m_pMemory[cc];
		if ( szValue[i] != ' ' && szValue[i] != 9 ) { bStartSpace = true; }
	}
	szValue[i] = 0;
	m_uFilePtr = cc;
	bRet = true;

	if ( bRet )
	{
		char *tmp;
		rnValue = strtol(szValue, &tmp, 10);
	}
	
	return bRet;
}

bool Parser::SearchKeyword_F_Next(float& rfValue)
{
	bool bRet = false;
	char szValue[32]="";
	rfValue = 0;
	bool bStartSpace=false;
	//now search for a number or end of the line.
	int i, cc;
	for (cc=m_uFilePtr, i=0; m_pMemory[cc] != '\r' && m_pMemory[cc] != '#' && ((m_pMemory[cc] != ' ' && m_pMemory[cc] != 9)|| !bStartSpace); cc++, i++)
	{
		szValue[i] = m_pMemory[cc];
		if ( szValue[i] != ' ' ) { bStartSpace = true; }
	}
	szValue[i] = 0;
	m_uFilePtr = cc;
	bRet = true;

	if ( bRet )
	{
		char *tmp;
		rfValue = (float)strtod(szValue, &tmp);
	}
	
	return bRet;
}

uint32_t Parser::GetFilePtr()
{
	return m_uFilePtr;
}

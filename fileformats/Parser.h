#ifndef PARSER_H
#define PARSER_H

#include "../CommonTypes.h"

class Parser
{
public:
	static void SetData(char *pMemory, u32 uLength, u32 uFilePtr=0);
	static s32 SearchKeyword_S32(const char *pszKeyword, s32& value);
	static bool SearchKeyword_Str(const char *pszKeyword, char *pszOut, bool bEndOnSpace=true, bool bPreserveSpaces=false, s32 maxSpaceCnt=4);
	static bool SearchKeyword_F1(const char *pszKeyword, f32& rfValue);
	static bool SearchKeyword_Next(s32& rnValue);
	static bool SearchKeyword_F_Next(f32& rfValue);
	static u32 GetFilePtr();

	static void SaveFilePtr()    { m_uFilePtrSaved = m_uFilePtr; }
	static void RestoreFilePtr() { m_uFilePtr = m_uFilePtrSaved; }

private:
	static const char *m_pMemory;
	static u32 m_uFilePtr;
	static u32 m_uFilePtrSaved;
	static u32 m_uLength;
	static u32 m_SeqRange[2];
};

#endif //PARSER_H
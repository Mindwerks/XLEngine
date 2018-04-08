#ifndef PARSER_H
#define PARSER_H

#include "../CommonTypes.h"

class Parser {
public:
    static void SetData(char *pMemory, uint32_t uLength, uint32_t uFilePtr = 0);

    static int32_t SearchKeyword_int32_t(const char *pszKeyword, int32_t &value);

    static bool
    SearchKeyword_Str(const char *pszKeyword, char *pszOut, bool bEndOnSpace = true, bool bPreserveSpaces = false,
                      int32_t maxSpaceCnt = 4);

    static bool SearchKeyword_F1(const char *pszKeyword, float &rfValue);

    static bool SearchKeyword_Next(int32_t &rnValue);

    static bool SearchKeyword_F_Next(float &rfValue);

    static uint32_t GetFilePtr();

    static void SaveFilePtr() { m_uFilePtrSaved = m_uFilePtr; }

    static void RestoreFilePtr() { m_uFilePtr = m_uFilePtrSaved; }

private:
    static const char *m_pMemory;
    static uint32_t m_uFilePtr;
    static uint32_t m_uFilePtrSaved;
    static uint32_t m_uLength;
    static uint32_t m_SeqRange[2];
};

#endif //PARSER_H
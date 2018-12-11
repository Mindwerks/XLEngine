#include "CutscenePlayer.h"
#include "../plugin_framework/plugin.h"
#include "../fileformats/ArchiveTypes.h"
#include "../movieplayback/MovieTypes.h"
#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <string>

bool CutscenePlayer::m_bCutscenePlaying;
CutscenePlayer::Cutscene CutscenePlayer::m_CutSceneList[256];
int32_t CutscenePlayer::m_nCutSceneCount;
int32_t CutscenePlayer::m_nCutSceneNum;
CutscenePlayer::Cutscene *CutscenePlayer::m_pCurCutScene;
const XLEngine_Plugin_API *CutscenePlayer::m_pAPI;

bool CutscenePlayer::Init(const XLEngine_Plugin_API *API)
{
    m_bCutscenePlaying = false;
    m_nCutSceneCount = 0;
    m_nCutSceneNum = 0;
    m_pCurCutScene = 0;

    m_pAPI = API;

    LoadCutsceneData();

    m_pAPI->MoviePlayer_SetPlayer( MOVIETYPE_LFD );

    return true;
}

void CutscenePlayer::Destroy()
{
}

void CutscenePlayer::LoadCutsceneData()
{
    if ( m_pAPI->GameFile_Open(ARCHIVETYPE_GOB, "DARK.GOB", "CUTSCENE.LST") )
    {
        uint32_t len = m_pAPI->GameFile_GetLength();
        char *pData = xlNew char[len+1];
        m_pAPI->GameFile_Read(pData, len);
        m_pAPI->GameFile_Close();

        //Set the data to parse: memory, length, filePtr
        m_pAPI->Parser_SetData( pData, len, 0 );
        //Now start parsing the file.
        int32_t nCuts=0;
        m_pAPI->Parser_SearchKeyword_int32_t("CUTS", nCuts);
        //now parse each line...
        char *pszCur = &pData[ m_pAPI->Parser_GetFilePtr() ];
        char szLine[256];
        int32_t lineIdx = 0;
        int32_t sl = (int32_t)strlen(pszCur);
        for (int l=0; l<sl; l++)
        {
            if ( pszCur[l] == '\r' || pszCur[l] == '\n' )
            {
                szLine[lineIdx] = 0;
                if ( lineIdx > 0 )
                {
                    CutScene_ParseLine(szLine);
                }

                lineIdx = 0;
                szLine[lineIdx] = 0;
            }
            else
            {
                szLine[lineIdx] = pszCur[l];
                lineIdx++;
            }
        }

        //free pData
        xlDelete [] pData;
        pData = nullptr;
    }
}

void CutscenePlayer::CutScene_ParseLine(char *pszLine)
{
    if ( pszLine[0] == '#' ) { return; }

    std::string str = pszLine;
    std::vector<std::string> result;

    std::istringstream iss(str);
    for(std::string str; iss >> str; )
        result.push_back(str);

    if (result.size() == 8) {
        // Uppercase filename
        std::transform(result.at(1).begin(), result.at(1).end(), result.at(1).begin(), ::toupper);

        m_CutSceneList[ m_nCutSceneCount ].num = (int16_t) std::stoi(result.at(0).substr(0, result.at(0).size() - 1));
        m_CutSceneList[ m_nCutSceneCount ].resLFD = result.at(1);
        m_CutSceneList[ m_nCutSceneCount ].scene = result.at(2);
        m_CutSceneList[ m_nCutSceneCount ].speed = (int16_t) std::stoi(result.at(3));
        m_CutSceneList[ m_nCutSceneCount ].nextScene = (int16_t) std::stoi(result.at(4));
        m_CutSceneList[ m_nCutSceneCount ].skipScene = (int16_t) std::stoi(result.at(5));
        m_CutSceneList[ m_nCutSceneCount ].midiSeq = (int16_t) std::stoi(result.at(6));
        m_CutSceneList[ m_nCutSceneCount ].midiVol = (int16_t) std::stoi(result.at(7));

        m_nCutSceneCount++;
    }
}

void CutscenePlayer::StartCutscene(int nID)
{
    m_nCutSceneNum = nID;

    //now find the cutscene index.
    int32_t c, cindex=-1;
    for (c=0; c<m_nCutSceneCount; c++)
    {
        if ( m_CutSceneList[c].num == nID )
        {
            cindex = c;
            break;
        }
    }

    //Can't find the cutscene index, just bail...
    if ( cindex == -1 )
    {
        return;
    }

    //The current cutscene.
    m_pCurCutScene = &m_CutSceneList[cindex];

    //Start the LFD_Film.
    std::string szFile = m_CutSceneList[cindex].scene + ".FILM";
    m_pAPI->MoviePlayer_SetArchives( ARCHIVETYPE_LFD, m_CutSceneList[cindex].resLFD.c_str(), "JEDISFX.LFD" );
    m_pAPI->MoviePlayer_Start( szFile.c_str(), (nID == 30) ? 1 : 0, (int)(m_CutSceneList[cindex].speed*1.30f) );

    //We're playing the cutscene (hopefully)
    m_bCutscenePlaying = true;
}

int CutscenePlayer::IsCutscenePlaying()
{
    return (m_bCutscenePlaying) ? 1 : 0;
}

void CutscenePlayer::Update()
{
    if ( m_pAPI->MoviePlayer_Update() == 0 )
    {
        //film is finished...
        if ( m_pCurCutScene->nextScene > 0 )
        {
            StartCutscene(m_pCurCutScene->nextScene);
        }
        else
        {
            m_pAPI->MoviePlayer_Stop();
            m_bCutscenePlaying = false;
        }
    }
}

void CutscenePlayer::KeyDown(int32_t key)
{
    //allow escape sequence.
    if ( key == XL_ESCAPE )
    {
        m_pAPI->MoviePlayer_Stop();

        if ( m_pCurCutScene->skipScene != 0 )
        {
            StartCutscene(m_pCurCutScene->skipScene);
        }
    }

    //allow individual scenes to be skipped.
    if ( key == XL_SPACE )
    {
        //film is finished...
        if ( m_pCurCutScene->nextScene > 0 )
        {
            StartCutscene(m_pCurCutScene->nextScene);
        }
        else
        {
            m_pAPI->MoviePlayer_Stop();
            m_bCutscenePlaying = false;
        }
    }
}

void CutscenePlayer::Render(float fDeltaTime)
{
    if ( m_bCutscenePlaying )
    {
        m_pAPI->MoviePlayer_Render(fDeltaTime);
    }
}

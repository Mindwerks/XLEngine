#include "CutscenePlayer.h"
#include "../plugin_framework/plugin.h"
#include "../fileformats/ArchiveTypes.h"
#include "../movieplayback/MovieTypes.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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
		pData = NULL;
	}
}

void CutscenePlayer::CutScene_ParseLine(char *pszLine)
{
	if ( pszLine[0] == '#' ) { return; }

	int32_t l = (int32_t)strlen(pszLine);
	char szElem[64];
	int32_t nElemIdx=0, n=0;
	//now acquire each component.
	for (int32_t i=0; i<l; i++)
	{
		if ( pszLine[i] != ' ' )
		{
			szElem[ nElemIdx++ ] = pszLine[i];
		}
		else if ( nElemIdx > 0 )
		{
			szElem[ nElemIdx ] = 0;
			switch (n)
			{
				case 0:	//num
					szElem[ strlen(szElem)-1 ] = 0;
					m_CutSceneList[ m_nCutSceneCount ].num = (short)atoi(szElem);
				break;
				case 1:	//resource LFD
					strcpy(m_CutSceneList[ m_nCutSceneCount ].resLFD, szElem);
				break;
				case 2:	//scene file.
					strcpy(m_CutSceneList[ m_nCutSceneCount ].scene, szElem);
				break;
				case 3:	//speed.
					m_CutSceneList[ m_nCutSceneCount ].speed = (short)atoi(szElem);
				break;
				case 4:	//next scene.
					m_CutSceneList[ m_nCutSceneCount ].nextScene = (short)atoi(szElem);
				break;
				case 5:	//skip scene.
					m_CutSceneList[ m_nCutSceneCount ].skipScene = (short)atoi(szElem);
				break;
				case 6:	//midi seq.
					m_CutSceneList[ m_nCutSceneCount ].midiSeq = (short)atoi(szElem);
				break;
				case 7:	//midi volume.
					m_CutSceneList[ m_nCutSceneCount ].midiVol = (short)atoi(szElem);
				break;
			};
			n++;
			nElemIdx = 0;
		}
	}

	if ( nElemIdx > 0 )
	{
		szElem[ nElemIdx ] = 0;
		switch (n)
		{
			case 0:	//num
				szElem[ strlen(szElem)-1 ] = 0;
				m_CutSceneList[ m_nCutSceneCount ].num = (short)atoi(szElem);
			break;
			case 1:	//resource LFD
				strcpy(m_CutSceneList[ m_nCutSceneCount ].resLFD, szElem);
			break;
			case 2:	//scene file.
				strcpy(m_CutSceneList[ m_nCutSceneCount ].scene, szElem);
			break;
			case 3:	//speed.
				m_CutSceneList[ m_nCutSceneCount ].speed = (short)atoi(szElem);
			break;
			case 4:	//next scene.
				m_CutSceneList[ m_nCutSceneCount ].nextScene = (short)atoi(szElem);
			break;
			case 5:	//skip scene.
				m_CutSceneList[ m_nCutSceneCount ].skipScene = (short)atoi(szElem);
			break;
			case 6:	//midi seq.
				m_CutSceneList[ m_nCutSceneCount ].midiSeq = (short)atoi(szElem);
			break;
			case 7:	//midi volume.
				m_CutSceneList[ m_nCutSceneCount ].midiVol = (short)atoi(szElem);
			break;
		};
		n++;
	}

	m_nCutSceneCount++;
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
	char szFile[32];
	sprintf(szFile, "%s.FILM", m_CutSceneList[cindex].scene);
	m_pAPI->MoviePlayer_SetArchives( ARCHIVETYPE_LFD, m_CutSceneList[cindex].resLFD, "JEDISFX.LFD" );
	m_pAPI->MoviePlayer_Start( szFile, (nID == 30) ? 1 : 0, (int)(m_CutSceneList[cindex].speed*1.30f) );

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

void CutscenePlayer::Render(f32 fDeltaTime)
{
	if ( m_bCutscenePlaying )
	{
		m_pAPI->MoviePlayer_Render(fDeltaTime);
	}
}

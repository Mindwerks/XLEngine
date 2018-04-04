#include "LFD_Film.h"
#include "../math/Math.h"
#include "../fileformats/Archive.h"
#include "../fileformats/LFD_Anim.h"
#include "../render/IDriver3D.h"
#include <math.h>
#include <string.h>

//#include "SoundSystem.h"

char m_Cache[4*1024*1024];
LFD_Film::Graphic LFD_Film::m_GraphicsCache[1024];
PLTT_File LFD_Film::m_PalCache[32];
s32 LFD_Film::m_nGCacheIdx=0;
s32 LFD_Film::m_nPalCacheIdx=0;
s32 m_nLastPalSetTime=0;
s32 last_time=0;

LFD_Film::Graphic *LFD_Film::m_apAnimQueue[202];

enum
{
	BTYPE_END=1,
	BTYPE_VIEW,
	BTYPE_DELT_ANIM_CUST,
	BTYPE_PLTT,
	BTYPE_VOIC,

	BTYPE_DELT,
	BTYPE_ANIM,
	BTYPE_CUST,
} BlockTypes_e;

enum
{
	CMD_END		=  2,
	CMD_TIME	=  3,
	CMD_MOVE	=  4,
	CMD_SPEED	=  5,
	CMD_LAYER	=  6,
	CMD_FRAME	=  7,
	CMD_ANIMATE	=  8,
	CMD_CUE		=  9,
	CMD_VAR		= 10,
	CMD_WINDOW	= 11,
	CMD_SWITCH	= 13,
	CMD_PALETTE	= 15,
	CMD_CUT		= 18,
	CMD_LOOP	= 20,
	CMD_PRELOAD	= 24,
	CMD_SOUND	= 25,
	CMD_STEREO	= 28
} Commands_e;

/////////////////////////////////////////////////////
LFD_Film::LFD_Film(IDriver3D *pDriver) : MoviePlayer(pDriver)
{
	m_nPalCount = 0;
	m_nTime = 0;
	m_nGCacheIdx = 0;
	m_nPalCacheIdx=0;
	m_nLastPalSetTime=0;
	m_fFrameDelay = 1.0f / 10.0f;
	m_fCurDelay = 0.0f;
	m_bFirstFrame = false;
	m_bTextCrawl = false;
	m_nCut = -1;
	m_fCutTime = 0.0f;
	m_fCurCutTime = 0.0f;
	m_Commands = NULL;
}

void LFD_Film::Stop()
{
	m_nPalCount = 0;
	m_nTime = 0;
	m_nGCacheIdx = 0;
	m_nPalCacheIdx=0;
	m_nLastPalSetTime=0;
	m_fCurDelay = 0.0f;

	if ( m_Graphics.size() > 0 )
	{
		vector<Graphic *>::iterator iter = m_Graphics.begin();
		vector<Graphic *>::iterator end  = m_Graphics.end();
		for (; iter!=end; ++iter)
		{
			if ( *iter )
			{
				Graphic *pGraphic = *iter;
				pGraphic->pAnim->Destroy();
				xlDelete pGraphic->pAnim;
			}
		}
		m_Graphics.clear();
	}

	if ( m_Commands )
	{
		for (s32 i=0; i<m_nFilmLength; i++)
		{
			m_Commands[i].commands.clear();
		}
		xlDelete [] m_Commands;
		m_Commands = NULL;
	}


	m_nCmdPoolIdx = 0;
	m_nFilmLength = 0;
}

int LFD_Film::ParseCommand(s32 cmd, char *pData)
{
	s32 size=0;
	s32 index=0;
	switch (cmd)
	{
		case CMD_TIME:
			last_time = *((s16 *)&pData[index]); index+=2;
			if ( last_time > m_nFilmLength )	//this is a fixed point time...
			{
				//u8 *pTimeFrac = (u8 *)&last_time;
				//last_time = (int)pTimeFrac[0];
				last_time = (last_time*10)/35;
			}
			size = 4;
		break;
		case CMD_CUT:
		{
			s16 cutHow  = *((s16 *)&pData[index]);  index+=2;
			s16 cutType = *((s16 *)&pData[index]); index+=2;
			s16 unknown = *((s16 *)&pData[index]); index+=2;
			printf("cut=%d, %d", cutHow, cutType);

			Command *pCmd = &m_CommandPool[ m_nCmdPoolIdx ]; m_nCmdPoolIdx++;
			pCmd->pData = pFile;
			pCmd->type = m_nEntry;
			pCmd->cmd  = CMD_CUT;
			pCmd->val[0] = cutHow;
			pCmd->val[1] = cutType;

			s32 time = (last_time >= m_nFilmLength-1) ? last_time-12 : last_time;
			m_Commands[ time ].commands.push_back( pCmd );

			size = 6;
		}
		break;
		case CMD_LOOP:
		{
			index+=2;
			Command *pCmd = &m_CommandPool[ m_nCmdPoolIdx ]; m_nCmdPoolIdx++;
			pCmd->pData = pFile;
			pCmd->type = m_nEntry;
			pCmd->cmd  = CMD_LOOP;

			m_Commands[ last_time ].commands.push_back( pCmd );

			size = 4;
		}
		break;
		case CMD_PRELOAD:
		{
			index+=2;
			Command *pCmd = &m_CommandPool[ m_nCmdPoolIdx ]; m_nCmdPoolIdx++;
			pCmd->pData = pFile;
			pCmd->type = m_nEntry;
			pCmd->cmd  = CMD_PRELOAD;

			m_Commands[ last_time ].commands.push_back( pCmd );

			size = 4;
		}
		break;
		case CMD_STEREO:
		{
			s16 onOff = *((s16 *)&pData[index]); index+=2;
			s16 vol   = *((s16 *)&pData[index]); index+=2;
			s16 vA    = *((s16 *)&pData[index]); index+=2;
			s16 vB    = *((s16 *)&pData[index]); index+=2;
			s16 pan   = *((s16 *)&pData[index]); index+=2;
			Command *pCmd = &m_CommandPool[ m_nCmdPoolIdx ]; m_nCmdPoolIdx++;
			pCmd->pData = pFile;
			pCmd->type = m_nEntry;
			pCmd->cmd  = CMD_STEREO;
			pCmd->val[0] = onOff;
			pCmd->val[1] = vol;
			pCmd->val[2] = pan;

			m_Commands[ last_time ].commands.push_back( pCmd );

			size = 16;
		}
		break;
		case CMD_SOUND:
		{
			s16 onOff = *((s16 *)&pData[index]); index+=2;
			s16 vol   = *((s16 *)&pData[index]); index+=2;
			s16 vA    = *((s16 *)&pData[index]); index+=2;
			s16 vB    = *((s16 *)&pData[index]); index+=2;

			Command *pCmd = &m_CommandPool[ m_nCmdPoolIdx ]; m_nCmdPoolIdx++;
			pCmd->pData = pFile;
			pCmd->type = m_nEntry;
			pCmd->cmd  = CMD_SOUND;
			pCmd->val[0] = onOff;
			pCmd->val[1] = vol;
			m_Commands[ last_time ].commands.push_back( pCmd );

			size = 10;
		}
		break;
		case CMD_PALETTE:
		{
			Command *pCmd = &m_CommandPool[ m_nCmdPoolIdx ]; m_nCmdPoolIdx++;
			pCmd->pData = pFile;
			pCmd->type = m_nEntry;
			pCmd->cmd  = CMD_PALETTE;
			m_Commands[ last_time ].commands.push_back( pCmd );

			size = 4;
		}
		break;
		case CMD_LAYER:
		{
			s16 layer = *((s16 *)&pData[index]); index+=2;

			Command *pCmd = &m_CommandPool[ m_nCmdPoolIdx ]; m_nCmdPoolIdx++;
			pCmd->pData = pFile;
			pCmd->type = m_nEntry;
			pCmd->cmd  = CMD_LAYER;
			pCmd->val[0] = layer;
			m_Commands[ last_time ].commands.push_back( pCmd );

			size = 4;
		}
		break;
		case CMD_SWITCH:
		{
			s16 onOff = *((s16 *)&pData[index]); index+=2;

			Command *pCmd = &m_CommandPool[ m_nCmdPoolIdx ]; m_nCmdPoolIdx++;
			pCmd->pData = pFile;
			pCmd->type = m_nEntry;
			pCmd->cmd  = CMD_SWITCH;
			pCmd->val[0] = onOff;
			m_Commands[ last_time ].commands.push_back( pCmd );

			size = 4;
		}
		break;
		case CMD_ANIMATE:
		{
			s16 dir  = *((s16 *)&pData[index]); index+=2;
			s16 move = *((s16 *)&pData[index]); index+=2;

			Command *pCmd = &m_CommandPool[ m_nCmdPoolIdx ]; m_nCmdPoolIdx++;
			pCmd->pData = pFile;
			pCmd->type = m_nEntry;
			pCmd->cmd  = CMD_ANIMATE;
			pCmd->val[0] = dir;
			pCmd->val[1] = move;
			m_Commands[ last_time ].commands.push_back( pCmd );

			size = 6;
		}
		break;
		case CMD_FRAME:
		{
			s16 frame = *((s16 *)&pData[index]); index+=2;
			s16 move  = *((s16 *)&pData[index]); index+=2;

			Command *pCmd = &m_CommandPool[ m_nCmdPoolIdx ]; m_nCmdPoolIdx++;
			pCmd->pData = pFile;
			pCmd->type = m_nEntry;
			pCmd->cmd  = CMD_FRAME;
			pCmd->val[0] = frame;
			pCmd->val[1] = move;
			m_Commands[ last_time ].commands.push_back( pCmd );

			size = 6;
		}
		break;
		case CMD_CUE:
		{
			s16 cue = *((s16 *)&pData[index]); index+=2;

			Command *pCmd = &m_CommandPool[ m_nCmdPoolIdx ]; m_nCmdPoolIdx++;
			pCmd->pData = pFile;
			pCmd->type = m_nEntry;
			pCmd->cmd  = CMD_CUE;
			pCmd->val[0] = cue;
			m_Commands[ last_time ].commands.push_back( pCmd );

			size = 4;
		}
		break;
		case CMD_MOVE:
		{
			s16 x = *((s16 *)&pData[index]); index+=2;
			s16 y = *((s16 *)&pData[index]); index+=2;

			Command *pCmd = &m_CommandPool[ m_nCmdPoolIdx ]; m_nCmdPoolIdx++;
			pCmd->pData = pFile;
			pCmd->type = m_nEntry;
			pCmd->cmd  = CMD_MOVE;
			pCmd->val[0] = x;
			pCmd->val[1] = y;
			m_Commands[ last_time ].commands.push_back( pCmd );

			size = 10;
		}
		break;
		case CMD_SPEED:
		{
			s16 r = *((s16 *)&pData[index]); index+=2;
			s16 d = *((s16 *)&pData[index]); index+=2;

			Command *pCmd = &m_CommandPool[ m_nCmdPoolIdx ]; m_nCmdPoolIdx++;
			pCmd->pData = pFile;
			pCmd->type = m_nEntry;
			pCmd->cmd  = CMD_SPEED;
			pCmd->val[0] = r;
			pCmd->val[1] = d;
			m_Commands[ last_time ].commands.push_back( pCmd );

			size = 10;
		}
		break;
	}
	return size;
}

bool LFD_Film::Start(Archive *pRes0, Archive *pRes1, const char *pszFile, u32 uFlags, s32 nSpeed)
{
	Stop();

	m_pReader = pRes0;
	Archive *pSndReader = pRes1;
	m_bTextCrawl = uFlags ? true : false;

	//Framerate.
	m_fFrameDelay = 1.0f / (f32)nSpeed;
	m_fCurDelay   = m_fFrameDelay;
	m_bFirstFrame = true;

	m_nCut = -1;
	m_fCutTime = 0.0f;
	m_fCurCutTime = 0.0f;

	//Read the file.
	m_pReader->OpenFile( pszFile );
	u32 len = m_pReader->GetFileLen();
	char *pData = xlNew char[len+1];
	m_pReader->ReadFile(pData, len);
	m_pReader->CloseFile();

	s16 nNumEntries;
	s32 index=2, c, cmd, size;
	char szName[32];
	char szFile[32];
	m_nFilmLength = *((s16 *)&pData[index]); index+=2;
	nNumEntries = *((s16 *)&pData[index]); index+=2;

	assert( m_nFilmLength > 0 );

	m_Commands = xlNew CommandSet[m_nFilmLength];
	for (c=0; c<m_nFilmLength; c++)
	{
		m_Commands[c].bActivated = false;
	}

	FilmEntry *pEntry;
	for (s32 e=0; e<nNumEntries; e++)
	{
		pEntry = (FilmEntry *)&pData[index]; index+=sizeof(FilmEntry);
		//commands
		switch (pEntry->nBlockType)
		{
			case BTYPE_VIEW:
				pFile = NULL;
				m_nEntry = BTYPE_VIEW;
			break;
			case BTYPE_VOIC:
				pFile = NULL;
				m_nEntry = BTYPE_VOIC;

				memcpy(szName, pEntry->szName, 8);
				szName[8] = 0;
				/*	Take care of sound later.
				if ( pSndReader->OpenFile(szName, "VOIC") )
				{
					len = pSndReader->GetFileLen();
					pSndReader->ReadFile(m_Cache, len);
					pSndReader->CloseFile();

					sprintf(szName, "%s.VOC", szName);
					pFile = (void *)SoundSystem::LoadAndPrepareSound(m_Cache, len, szName);
				}
				*/
			break;
			case BTYPE_PLTT:
			{
				PLTT_File *pal = &m_PalCache[m_nPalCacheIdx++];
				assert( m_nPalCacheIdx <= 32 );

				memcpy(szName, pEntry->szName, 8);
				szName[8] = 0;

				sprintf(szFile, "%s.PLTT", szName);

				if ( m_pReader->OpenFile(szFile) )
				{
					len = m_pReader->GetFileLen();
					m_pReader->ReadFile(m_Cache, len);
					m_pReader->CloseFile();

					pal->First = *((u8 *)&m_Cache[0]);
					pal->Last  = *((u8 *)&m_Cache[1]);
					pal->num_colors = pal->Last - pal->First + 1;
					memset(pal->colors, 0, 256*3);
					memcpy(pal->colors, &m_Cache[2], pal->num_colors*3);
				}

				pFile = pal;
				m_nEntry = BTYPE_PLTT;
			}
			break;
			case BTYPE_DELT_ANIM_CUST:
				pFile = NULL;
				m_nEntry = BTYPE_DELT_ANIM_CUST;
				if ( pEntry->szType[0] == 'D' && pEntry->szType[1] == 'E' && pEntry->szType[2] == 'L' && pEntry->szType[3] == 'T' )
				{
					memcpy(szName, pEntry->szName, 8);
					szName[8] = 0;

					sprintf(szFile, "%s.DELT", szName);

					m_pReader->OpenFile( szFile );
					len = m_pReader->GetFileLen();
					m_pReader->ReadFile(m_Cache, len);
					m_pReader->CloseFile();

					//find the latest palette that is at this time or sooner...
					LFD_Anim::SetPLTT( &m_PalCache[0] );

					LFD_Anim *pDelt = xlNew LFD_Anim(m_pDriver);
					pDelt->LoadDELT(m_Cache, len, true);

					Graphic *pGraphic = &m_GraphicsCache[m_nGCacheIdx++];
					pFile = pGraphic;

					pGraphic->pAnim = pDelt;
					pGraphic->bVisible = 0;
					pGraphic->nX = 0;
					pGraphic->nY = 0;
					pGraphic->nFrame = 0;
					pGraphic->nAnimate = 0;
					pGraphic->anScroll[0] = 0;
					pGraphic->anScroll[1] = 0;
					pGraphic->nLayer = 0;
					pGraphic->nType = 0;
					pGraphic->bLoop = false;
					pGraphic->bNoReverse = false;
					//pDelt->SetOffsScale(1.0f, _fYOffsScale);
					strcpy(pGraphic->szFile, szName);

					m_Graphics.push_back(pGraphic);

					m_nEntry = BTYPE_DELT;
				}
				else if ( pEntry->szType[0] == 'A' && pEntry->szType[1] == 'N' && pEntry->szType[2] == 'I' && pEntry->szType[3] == 'M' )
				{
					memcpy(szName, pEntry->szName, 8);
					szName[8] = 0;

					sprintf(szFile, "%s.ANIM", szName);

					m_pReader->OpenFile( szFile );
					len = m_pReader->GetFileLen();
					m_pReader->ReadFile(m_Cache, len);
					m_pReader->CloseFile();

					//find the latest palette that is at this time or sooner...
					LFD_Anim::SetPLTT( &m_PalCache[0] );

					LFD_Anim *pAnim = xlNew LFD_Anim(m_pDriver);
					pAnim->Load(m_Cache, len, true);

					Graphic *pGraphic = &m_GraphicsCache[m_nGCacheIdx++];
					pFile = pGraphic;

					pGraphic->pAnim = pAnim;
					pGraphic->bVisible = 0;
					pGraphic->nX = 0;
					pGraphic->nY = 0;
					pGraphic->nFrame = 0;
					pGraphic->nLayer = 0;
					pGraphic->nType = 1;
					pGraphic->anScroll[0] = 0;
					pGraphic->anScroll[1] = 0;
					pGraphic->bLoop = false;
					pGraphic->bNoReverse = false;
					//pAnim->SetOffsScale(1.0f, _fYOffsScale);
					strcpy(pGraphic->szFile, szName);

					//temp. hack until I figure out looping.
					if ( stricmp(pGraphic->szFile, "moceyes") == 0 || stricmp(pGraphic->szFile, "montalk1") == 0 || stricmp(pGraphic->szFile, "montalk2") == 0 ||
						 stricmp(pGraphic->szFile, "madine") == 0 )
					{
						pGraphic->bLoop = true;
					}
					if ( stricmp(pGraphic->szFile, "tube2x") == 0 )
					{
						pGraphic->bNoReverse = true;
					}
					//

					m_Graphics.push_back(pGraphic);

                    m_nEntry = BTYPE_ANIM;
				}
				else if ( pEntry->szType[0] == 'C' && pEntry->szType[1] == 'U' && pEntry->szType[2] == 'S' && pEntry->szType[3] == 'T' )
				{
					m_nEntry = BTYPE_CUST;
				}
			break;
		};
		index -= 2;
		for (c=0; c<pEntry->nNumCommands; c++)
		{
			size = *((s16 *)&pData[index]); index+=2;
			cmd  = *((s16 *)&pData[index]); index+=2;
			ParseCommand(cmd, &pData[index]);
			index += size-4;
		}
	}

	if ( pData )
	{
		xlDelete [] pData;
	}

	return true;
}

bool LFD_Film::Update()
{
	const f32 fDeltaTime = (1.0f/60.0f);
	bool bFilmFinished = false;
	s32 len;

	bool bUpdateAnim = false;

	if ( !bFilmFinished )
	{
		m_fCurDelay -= fDeltaTime;
		if ( m_fCurDelay <= 0.0f )
		{
			m_nTime++;
			bUpdateAnim = true;
			if ( m_nTime >= m_nFilmLength-1 )
			{
				m_nTime = m_nFilmLength-1;
				bFilmFinished = true;
			}
			m_fCurDelay = m_fFrameDelay;
		}
		else if ( m_bFirstFrame )
		{
			bUpdateAnim = true;
			m_bFirstFrame = false;
		}

		//SoundSystem::SetListenerLoc(&Vector3::Zero);
		//SoundSystem::SetListenerVel(&Vector3::Zero);

		char szFile[64];

		if ( m_nTime > -1 && (!m_Commands[m_nTime].bActivated) && m_Commands[m_nTime].commands.size() > 0 )
		{
			m_Commands[m_nTime].bActivated = true;

			vector<Command *>::iterator iter = m_Commands[m_nTime].commands.begin();
			vector<Command *>::iterator end  = m_Commands[m_nTime].commands.end();

			for (; iter!=end; ++iter)
			{
				Command *pCmd = *iter;
				if ( pCmd->cmd == CMD_SWITCH )
				{
					Graphic *pGraphic = (Graphic *)pCmd->pData;
					pGraphic->bVisible = pCmd->val[0];
					if ( pGraphic->bVisible )
					{
						pGraphic->pAnim->Destroy();
						if ( pGraphic->nType == 0 )
						{
							sprintf(szFile, "%s.DELT", pGraphic->szFile);
							m_pReader->OpenFile( szFile );
							len = m_pReader->GetFileLen();
							m_pReader->ReadFile(m_Cache, len);
							m_pReader->CloseFile();
							pGraphic->pAnim->LoadDELT(m_Cache, len, true);
						}
						else
						{
							sprintf(szFile, "%s.ANIM", pGraphic->szFile);
							m_pReader->OpenFile( szFile );
							len = m_pReader->GetFileLen();
							m_pReader->ReadFile(m_Cache, len);
							m_pReader->CloseFile();
							pGraphic->pAnim->Load(m_Cache, len, true);
						}
						//pGraphic->pAnim->SetOffsScale(1.0f, _fYOffsScale);
					}
					else
					{
						pGraphic->nFrame = 0;
					}
				}
				else if ( pCmd->cmd == CMD_FRAME )
				{
					Graphic *pGraphic = (Graphic *)pCmd->pData;
					if ( pGraphic->bNoReverse )
					{
						pGraphic->nFrame = MAX(pGraphic->nFrame, pCmd->val[0]);
					}
					else
					{
						pGraphic->nFrame = pCmd->val[0];
					}
					if ( pGraphic->nAnimate > 0 ) pGraphic->nAnimate = 0;
				}
				else if ( pCmd->cmd == CMD_ANIMATE )
				{
					Graphic *pGraphic = (Graphic *)pCmd->pData;
					pGraphic->nAnimate = pCmd->val[0];
				}
				else if ( pCmd->cmd == CMD_SPEED )
				{
					Graphic *pGraphic = (Graphic *)pCmd->pData;
					pGraphic->anScroll[0] = pCmd->val[0];
					pGraphic->anScroll[1] = pCmd->val[1];
				}
				else if ( pCmd->cmd == CMD_MOVE )
				{
					Graphic *pGraphic = (Graphic *)pCmd->pData;
					if ( pGraphic )
					{
						pGraphic->nX = pCmd->val[0];
						pGraphic->nY = pCmd->val[1];
					}
				}
				else if ( pCmd->cmd == CMD_LAYER )
				{
					Graphic *pGraphic = (Graphic *)pCmd->pData;
					if ( pGraphic )
					{
						pGraphic->nLayer = pCmd->val[0];
					}
				}
				else if ( pCmd->cmd == CMD_PALETTE )
				{
					PLTT_File *pal = (PLTT_File *)pCmd->pData;
					LFD_Anim::SetPLTT(pal, false);
					m_nLastPalSetTime = m_nTime;
				}
				else if ( pCmd->cmd == CMD_CUT )
				{
					StartCut( pCmd->val[0], pCmd->val[1] );
				}
				else if ( pCmd->cmd == CMD_SOUND || pCmd->cmd == CMD_STEREO )
				{
					int sndIdx = pCmd->nData;
					int onOff  = pCmd->val[0];
					int vol    = pCmd->val[1];

					if ( onOff == 0 )
					{
						//SoundSystem::StopSound(sndIdx);
					}
					else
					{
						//if ( !SoundSystem::IsSoundPlaying(sndIdx) )
						{
							if ( vol == 0 ) { vol = 100; }
							float fvol = (float)vol*0.01f;
							float fpan = 0.0f;

							if ( pCmd->cmd == CMD_STEREO )
							{
								int pan = pCmd->val[2];
								fpan = ((float)pan/255.0f)*2.0f - 1.0f;
							}

							//SoundSystem::PlayOneShotSoundSrc(sndIdx, false, fvol);
							//SoundSystem::SetPan(sndIdx, fpan);
						}
					}
				}
			}
		}

		if ( bUpdateAnim )
		{
			if ( !bFilmFinished )
			{
				vector<Graphic *>::iterator iter = m_Graphics.begin();
				vector<Graphic *>::iterator end  = m_Graphics.end();
				for (; iter!=end; ++iter)
				{
					Graphic *pGraphic = *iter;

					if ( pGraphic->nAnimate > 0)
					{
						pGraphic->nFrame++;
						if ( pGraphic->nFrame > pGraphic->pAnim->GetFrameCount()-1 )
						{
							if ( !pGraphic->bLoop )
							{
								pGraphic->nFrame = pGraphic->pAnim->GetFrameCount()-1;
							}
							else
							{
								pGraphic->nFrame = 0;
							}
						}
					}
					else if ( pGraphic->nAnimate < 0 )
					{
						pGraphic->nFrame--;
						if ( pGraphic->nFrame < 0 )
						{
							if ( !pGraphic->bLoop )
							{
								pGraphic->nFrame = 0;
							}
							else
							{
								pGraphic->nFrame = pGraphic->pAnim->GetFrameCount()-1;
							}
						}
					}
					if ( pGraphic->bVisible )
					{
						pGraphic->nX += pGraphic->anScroll[0];
						pGraphic->nY += pGraphic->anScroll[1];
					}
				}
			}
		}
	}

	if ( bFilmFinished )
	{
		//SoundSystem::StopAllSounds();
		//SoundSystem::UnpauseAllSounds();
	}

	return !bFilmFinished;
}

void LFD_Film::StartCut(int how, int type)
{
	switch (how)
	{
		case 1:		//swap - no clear.
			if ( type != 2 )
			{
				m_nCut = -1;
			}
			break;
		case 2:		//clear
		case 3:		//dirty
			if ( type == 2 )
			{
				m_nCut = FADE_CLEAR;
				m_fCutTime = 0.5f;
				m_fCurCutTime = 0.0f;
			}
			else
			{
				m_nCut = -1;
			}
			break;
		case 12:	//fade right
			if ( type == 2 )
			{
				m_nCut = FADE_RIGHT_ON_OFF;
				m_fCutTime = 1.0f;
			}
			else if ( type == 3 )
			{
				m_nCut = FADE_RIGHT_OFF;
				m_fCutTime = 1.0f;
			}
			else if ( type == 4 )
			{
				m_nCut = FADE_RIGHT_ON;
				m_fCutTime = 1.0f;
			}
			m_fCurCutTime = 0.0f;
			break;
		case 13:	//fade left
			if ( type == 2 )
			{
				m_nCut = FADE_LEFT_ON_OFF;
				m_fCutTime = 1.0f;
			}
			else if ( type == 3 )
			{
				m_nCut = FADE_LEFT_OFF;
				m_fCutTime = 1.0f;
			}
			else if ( type == 4 )
			{
				m_nCut = FADE_LEFT_ON;
				m_fCutTime = 1.0f;
			}
			m_fCurCutTime = 0.0f;
			break;
		case 14:	//fade up
			if ( type == 2 )
			{
				m_nCut = FADE_UP_ON_OFF;
				m_fCutTime = 1.0f;
			}
			else if ( type == 3 )
			{
				m_nCut = FADE_UP_OFF;
				m_fCutTime = 1.0f;
			}
			else if ( type == 4 )
			{
				m_nCut = FADE_UP_ON;
				m_fCutTime = 1.0f;
			}
			m_fCurCutTime = 0.0f;
			break;
		case 15:	//fade down
			if ( type == 2 )
			{
				m_nCut = FADE_DOWN_ON_OFF;
				m_fCutTime = 1.0f;
			}
			else if ( type == 3 )
			{
				m_nCut = FADE_DOWN_OFF;
				m_fCutTime = 1.0f;
			}
			else if ( type == 4 )
			{
				m_nCut = FADE_DOWN_ON;
				m_fCutTime = 1.0f;
			}
			m_fCurCutTime = 0.0f;
			break;
		case 21:	//fade up/down
			break;
		case 23:	//soft fade to black.
			if ( type == 2 )
			{
				m_nCut = FADE_TO_THEN_FROM_BLACK;
				m_fCutTime = 3.0f;
			}
			else if ( type == 3 )
			{
				m_nCut = FADE_TO_BLACK;
				m_fCutTime = 3.0f;
			}
			else if ( type == 4 )
			{
				m_nCut = FADE_FROM_BLACK;
				m_fCutTime = 3.0f;
			}
			m_fCurCutTime = 0.0f;
			break;
		case 26:	//really quick fade from black.
		case 27:	//really quick fade from black.
			if ( type == 2 )
			{
				m_nCut = FADE_TO_THEN_FROM_BLACK;
				m_fCutTime = 0.5f;
			}
			else if ( type == 3 )
			{
				m_nCut = FADE_TO_BLACK;
				m_fCutTime = 0.5f;
			}
			else if ( type == 4 )
			{
				m_nCut = FADE_FROM_BLACK;
				m_fCutTime = 0.5f;
			}
			m_fCurCutTime = 0.0f;
			break;
		case 2333:	//fade to black
			if ( m_nCut != 23 || type != 3 )
			{
				m_fCurCutTime = 0.0f;
			}
			else
			{
				m_fCurCutTime = m_fCurCutTime/3.0f;
			}
			if ( type == 2 )
			{
				m_nCut = FADE_TO_THEN_FROM_BLACK;
				m_fCutTime = 1.0f;
			}
			else if ( type == 3 )
			{
				m_nCut = FADE_TO_BLACK;
				m_fCutTime = 1.0f;
			}
			else if ( type == 4 )
			{
				m_nCut = FADE_FROM_BLACK;
				m_fCutTime = 1.0f;
			}
			break;
		default:
				m_nCut = -1;
			break;
	}
}

void LFD_Film::AddGraphicToQueue(Graphic *pGraphic, int nLayer)
{
	pGraphic->pNext = NULL;
	if ( nLayer < 0 ) { nLayer = 0; }
	if ( nLayer > 201 ) { nLayer = 201; }

	if ( !m_apAnimQueue[ nLayer ] )
	{
		m_apAnimQueue[ nLayer ] = pGraphic;
	}
	else
	{
		Graphic *pCur = m_apAnimQueue[ nLayer ];
		while (pCur)
		{
			if ( !pCur->pNext )
			{
				pCur->pNext = pGraphic;
				return;
			}

			pCur = pCur->pNext;
		};
	}
}

void LFD_Film::Render(f32 fDeltaTime)
{
	const f32 fOO320 = 1.0f / 320.0f;
	const f32 fOO200 = 1.0f / 200.0f;

	m_pDriver->SetBlendMode();
	m_pDriver->EnableAlphaTest( true );
	m_pDriver->EnableCulling( false );
	m_pDriver->EnableDepthRead( false );
	m_pDriver->EnableDepthWrite( false );

	s32 i;
	for (i=0; i<202; i++)
	{
		m_apAnimQueue[i] = NULL;
	}

	//render all the graphics that are visible...
	vector<Graphic *>::iterator iter = m_Graphics.begin();
	vector<Graphic *>::iterator end  = m_Graphics.end();
	for (; iter!=end; ++iter)
	{
		Graphic *pGraphic = *iter;

		if ( pGraphic->bVisible )
		{
			AddGraphicToQueue(pGraphic, pGraphic->nLayer+100);
		}
	}

	//go through the render queue and render in order.
	for (i=201; i>=0; i--)
	{
		if ( m_apAnimQueue[i] )
		{
			Graphic *pCur = m_apAnimQueue[i];
			while (pCur)
			{
				bool bDistort = false;
				if ( pCur->nLayer == 0 && m_bTextCrawl )
				{
					bDistort = true;
				}

				pCur->pAnim->Render( pCur->nFrame, (float)pCur->nX * fOO320, ((float)pCur->nY * fOO200), 1000.0f, -1000.0f, 0.0f, 0.0f, bDistort );
				pCur = pCur->pNext;
			};
		}
	}

	s32 nScrWidth, nScrHeight;
	m_pDriver->GetWindowSize(nScrWidth, nScrHeight);
	Vector4 posScale(0.0f, 0.0f, (f32)nScrWidth, (f32)nScrHeight);
	Vector2 uv(0,0);
	Vector4 color(0,0,0,1);

	m_pDriver->SetTexture(0, XL_INVALID_TEXTURE);
	m_pDriver->EnableAlphaTest(false);
	m_pDriver->SetBlendMode( IDriver3D::BLEND_ALPHA );

	if ( m_nCut > -1 )
	{
		f32 alpha = 0.0f;
		if ( m_nCut == FADE_FROM_BLACK )
		{
			alpha = 1.0f - m_fCurCutTime / m_fCutTime;
			m_fCurCutTime += fDeltaTime;
			if ( m_fCurCutTime >= m_fCutTime )
			{
				m_nCut = -1;
				m_fCutTime = 0.0f;
				m_fCurCutTime = 0.0f;
			}
			if ( alpha > 0.0f )
			{
				color.w = alpha;
				m_pDriver->RenderScreenQuad(posScale, uv, uv, color, color);
			}
		}
		else if ( m_nCut == FADE_TO_BLACK )
		{
			alpha = m_fCurCutTime / m_fCutTime;
			m_fCurCutTime += fDeltaTime;
			if ( m_fCurCutTime >= m_fCutTime )
			{
				m_fCurCutTime = m_fCutTime;
				if ( alpha > 1.0f ) { alpha = 1.0f; }
			}
			if ( alpha > 0.0f )
			{
				color.w = alpha;
				m_pDriver->RenderScreenQuad(posScale, uv, uv, color, color);
			}
		}
		else if ( m_nCut == FADE_TO_THEN_FROM_BLACK )
		{
			alpha = 2.0f * m_fCurCutTime / m_fCutTime;
			if ( alpha > 1.0f ) { alpha = 2.0f - alpha; }

			m_fCurCutTime += fDeltaTime;
			if ( m_fCurCutTime >= m_fCutTime )
			{
				m_nCut = -1;
				m_fCutTime = 0.0f;
				m_fCurCutTime = 0.0f;
			}
			if ( alpha > 0.0f )
			{
				color.w = alpha;
				m_pDriver->RenderScreenQuad(posScale, uv, uv, color, color);
			}
		}
		else if ( m_nCut == FADE_RIGHT_OFF )
		{
			alpha = m_fCurCutTime / m_fCutTime;
			m_fCurCutTime += fDeltaTime;
			if ( m_fCurCutTime >= m_fCutTime )
			{
				m_fCurCutTime = m_fCutTime;
				if ( alpha > 1.0f ) { alpha = 1.0f; }
			}
			if ( alpha > 0.0f )
			{
				posScale.z = alpha*(f32)nScrWidth;
				m_pDriver->RenderScreenQuad(posScale, uv, uv, color, color);
			}
		}
		else if ( m_nCut == FADE_RIGHT_ON )
		{
			alpha = m_fCurCutTime / m_fCutTime;
			m_fCurCutTime += fDeltaTime;
			if ( m_fCurCutTime >= m_fCutTime )
			{
				m_nCut = -1;
				m_fCutTime = 0.0f;
				m_fCurCutTime = 0.0f;

				if ( alpha > 1.0f ) { alpha = 1.0f; }
			}
			if ( alpha < 1.0f )
			{
				posScale.x = alpha*(f32)nScrWidth;
				posScale.z = (1.0f-alpha)*(f32)nScrWidth;
				m_pDriver->RenderScreenQuad(posScale, uv, uv, color, color);
			}
		}
		else if ( m_nCut == FADE_LEFT_OFF )
		{
			alpha = m_fCurCutTime / m_fCutTime;
			m_fCurCutTime += fDeltaTime;
			if ( m_fCurCutTime >= m_fCutTime )
			{
				m_fCurCutTime = m_fCutTime;
				if ( alpha > 1.0f ) { alpha = 1.0f; }
			}
			if ( alpha > 0.0f )
			{
				posScale.x = (1.0f-alpha)*(f32)nScrWidth;
				posScale.z = alpha*(f32)nScrWidth;
				m_pDriver->RenderScreenQuad(posScale, uv, uv, color, color);
			}
		}
		else if ( m_nCut == FADE_LEFT_ON )
		{
			alpha = m_fCurCutTime / m_fCutTime;
			m_fCurCutTime += fDeltaTime;
			if ( m_fCurCutTime >= m_fCutTime )
			{
				m_nCut = -1;
				m_fCutTime = 0.0f;
				m_fCurCutTime = 0.0f;

				if ( alpha > 1.0f ) { alpha = 1.0f; }
			}
			if ( alpha < 1.0f )
			{
				posScale.z = alpha*(f32)nScrWidth;
				m_pDriver->RenderScreenQuad(posScale, uv, uv, color, color);
			}
		}
		else if ( m_nCut == FADE_UP_OFF )
		{
			alpha = m_fCurCutTime / m_fCutTime;
			m_fCurCutTime += fDeltaTime;
			if ( m_fCurCutTime >= m_fCutTime )
			{
				m_fCurCutTime = m_fCutTime;
				if ( alpha > 1.0f ) { alpha = 1.0f; }
			}
			if ( alpha > 0.0f )
			{
				posScale.w = alpha*(f32)nScrHeight;
				m_pDriver->RenderScreenQuad(posScale, uv, uv, color, color);
			}
		}
		else if ( m_nCut == FADE_UP_ON )
		{
			alpha = m_fCurCutTime / m_fCutTime;
			m_fCurCutTime += fDeltaTime;
			if ( m_fCurCutTime >= m_fCutTime )
			{
				m_nCut = -1;
				m_fCutTime = 0.0f;
				m_fCurCutTime = 0.0f;

				if ( alpha > 1.0f ) { alpha = 1.0f; }
			}
			if ( alpha < 1.0f )
			{
				posScale.y = alpha*(f32)nScrHeight;
				posScale.w = (1.0f-alpha)*(f32)nScrHeight;
				m_pDriver->RenderScreenQuad(posScale, uv, uv, color, color);
			}
		}
		else if ( m_nCut == FADE_DOWN_OFF )
		{
			alpha = m_fCurCutTime / m_fCutTime;
			m_fCurCutTime += fDeltaTime;
			if ( m_fCurCutTime >= m_fCutTime )
			{
				m_fCurCutTime = m_fCutTime;
				if ( alpha > 1.0f ) { alpha = 1.0f; }
			}
			if ( alpha > 0.0f )
			{
				posScale.y = (1.0f-alpha)*(f32)nScrHeight;
				posScale.w = alpha*(f32)nScrHeight;
				m_pDriver->RenderScreenQuad(posScale, uv, uv, color, color);
			}
		}
		else if ( m_nCut == FADE_DOWN_ON )
		{
			alpha = m_fCurCutTime / m_fCutTime;
			m_fCurCutTime += fDeltaTime;
			if ( m_fCurCutTime >= m_fCutTime )
			{
				m_nCut = -1;
				m_fCutTime = 0.0f;
				m_fCurCutTime = 0.0f;

				if ( alpha > 1.0f ) { alpha = 1.0f; }
			}
			if ( alpha < 1.0f )
			{
				posScale.w = (1.0f-alpha)*(f32)nScrHeight;
				m_pDriver->RenderScreenQuad(posScale, uv, uv, color, color);
			}
		}
		else if ( m_nCut == FADE_CLEAR )
		{
			m_fCurCutTime += fDeltaTime;
			if ( m_fCurCutTime >= m_fCutTime )
			{
				m_nCut = -1;
				m_fCutTime = 0.0f;
				m_fCurCutTime = 0.0f;

				if ( alpha > 1.0f ) { alpha = 1.0f; }
			}
			posScale.w = (1.0f-alpha)*(f32)nScrHeight;
			m_pDriver->RenderScreenQuad(posScale, uv, uv, color, color);
		}
	}

	m_pDriver->SetBlendMode();
	m_pDriver->EnableCulling(true);
	m_pDriver->EnableDepthWrite(true);
	m_pDriver->EnableDepthRead(true);
}

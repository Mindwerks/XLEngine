#ifndef LFD_FILM_H
#define LFD_FILM_H

#include <math.h>
#include "../CommonTypes.h"
#include "../fileformats/LFD_Anim.h"
#include "../movieplayback/MoviePlayer.h"

#include <vector>
using namespace std;

class LFD_Anim;
class Archive;

class LFD_Film : public MoviePlayer
{
public:
	LFD_Film(IDriver3D *pDriver);

	bool Start(Archive *pRes0, Archive *pRes1, const char *pszFile, u32 uFlags, s32 nSpeed);
	void Stop();

	bool Update();
	void Render(f32 fDeltaTime);

private:
	struct FilmEntry
	{
		char szType[4];		// type of the resource
		char szName[8];		// name of the resource
		s32 nLength;		// length of the resource including this structure
		s16 nBlockType;
		s16 nNumCommands;
		s16 nBlockM22;
	};

	struct Command
	{
		s16 type;
		s16 cmd;
		s32 val[4];
		union
		{
            void *pData;
            int nData;
		};
	};

	struct CommandSet
	{
		bool bActivated;
		vector<Command *> commands;
	};

	struct Graphic
	{
		LFD_Anim *pAnim;
		s8 bVisible;
		s32 nX, nY;
		s16 nFrame;
		s16 nLayer;
		s16 nType;
		s16 nAnimate;
		s16 anScroll[2];
		char szFile[32];
		bool bLoop;
		bool bNoReverse;

		Graphic *pNext;
	};

	enum
	{
		FADE_TO_BLACK=0,
		FADE_FROM_BLACK,
		FADE_TO_THEN_FROM_BLACK,
		FADE_RIGHT_ON,
		FADE_RIGHT_OFF,
		FADE_RIGHT_ON_OFF,
		FADE_LEFT_ON,
		FADE_LEFT_OFF,
		FADE_LEFT_ON_OFF,
		FADE_UP_ON,
		FADE_UP_OFF,
		FADE_UP_ON_OFF,
		FADE_DOWN_ON,
		FADE_DOWN_OFF,
		FADE_DOWN_ON_OFF,
		FADE_CLEAR,
	};

	vector<Graphic *> m_Graphics;
	PLTT_File m_Pal[32];
	s32 m_nPalCount;
	s16 m_nFilmLength;

	Command m_CommandPool[1024];
	s32 m_nCmdPoolIdx;
	s32 m_nEntry;
	void *pFile;
	CommandSet *m_Commands;

	Archive *m_pReader;
	s32 m_nTime;

	f32 m_fFrameDelay;
	f32 m_fCurDelay;

	bool m_bFirstFrame;
	bool m_bTextCrawl;
	s32 m_nCut;
	f32 m_fCutTime;
	f32 m_fCurCutTime;

	static Graphic m_GraphicsCache[1024];
	static PLTT_File m_PalCache[32];
	static s32 m_nGCacheIdx;
	static s32 m_nPalCacheIdx;
	static Graphic *m_apAnimQueue[202];

	s32 ParseCommand(s32 cmd, char *pData);
	void StartCut(s32 how, s32 type);
	static void AddGraphicToQueue(Graphic *pGraphic, s32 nLayer);
};

#endif //LFD_FILM_H

#ifndef CUTSCENE_PLAYER
#define CUTSCENE_PLAYER

#include "../CommonTypes.h"

struct XLEngine_Plugin_API;

class CutscenePlayer
{
public:
	static bool Init(const XLEngine_Plugin_API *API);
	static void Destroy();

	static void StartCutscene(int nID);
	static int IsCutscenePlaying();

	static void Update();
	static void Render(f32 fDeltaTime);

	static void KeyDown(s32 key);

private:
	struct Cutscene
	{
		s16 num;
		s16 speed;
		s16 nextScene;
		s16 skipScene;
		s16 midiSeq;
		s16 midiVol;
		char resLFD[32];
		char scene[32];
	};

	static Cutscene m_CutSceneList[256];
	static s32 m_nCutSceneCount;
	static s32 m_nCutSceneNum;
	static bool m_bCutscenePlaying;

	static Cutscene *m_pCurCutScene;
	static const XLEngine_Plugin_API *m_pAPI;

private:
	static void LoadCutsceneData();
	static void CutScene_ParseLine(char *pszLine);
};

#endif //CUTSCENE_PLAYER
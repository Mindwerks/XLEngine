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
    static void Render(float fDeltaTime);

    static void KeyDown(int32_t key);

private:
    struct Cutscene
    {
        int16_t num;
        int16_t speed;
        int16_t nextScene;
        int16_t skipScene;
        int16_t midiSeq;
        int16_t midiVol;
        char resLFD[32];
        char scene[32];
    };

    static Cutscene m_CutSceneList[256];
    static int32_t m_nCutSceneCount;
    static int32_t m_nCutSceneNum;
    static bool m_bCutscenePlaying;

    static Cutscene *m_pCurCutScene;
    static const XLEngine_Plugin_API *m_pAPI;

private:
    static void LoadCutsceneData();
    static void CutScene_ParseLine(char *pszLine);
};

#endif //CUTSCENE_PLAYER
#ifndef LFD_FILM_H
#define LFD_FILM_H

#include "../CommonTypes.h"
#include "../fileformats/LFD_Anim.h"
#include "../movieplayback/MoviePlayer.h"

#include <cmath>
#include <vector>

class LFD_Anim;
class Archive;

class LFD_Film : public MoviePlayer
{
public:
    LFD_Film(IDriver3D *pDriver);

    virtual bool Start(Archive *pRes0, Archive *pRes1, const char *pszFile, uint32_t uFlags, int32_t nSpeed) override;
    virtual void Stop() override;

    virtual bool Update() override;
    virtual void Render(float fDeltaTime) override;

private:
    struct FilmEntry
    {
        char szType[4];     // type of the resource
        char szName[8];     // name of the resource
        int32_t nLength;        // length of the resource including this structure
        int16_t nBlockType;
        int16_t nNumCommands;
        int16_t nBlockM22;
    };

    struct Command
    {
        int16_t type;
        int16_t cmd;
        int32_t val[4];
        union
        {
            void *pData;
            int nData;
        };
    };

    struct CommandSet
    {
        bool bActivated;
        std::vector<Command *> commands;
    };

    struct Graphic
    {
        LFD_Anim *pAnim;
        int8_t bVisible;
        int32_t nX, nY;
        int16_t nFrame;
        int16_t nLayer;
        int16_t nType;
        int16_t nAnimate;
        int16_t anScroll[2];
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

    std::vector<Graphic *> m_Graphics;
    PLTT_File m_Pal[32];
    int32_t m_nPalCount;
    int16_t m_nFilmLength;

    Command m_CommandPool[1024];
    int32_t m_nCmdPoolIdx;
    int32_t m_nEntry;
    void *pFile;
    CommandSet *m_Commands;

    Archive *m_pReader;
    int32_t m_nTime;

    float m_fFrameDelay;
    float m_fCurDelay;

    bool m_bFirstFrame;
    bool m_bTextCrawl;
    int32_t m_nCut;
    float m_fCutTime;
    float m_fCurCutTime;

    static Graphic m_GraphicsCache[1024];
    static PLTT_File m_PalCache[32];
    static int32_t m_nGCacheIdx;
    static int32_t m_nPalCacheIdx;
    static Graphic *m_apAnimQueue[202];

    int32_t ParseCommand(int32_t cmd, char *pData);
    void StartCut(int32_t how, int32_t type);
    static void AddGraphicToQueue(Graphic *pGraphic, int32_t nLayer);
};

#endif //LFD_FILM_H

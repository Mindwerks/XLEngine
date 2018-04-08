#ifndef MOVIEPLAYER_H
#define MOVIEPLAYER_H

#include "../CommonTypes.h"

class Archive;

class IDriver3D;

class MoviePlayer {
public:
    MoviePlayer(IDriver3D *pDriver) {
        m_bPlaying = false;
        m_pDriver = pDriver;
    }

    virtual ~MoviePlayer() { Stop(); }

    virtual bool
    Start(Archive *pRes0, Archive *pRes1, const char *pszFile, uint32_t uFlags, int32_t nSpeed) { return false; }

    virtual void Stop() {};

    virtual bool Update() { return false; }

    virtual void Render(float fDeltaTime) {};

    bool IsPlaying() { return m_bPlaying; }

protected:
    bool m_bPlaying;
    IDriver3D *m_pDriver;
};

#endif //MOVIEPLAYER_H
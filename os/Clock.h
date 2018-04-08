#ifndef CLOCK_H
#define CLOCK_H

#include "../Engine.h"

class Clock
{
public:
    static bool Init();
    static void Destroy();
    static void StartTimer(int32_t timerID=0);
    static float GetDeltaTime(float fMax, int32_t timerID=0);
    static uint64_t GetDeltaTime_uS(int32_t timeID=0);

    static float m_fDeltaTime;
    static float m_fRealDeltaTime;
    static int32_t m_nDeltaTicks;
};

#endif //CLOCK_H
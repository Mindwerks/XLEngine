#include "../Clock.h"
#include <sys/time.h>
#include <cstdio>
#include <cassert>

#define SEC_TO_uS 1000000.0

float Clock::m_fDeltaTime;
float Clock::m_fRealDeltaTime;
int32_t Clock::m_nDeltaTicks;

static __time_t _StartTime_Sec;
static uint64_t _Start_Tick[16];
uint64_t _GetCurTickCnt();

bool Clock::Init()
{
    timeval t;
    gettimeofday(&t, 0);
    _StartTime_Sec = t.tv_sec;
    return true;
}

void Clock::Destroy()
{
    //nothing to do right now.
}

void Clock::StartTimer(int32_t timerID/*=0*/)
{
    assert( timerID < 16 );
    _Start_Tick[timerID] = _GetCurTickCnt();
}

float Clock::GetDeltaTime(float fMax, int32_t timerID/*=0*/)
{
    assert( timerID < 16 );
    uint64_t End = _GetCurTickCnt();
    uint64_t uDelta_uS = End - _Start_Tick[timerID];

    float fTimeDelta = (float)( (double)uDelta_uS / SEC_TO_uS );
    if ( fTimeDelta > fMax ) { fTimeDelta = fMax; }

    return fTimeDelta;
}

uint64_t Clock::GetDeltaTime_uS(int32_t timerID/*=0*/)
{
    uint64_t End = _GetCurTickCnt();
    return End - _Start_Tick[timerID];
}

uint64_t _GetCurTickCnt()
{
    timeval t;
    gettimeofday(&t, 0);
    uint64_t uDeltaSec = (uint64_t)(t.tv_sec-_StartTime_Sec);
    return uDeltaSec*(uint64_t)1000000 + (uint64_t)t.tv_usec;
}

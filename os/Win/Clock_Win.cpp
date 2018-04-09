#include "../Clock.h"
#include <windows.h>
#include <cstdio>
#include <cassert>

#define SEC_TO_uS 1000000.0

float Clock::m_fDeltaTime;
float Clock::m_fRealDeltaTime;
int32_t Clock::m_nDeltaTicks;

static LARGE_INTEGER _Timer_Freq;
static uint64_t _Start_Tick[16];
LONGLONG _GetCurTickCnt();

bool Clock::Init()
{
    BOOL bRet = QueryPerformanceFrequency(&_Timer_Freq);
    return (bRet) ? true : false;
}

void Clock::Destroy()
{
    //nothing to do currently.
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

    float fTimeDelta = (float)( (double)(End - _Start_Tick[timerID]) / (double)(_Timer_Freq.QuadPart) );
    if ( fTimeDelta > fMax ) 
    { 
        fTimeDelta = fMax; 
    }

    return fTimeDelta;
}

uint64_t Clock::GetDeltaTime_uS(int32_t timerID/*=0*/)
{
    uint64_t End = _GetCurTickCnt();
    double quadPart_uS = (double)(_Timer_Freq.QuadPart) / SEC_TO_uS;
    return (uint64_t)( (double)(End - _Start_Tick[timerID]) / quadPart_uS );
}

LONGLONG _GetCurTickCnt()
{
    LARGE_INTEGER lcurtick;
    QueryPerformanceCounter(&lcurtick);

    return lcurtick.QuadPart;
}

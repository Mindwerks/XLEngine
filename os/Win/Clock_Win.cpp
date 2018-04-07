#include "../Clock.h"
#include <windows.h>
#include <stdio.h>
#include <assert.h>

#define SEC_TO_uS 1000000.0

f32 Clock::m_fDeltaTime;
f32 Clock::m_fRealDeltaTime;
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

f32 Clock::GetDeltaTime(f32 fMax, int32_t timerID/*=0*/)
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
	f64 quadPart_uS = (f64)(_Timer_Freq.QuadPart) / SEC_TO_uS;
	return (uint64_t)( (f64)(End - _Start_Tick[timerID]) / quadPart_uS );
}

LONGLONG _GetCurTickCnt()
{
	LARGE_INTEGER lcurtick;
	QueryPerformanceCounter(&lcurtick);

	return lcurtick.QuadPart;
}

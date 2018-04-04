#include "../Clock.h"
#include <sys/time.h>
#include <stdio.h>
#include <assert.h>

#define SEC_TO_uS 1000000.0

f32 Clock::m_fDeltaTime;
f32 Clock::m_fRealDeltaTime;
s32 Clock::m_nDeltaTicks;

static __time_t _StartTime_Sec;
static u64 _Start_Tick[16];
u64 _GetCurTickCnt();

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

void Clock::StartTimer(s32 timerID/*=0*/)
{
	assert( timerID < 16 );
	_Start_Tick[timerID] = _GetCurTickCnt();
}

f32 Clock::GetDeltaTime(f32 fMax, s32 timerID/*=0*/)
{
	assert( timerID < 16 );
	u64 End = _GetCurTickCnt();
	u64 uDelta_uS = End - _Start_Tick[timerID];

	f32 fTimeDelta = (f32)( (f64)uDelta_uS / SEC_TO_uS );
	if ( fTimeDelta > fMax ) { fTimeDelta = fMax; }

	return fTimeDelta;
}

u64 Clock::GetDeltaTime_uS(s32 timerID/*=0*/)
{
	u64 End = _GetCurTickCnt();
	return End - _Start_Tick[timerID];
}

u64 _GetCurTickCnt()
{
	timeval t;
	gettimeofday(&t, 0);
	u64 uDeltaSec = (u64)(t.tv_sec-_StartTime_Sec);
	return uDeltaSec*(u64)1000000 + (u64)t.tv_usec;
}

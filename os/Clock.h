#ifndef CLOCK_H
#define CLOCK_H

#include "../Engine.h"

class Clock
{
public:
	static bool Init();
	static void Destroy();
	static void StartTimer(s32 timerID=0);
	static f32 GetDeltaTime(f32 fMax, s32 timerID=0);
	static u64 GetDeltaTime_uS(s32 timeID=0);

	static f32 m_fDeltaTime;
	static f32 m_fRealDeltaTime;
	static s32 m_nDeltaTicks;
};

#endif //CLOCK_H
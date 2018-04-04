#ifndef LEVELFUNC_DEFAULT_H
#define LEVELFUNC_DEFAULT_H

//This is temporary in order to get the system working.
//I'll move these over to the game code and proper scripts later.

#include "../CommonTypes.h"

class LevelFunc;

extern void SetupDefaultLevelFuncs();

//SlidingDoor
extern void LFunc_SlidingDoor_SetValue(LevelFunc *pFunc, s32 nSector, f32 value, bool bInstant);

//TriggerToggle
extern void LFunc_TriggerToggle_Activate(LevelFunc *pFunc, s32 mask, s32 items, bool bForce);

#endif //LEVELFUNC_DEFAULT_H

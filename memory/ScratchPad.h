#ifndef SCRATCHPAD_H
#define SCRATCHPAD_H

#include "../CommonTypes.h"

//Simple scratch pad, frame based allocator.
class ScratchPad
{
    public:
		static bool Init();
		static void Destroy();

		static void StartFrame();
		static void *AllocMem(u32 uSize);
		static void FreeFrame();
		static void FreeAllFrames();
    private:
		static u8 *m_pMemory;
		static u32 m_uFramePtr;

		static s32 m_nCurFrame;
		static u32 m_aFrames[];
};

#endif // SCRATCHPAD_H

#ifndef SCRATCHPAD_H
#define SCRATCHPAD_H

#include "../CommonTypes.h"

//Simple scratch pad, frame based allocator.
class ScratchPad {
public:
    static bool Init();

    static void Destroy();

    static void StartFrame();

    static void *AllocMem(uint32_t uSize);

    static void FreeFrame();

    static void FreeAllFrames();

private:
    static uint8_t *m_pMemory;
    static uint32_t m_uFramePtr;

    static int32_t m_nCurFrame;
    static uint32_t m_aFrames[];
};

#endif // SCRATCHPAD_H

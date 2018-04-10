#include "ScratchPad.h"
#include <cstring>
#include <cstdio>
#include <memory.h>
#include <cassert>

#define SCRATCH_PAD_SIZE 64*1024*1024
#define MAX_FRAME_COUNT 128

uint8_t *ScratchPad::m_pMemory=nullptr;
uint32_t ScratchPad::m_uFramePtr=0;

int32_t ScratchPad::m_nCurFrame;
uint32_t ScratchPad::m_aFrames[MAX_FRAME_COUNT];

bool ScratchPad::Init()
{
    m_uFramePtr = 0;
    m_pMemory = xlNew uint8_t[SCRATCH_PAD_SIZE];
    m_nCurFrame = 0;

    return m_pMemory ? true : false;
}

void ScratchPad::Destroy()
{
    if ( m_pMemory )
    {
        xlDelete [] m_pMemory;
    }
    m_pMemory = nullptr;
}

void ScratchPad::StartFrame()
{
    assert( m_nCurFrame < MAX_FRAME_COUNT );
    if ( m_nCurFrame < MAX_FRAME_COUNT )
    {
        m_aFrames[m_nCurFrame] = m_uFramePtr;
        m_nCurFrame++;
    }
}

void *ScratchPad::AllocMem(uint32_t uSize)
{
    if ( m_uFramePtr + uSize >= SCRATCH_PAD_SIZE )
    {
        assert(0);
        return nullptr;
    }

    uint32_t uLoc = m_uFramePtr;
    m_uFramePtr += uSize;

    return &m_pMemory[uLoc];
}

void ScratchPad::FreeFrame()
{
    if ( m_nCurFrame > 0 )
    {
        m_nCurFrame--;
        m_uFramePtr = m_aFrames[m_nCurFrame];
    }
}

void ScratchPad::FreeAllFrames()
{
    m_uFramePtr = 0;
    m_nCurFrame = 0;
}

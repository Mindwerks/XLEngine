#include "ScratchPad.h"
#include <string.h>
#include <stdio.h>
#include <memory.h>
#include <assert.h>

#define SCRATCH_PAD_SIZE 32*1024*1024
#define MAX_FRAME_COUNT 128

u8 *ScratchPad::m_pMemory=NULL;
u32 ScratchPad::m_uFramePtr=0;

s32 ScratchPad::m_nCurFrame;
u32 ScratchPad::m_aFrames[MAX_FRAME_COUNT];

bool ScratchPad::Init()
{
	m_uFramePtr = 0;
	m_pMemory = xlNew u8[SCRATCH_PAD_SIZE];
	m_nCurFrame = 0;

	return m_pMemory ? true : false;
}

void ScratchPad::Destroy()
{
	if ( m_pMemory )
	{
		xlDelete [] m_pMemory;
	}
	m_pMemory = NULL;
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

void *ScratchPad::AllocMem(u32 uSize)
{
	if ( m_uFramePtr + uSize >= SCRATCH_PAD_SIZE )
	{
		assert(0);
		return NULL;
	}

	u32 uLoc = m_uFramePtr;
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

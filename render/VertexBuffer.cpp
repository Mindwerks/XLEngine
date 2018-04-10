#include "VertexBuffer.h"
#include "IDriver3D.h"
#include <cassert>
#include <cstdlib>
#include <memory.h>

VertexBuffer::VertexBuffer(IDriver3D *pDriver)
{
    m_uStride = 0;
    m_uCount = 0;
    m_uSize = 0;
    m_uVBO_ID = 0xffffffff;
    m_bLocked = false;
    m_pMemory = nullptr;

    m_pDriver = pDriver;
}

VertexBuffer::~VertexBuffer(void)
{
    if ( m_pMemory )
    {
        xlFree(m_pMemory);
        m_pMemory = nullptr;
    }
    if ( m_uVBO_ID != 0xffffffff )
    {
        m_pDriver->DeleteBuffer( m_uVBO_ID );
        m_uVBO_ID = 0xffffffff;
    }
}

void VertexBuffer::Set()
{
    m_pDriver->SetVBO(m_uVBO_ID, m_uStride, m_uVBO_Flags);
}

bool VertexBuffer::Create(uint32_t uStride, uint32_t uCount, bool bDynamic, uint32_t uVBO_Flags)
{
    bool bSuccess = true;
    m_pMemory = xlMalloc( uStride * uCount );
    if ( m_pMemory == nullptr )
        bSuccess = false;

    m_uStride = uStride;
    m_uCount = uCount;
    m_uSize = m_uStride * m_uCount;
    m_uVBO_Flags = uVBO_Flags;
    m_bDynamic = bDynamic;

    if ( m_uVBO_ID == 0xffffffff )
    {
        m_uVBO_ID = m_pDriver->CreateVBO();
        //set the size now.
        m_pDriver->AllocVBO_Mem(m_uVBO_ID, m_uCount, m_uSize, m_bDynamic);
    }

    return bSuccess;
}

void VertexBuffer::Destroy()
{
    if ( m_uVBO_ID != 0xffffffff )
    {
        m_pDriver->DeleteBuffer( m_uVBO_ID );
        m_uVBO_ID = 0xffffffff;
    }
    if ( m_pMemory )
    {
        xlFree(m_pMemory);
        m_pMemory = nullptr;
    }
    m_uCount = 0;
    m_uSize = 0;
}

void VertexBuffer::Fill(void *pData)
{
    void *pDest = Lock();
    memcpy(pDest, pData, m_uSize);
    Unlock();

    m_pDriver->FillVBO(m_uVBO_ID, pData, m_uSize, m_bDynamic);
}

void *VertexBuffer::Lock()
{
    void *pRet = nullptr;
    assert( m_bLocked == false );
    if ( m_bLocked == false )
    {
        m_bLocked = true;
        pRet = m_pMemory;
    }
    return pRet;
}

void VertexBuffer::Unlock()
{
    assert( m_bLocked == true );
    m_bLocked = false;

    m_pDriver->FillVBO(m_uVBO_ID, m_pMemory, m_uSize, m_bDynamic);
}

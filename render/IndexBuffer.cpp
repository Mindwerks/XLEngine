#include "IndexBuffer.h"
#include "IDriver3D.h"
#include <cassert>
#include <stdlib.h>

IndexBuffer::IndexBuffer(IDriver3D *pDriver) {
    m_uCount = 0;
    m_uSize = 0;
    m_uIBO_ID = 0xffffffff;
    m_bLocked = false;
    m_pMemory = NULL;
    m_pDriver = pDriver;
}

IndexBuffer::~IndexBuffer(void) {
    if (m_pMemory)
    {
        xlFree(m_pMemory);
        m_pMemory = NULL;
    }
    if (m_uIBO_ID != 0xffffffff)
    {
        m_pDriver->DeleteBuffer(m_uIBO_ID);
        m_uIBO_ID = 0xffffffff;
    }
}

bool IndexBuffer::Create(uint32_t uCount, uint32_t uStride, bool bDynamic) {
    m_uStride = uStride;

    bool bSuccess = true;
    m_pMemory = (uint32_t *) xlMalloc(m_uStride * uCount);
    if (m_pMemory == NULL)
        bSuccess = false;

    m_uCount = uCount;
    m_uSize = m_uStride * uCount;

    if (m_uIBO_ID == 0xffffffff)
    {
        m_uIBO_ID = m_pDriver->CreateIB();
    }

    m_bDynamic = bDynamic;

    return bSuccess;
}

void IndexBuffer::Destroy() {
    if (m_pMemory)
    {
        xlFree(m_pMemory);
        m_pMemory = NULL;
    }
    if (m_uIBO_ID != 0xffffffff)
    {
        m_pDriver->DeleteBuffer(m_uIBO_ID);
        m_uIBO_ID = 0xffffffff;
    }
    m_uCount = 0;
    m_uSize = 0;
}

void IndexBuffer::Fill(uint32_t *pData) {
    uint32_t *pDest = Lock();
    memcpy(pDest, pData, m_uSize);
    Unlock();

    m_pDriver->FillIB(m_uIBO_ID, pData, m_uSize, m_bDynamic);
}

uint32_t *IndexBuffer::Lock() {
    uint32_t *pRet = NULL;
    assert(m_bLocked == false);
    if (m_bLocked == false)
    {
        m_bLocked = true;
        pRet = m_pMemory;
    }
    return pRet;
}

void IndexBuffer::Unlock() {
    assert(m_bLocked == true);
    m_bLocked = false;

    m_pDriver->FillIB(m_uIBO_ID, m_pMemory, m_uSize, m_bDynamic);
}

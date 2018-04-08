#ifndef INDEXBUFFER_H
#define INDEXBUFFER_H

#include "../CommonTypes.h"

class IDriver3D;

class IndexBuffer
{
public:
    IndexBuffer(IDriver3D *pDriver);
    ~IndexBuffer(void);

    bool Create(uint32_t uCount, uint32_t uStride, bool bDynamic);
    void Destroy();
    void Fill(uint32_t *pData);
    uint32_t *Lock();
    void Unlock();

    uint32_t GetCount()  { return m_uCount; }
    uint32_t GetSize()   { return m_uSize; }
    uint32_t GetStride() { return m_uStride; }

    uint32_t GetID() { return m_uIBO_ID; }

private:
    uint32_t m_uCount;
    uint32_t m_uSize;
    uint32_t m_uStride;
    uint32_t m_uIBO_ID;
    bool m_bLocked;
    bool m_bDynamic;
    uint32_t *m_pMemory;
    IDriver3D *m_pDriver;
};

#endif //INDEXBUFFER_H
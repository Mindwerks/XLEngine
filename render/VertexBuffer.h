#ifndef VERTEXBUFFER_H
#define VERTEXBUFFER_H

#include "../CommonTypes.h"

class IDriver3D;

class VertexBuffer
{
public:
    VertexBuffer(IDriver3D *pDriver);
    ~VertexBuffer();

    bool Create(uint32_t uStride, uint32_t uCount, bool bDynamic, uint32_t uVBO_Flags);
    void Destroy();
    void Fill(void *pData);
    void *Lock();
    void Unlock();

    uint32_t GetStride() { return m_uStride; }
    uint32_t GetCount()  { return m_uCount; }
    uint32_t GetSize()   { return m_uSize; }

    void Set();

private:
    uint32_t m_uStride;
    uint32_t m_uCount;
    uint32_t m_uSize;
    uint32_t m_uVBO_ID;
    uint32_t m_uVBO_Flags;
    bool m_bLocked;
    bool m_bDynamic;
    void *m_pMemory;
    IDriver3D *m_pDriver;
};

#endif //VERTEXBUFFER_H
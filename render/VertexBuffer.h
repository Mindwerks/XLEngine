#ifndef VERTEXBUFFER_H
#define VERTEXBUFFER_H

#include "../CommonTypes.h"

class IDriver3D;

class VertexBuffer
{
public:
	VertexBuffer(IDriver3D *pDriver);
	~VertexBuffer(void);

	bool Create(u32 uStride, u32 uCount, bool bDynamic, u32 uVBO_Flags);
	void Destroy();
	void Fill(void *pData);
	void *Lock();
	void Unlock();

	u32 GetStride() { return m_uStride; }
	u32 GetCount()  { return m_uCount; }
	u32 GetSize()   { return m_uSize; }

	void Set();

private:
	u32 m_uStride;
	u32 m_uCount;
	u32 m_uSize;
	u32 m_uVBO_ID;
	u32 m_uVBO_Flags;
	bool m_bLocked;
	bool m_bDynamic;
	void *m_pMemory;
	IDriver3D *m_pDriver;
};

#endif //VERTEXBUFFER_H
#ifndef INDEXBUFFER_H
#define INDEXBUFFER_H

#include "../CommonTypes.h"

class IDriver3D;

class IndexBuffer
{
public:
	IndexBuffer(IDriver3D *pDriver);
	~IndexBuffer(void);

	bool Create(u32 uCount, u32 uStride, bool bDynamic);
	void Destroy();
	void Fill(u32 *pData);
	u32 *Lock();
	void Unlock();

	u32 GetCount()  { return m_uCount; }
	u32 GetSize()   { return m_uSize; }
	u32 GetStride() { return m_uStride; }

	u32 GetID() { return m_uIBO_ID; }

private:
	u32 m_uCount;
	u32 m_uSize;
	u32 m_uStride;
	u32 m_uIBO_ID;
	bool m_bLocked;
	bool m_bDynamic;
	u32 *m_pMemory;
	IDriver3D *m_pDriver;
};

#endif //INDEXBUFFER_H
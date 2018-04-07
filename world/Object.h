#ifndef OBJECT_H
#define OBJECT_H

#include "../CommonTypes.h"
#include "../math/Vector3.h"
#include "../math/Matrix.h"
#include "ObjectDef.h"
#include "RenderComponent.h"
#include "CollisionComponent.h"
#include <vector>
#include <string>
#include <stdlib.h>

using namespace std;

class Logic;
class IDriver3D;
class LightObject;

class Object
{
public:
	enum
	{
		MAX_LIGHT_COUNT = 32
	};
public:
	Object();
	virtual ~Object();

	void SetName(const string& sName) { m_Name = sName; }
	const string& GetName() { return m_Name; }

	void Update();
	void Init();
	void Reset();
	void SendMessage(uint32_t uMsgID, f32 fValue);

	void SetLoc(const Vector3& vLoc) { m_ObjPhysicsData.m_Loc = vLoc; }
	void GetLoc(Vector3& vLoc)		 { vLoc = m_ObjPhysicsData.m_Loc; }

	void SetDir(const Vector3& vDir) { m_ObjPhysicsData.m_Dir = vDir; }
	void GetDir(Vector3& vDir)		 { vDir = m_ObjPhysicsData.m_Dir; }

	void SetUp(const Vector3& vDir)  { m_ObjPhysicsData.m_Up = vDir; }
	void GetUp(Vector3& vDir)		 { vDir = m_ObjPhysicsData.m_Up; }

	void SetScale(const Vector3& vScale) { m_ObjPhysicsData.m_Scale = vScale; }
	void GetScale(Vector3& vScale)		 { vScale = m_ObjPhysicsData.m_Scale; }

	void SetSector(uint16_t uSector) { m_ObjPhysicsData.m_uSector = uSector; }
	uint16_t  GetSector()			{ return m_ObjPhysicsData.m_uSector; }

	void AddLogic(Logic *pLogic);

	void SetGameData(void *pData) { m_pDataComp = pData; }
	void *GetGameData()			  { return m_pDataComp; }
	void AllocGameData(uint32_t uSize) { m_pDataComp = xlMalloc(uSize); m_bAllocGameData = true; }
	ObjectPhysicsData *GetPhysicsData() { return &m_ObjPhysicsData; }
	void EnableCollision(bool bEnable) { m_bCollisionEnable = bEnable; }

	void SetRenderComponent(RenderComponent *pRender) { m_pRenderComp = pRender; }
	RenderComponent *GetRenderComponent() { return m_pRenderComp; }
	void Render(IDriver3D *pDriver, f32 fIntensity, const Vector3& vOffset) { if (m_pRenderComp && (m_uFlags&OBJFLAGS_ACTIVE)) m_pRenderComp->Render(this, pDriver, fIntensity, vOffset); }

	void SetCollisionComponent(CollisionComponent *pCollision) { m_pCollisionComp = pCollision; }
	bool Collide(CollisionPacket *packet, const Vector3& vOffset) { if (m_pCollisionComp&&m_bCollisionEnable && (m_uFlags&OBJFLAGS_ACTIVE)) return m_pCollisionComp->Collide(packet, &m_worldMtx, vOffset); else return false; }
	bool Raycast(RaycastPacket *packet, Sector *pSector, const Vector3& vOffset) { if (m_pCollisionComp && (m_uFlags&OBJFLAGS_ACTIVE)) return m_pCollisionComp->Raycast(packet, &m_worldMtx, this, pSector, vOffset); else return false; }

	uint32_t GetID() { return m_uID; }
	void SetID(uint32_t ID) { m_uID = ID; }

	uint32_t GetGameID() { return m_uGameID; }
	void SetGameID(uint32_t uID) { m_uGameID = uID; }

	f32 GetBrightness() { return m_fBrightness; }
	void SetBrightness(f32 fBrightness) { m_fBrightness = fBrightness; }

	void SetMatrix(const Matrix& worldMtx) { m_worldMtx = worldMtx; }
	Matrix *GetMatrixPtr() { return &m_worldMtx; }
	void ComputeTransformedBounds();
	void GetWorldBounds(Vector3& vMin, Vector3& vMax) { vMin = m_worldBounds[0]; vMax = m_worldBounds[1]; }
	Vector3 GetWorldMin() { return m_worldBounds[0]; }
	Vector3 GetWorldMax() { return m_worldBounds[1]; }
	void SetWorldBounds(const Vector3& vMin, const Vector3& vMax) { m_worldBounds[0]= vMin; m_worldBounds[1] = vMax; }
	float GetBoundingSphere(Vector3& vCen) { vCen = m_worldCen; return m_fRadius; }
	void SetBoundingSphere(const Vector3& vCen, float fRadius) { m_worldCen = vCen; m_fRadius = fRadius; }

	void SetRenderKey(uint32_t uKey) { m_uRenderKey = uKey; }
	uint32_t GetRenderKey() { return m_uRenderKey; }

	void AddLight(const LightObject *pLight) { if ( m_nLightCount < MAX_LIGHT_COUNT ) m_apLights[ m_nLightCount++ ] = pLight; }
	const LightObject **GetLightList(int& rnLightCnt) { rnLightCnt = m_nLightCount; return m_apLights; }

	uint16_t GetRefCnt() { return m_uRefCnt; }
	void AddRef() { m_uRefCnt++; }
	uint16_t Release();

	void SetFlag(uint32_t uFlag)   { m_uFlags |= uFlag; }
	void ClearFlag(uint32_t uFlag) { m_uFlags &= ~uFlag; }
	bool IsFlagSet(uint32_t uFlag) { return (m_uFlags&uFlag)!=0; }

	int32_t GetWorldX() { return m_ObjPhysicsData.m_worldX; }
	int32_t GetWorldY() { return m_ObjPhysicsData.m_worldY; }
	void SetWorldPos(int32_t x, int32_t y) { m_ObjPhysicsData.m_worldX = x; m_ObjPhysicsData.m_worldY = y; }

public:
	enum ObjFlags_e
	{
		OBJFLAGS_NONE		= 0,
		OBJFLAGS_ACTIVE		= (1<<0),
		OBJFLAGS_RENDERABLE = (1<<1),
		OBJFLAGS_DYNAMIC	= (1<<2),
	};

protected:
	string m_Name;

	uint16_t m_uPad;
	uint16_t m_uRefCnt;
	uint32_t m_uRenderKey;

	ObjectPhysicsData m_ObjPhysicsData;
	uint32_t m_uFlags;
	uint32_t m_uID;
	uint32_t m_uGameID;	//used for game specific purposes.
	f32 m_fBrightness;
	vector<Logic *> m_Logics;
	void *m_pDataComp;
	RenderComponent *m_pRenderComp;
	CollisionComponent *m_pCollisionComp;

	Matrix m_worldMtx;
	Vector3 m_worldBounds[2];
	Vector3 m_worldCen;
	float m_fRadius;

	int m_nLightCount;
	const LightObject *m_apLights[MAX_LIGHT_COUNT];

	bool m_bAllocGameData;
	bool m_bCollisionEnable;
};

#endif //OBJECT_H

#include "Object.h"
#include "ObjectManager.h"
#include "Logic.h"
#include "Sector.h"
#include <assert.h>
#include <float.h>

Object::Object()
{
	m_ObjPhysicsData.m_uSector = INVALID_SECTOR;
	m_uFlags  = OBJFLAGS_ACTIVE;
	m_uID     = 0;
	m_uRefCnt = 0;
	m_uRenderKey = 0xffffffff;
	
	m_pDataComp   = NULL;
	m_pRenderComp = NULL;
	m_pCollisionComp = NULL;

	m_ObjPhysicsData.m_Loc.Set(0.0f, 0.0f, 0.0f);
	m_ObjPhysicsData.m_Dir.Set(0.0f, 1.0f, 0.0f);
	m_ObjPhysicsData.m_Up.Set(0.0f, 0.0f, 1.0f);
	m_ObjPhysicsData.m_Scale.Set(1.0f, 1.0f, 1.0f);
	m_ObjPhysicsData.m_worldX = 0;
	m_ObjPhysicsData.m_worldY = 0;

	m_worldMtx.Identity();
	m_Name = "Default";

	m_nLightCount = 0;
	m_bAllocGameData = false;
	m_bCollisionEnable = true;
}

Object::~Object()
{
	Reset();
}

void Object::Reset()
{
	m_uRefCnt = 0;
	m_Name = "Default";

	m_ObjPhysicsData.m_uSector = INVALID_SECTOR;
	m_uFlags  = OBJFLAGS_ACTIVE;
	m_uRenderKey = 0xffffffff;
	m_fBrightness = 1.0f;
	m_ObjPhysicsData.m_Loc.Set(0.0f, 0.0f, 0.0f);
	m_ObjPhysicsData.m_Dir.Set(0.0f, 1.0f, 0.0f);
	m_ObjPhysicsData.m_Up.Set(0.0f, 0.0f, 1.0f);
	m_ObjPhysicsData.m_Scale.Set(1.0f, 1.0f, 1.0f);
	if ( m_pDataComp )	//this cannot be freed by the system, it must be freed by the caller.
	{
		if ( m_bAllocGameData )
		{
			xlFree(m_pDataComp);
		}
		m_pDataComp = NULL;
	}
	if ( m_pRenderComp )
	{
		//xlDelete m_pRenderComp;
		m_pRenderComp = NULL;
	}

	m_Logics.clear();
	m_worldMtx.Identity();

	m_bCollisionEnable = true;
}

u16 Object::Release() 
{ 
	assert(m_uRefCnt);
	if ( m_uRefCnt ) 
		m_uRefCnt--; 

	if ( m_uRefCnt <= 0 )
	{
		ObjectManager::FreeObject(this);
		return 0;
	}

	return m_uRefCnt; 
}

void Object::AddLogic(Logic *pLogic)
{
	m_Logics.push_back( pLogic );
}

//initialize the object
void Object::Init()
{
	vector<Logic *>::iterator iLogic = m_Logics.begin();
	vector<Logic *>::iterator eLogic = m_Logics.end();

	for (; iLogic != eLogic; ++iLogic)
	{
		(*iLogic)->InitObject(this);
	}
}

void Object::SendMessage(u32 uMsgID, f32 fValue)
{
	vector<Logic *>::iterator iLogic = m_Logics.begin();
	vector<Logic *>::iterator eLogic = m_Logics.end();

	for (; iLogic != eLogic; ++iLogic)
	{
		(*iLogic)->SendMessage(this, uMsgID, fValue);
	}
}

void Object::Update()
{
	if ( m_uFlags&OBJFLAGS_ACTIVE )
	{
		vector<Logic *>::iterator iLogic = m_Logics.begin();
		vector<Logic *>::iterator eLogic = m_Logics.end();

		for (; iLogic != eLogic; ++iLogic)
		{
			(*iLogic)->Update(this);
		}
	}
}

void Object::ComputeTransformedBounds()
{
	if ( m_pRenderComp == NULL )
		return;

	Vector3 vMin, vMax;
	m_pRenderComp->GetBounds(vMin, vMax);

	Vector3 avCorners[8], avWorldCorners[8];
	avCorners[0].Set(vMin.x, vMin.y, vMin.z);
	avCorners[1].Set(vMax.x, vMin.y, vMin.z);
	avCorners[2].Set(vMax.x, vMax.y, vMin.z);
	avCorners[3].Set(vMin.x, vMax.y, vMin.z);
	avCorners[4].Set(vMin.x, vMin.y, vMax.z);
	avCorners[5].Set(vMax.x, vMin.y, vMax.z);
	avCorners[6].Set(vMax.x, vMax.y, vMax.z);
	avCorners[7].Set(vMin.x, vMax.y, vMax.z);

	m_worldCen.Set(0,0,0);
	for (int i=0; i<8; i++)
	{
		avWorldCorners[i] = m_worldMtx.TransformVector(avCorners[i]);
		m_worldCen = m_worldCen + avWorldCorners[i];
	}
	m_worldCen = m_worldCen * (1.0f/8.0f);

	float fMaxDist = 0.0f, fDist;
	m_worldBounds[0].Set(FLT_MAX, FLT_MAX, FLT_MAX);
	m_worldBounds[1] = -m_worldBounds[0];
	for (int i=0; i<8; i++)
	{
		Vector3 vOffs = avWorldCorners[i] - m_worldCen;
		fDist = vOffs.Dot(vOffs);
		if ( fDist > fMaxDist )
			fMaxDist = fDist;

		if ( avWorldCorners[i].x < m_worldBounds[0].x ) m_worldBounds[0].x = avWorldCorners[i].x;
		if ( avWorldCorners[i].y < m_worldBounds[0].y ) m_worldBounds[0].y = avWorldCorners[i].y;
		if ( avWorldCorners[i].z < m_worldBounds[0].z ) m_worldBounds[0].z = avWorldCorners[i].z;

		if ( avWorldCorners[i].x > m_worldBounds[1].x ) m_worldBounds[1].x = avWorldCorners[i].x;
		if ( avWorldCorners[i].y > m_worldBounds[1].y ) m_worldBounds[1].y = avWorldCorners[i].y;
		if ( avWorldCorners[i].z > m_worldBounds[1].z ) m_worldBounds[1].z = avWorldCorners[i].z;
	}
	m_fRadius = sqrtf(fMaxDist)+0.01f;
}
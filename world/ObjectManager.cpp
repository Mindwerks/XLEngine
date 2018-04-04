#include "ObjectManager.h"
#include "LogicManager.h"
#include "../render/Mesh.h"
#include "../world/Sprite_ZAxis.h"
#include "../world/World.h"
#include "../world/Terrain.h"

vector<Object *> ObjectManager::m_ObjectPool;
vector<Object *> ObjectManager::m_FreeObjects;
vector<Object *> ObjectManager::m_ActiveObjects;
u32 ObjectManager::m_uReservedCount;
World *ObjectManager::m_pWorld;

#define OBJCOUNT_PER_BANK 128

bool ObjectManager::Init(World *pWorld)
{
	LogicManager::Init();

	Object *objPool = xlNew Object[OBJCOUNT_PER_BANK];
	m_ObjectPool.push_back( objPool );

	//now add all the objects to the free list.
	//objects are added in reverse order so that they are allocated from the free
	//list in chronological order.
	//The free list works by popping elements off the back for speed (no need to shuffle memory around).
	for (s32 i=OBJCOUNT_PER_BANK-1; i>=0; i--)
	{
		objPool[i].SetID(i);
		m_FreeObjects.push_back( &objPool[i] );
	}

	m_uReservedCount = 0;
	m_pWorld = pWorld;

	return true;
}

void ObjectManager::Destroy()
{
	LogicManager::Destroy();

	vector<Object *>::iterator iObjPool = m_ObjectPool.begin();
	vector<Object *>::iterator eObjPool = m_ObjectPool.end();
	for (; iObjPool != eObjPool; ++iObjPool)
	{
		Object *pool = *iObjPool;
		xlDelete [] pool;
	}
	m_ObjectPool.clear();
	m_FreeObjects.clear();
	m_ActiveObjects.clear();
}

Object *ObjectManager::CreateObject(const string& sName)
{
	Object *pObj = NULL;
	if ( m_FreeObjects.size() )
	{
		//if there is a free object, just use it.
		pObj = m_FreeObjects.back();
		m_FreeObjects.pop_back();
	}
	else
	{
		//otherwise allocate another bank for the pool...
		u32 uID_Start = (u32)m_ObjectPool.size()*OBJCOUNT_PER_BANK;

		Object *objPool = xlNew Object[OBJCOUNT_PER_BANK];
		m_ObjectPool.push_back( objPool );

		for (s32 i=OBJCOUNT_PER_BANK-2; i>=0; i--)
		{
			objPool[i].SetID(i+uID_Start);
			m_FreeObjects.push_back( &objPool[i] );
		}
		objPool[OBJCOUNT_PER_BANK-1].SetID( uID_Start+OBJCOUNT_PER_BANK-1 );
		pObj = &objPool[OBJCOUNT_PER_BANK-1];
	}
	if ( pObj )
	{
		pObj->Reset();
		pObj->AddRef();
		pObj->SetName( sName );

		m_ActiveObjects.push_back( pObj );
	}
	return pObj;
}

void ObjectManager::SetRenderComponent(u32 uID, const char *pszComponent)
{
	Object *pObj = GetObjectFromID(uID);
	if ( pObj == NULL )
		return;

	if ( stricmp(pszComponent, "Mesh") == 0 )
	{
		pObj->SetRenderComponent( xlNew Mesh() );	
	}
	else if ( stricmp(pszComponent, "Sprite_ZAxis") == 0 )
	{
		pObj->SetRenderComponent( xlNew Sprite_ZAxis() );	
	}
}

void ObjectManager::SetRenderTexture(u32 uID, TextureHandle hTex)
{
	Object *pObj = GetObjectFromID(uID);
	if ( pObj )
	{
		RenderComponent *pRenderComp = pObj->GetRenderComponent();
		if ( pRenderComp )
		{
			pRenderComp->SetTextureHandle(hTex);
		}
	}
}

void ObjectManager::SetRenderFlip(u32 uID, XL_BOOL bFlipX, XL_BOOL bFlipY)
{
	Object *pObj = GetObjectFromID(uID);
	if ( pObj )
	{
		RenderComponent *pRenderComp = pObj->GetRenderComponent();
		if ( pRenderComp )
		{
			pRenderComp->SetUV_Flip(bFlipX!=0, bFlipY!=0);
		}
	}
}

void ObjectManager::SetActive(u32 uID, XL_BOOL bActive)
{
	Object *pObj = GetObjectFromID(uID);
	if ( pObj )
	{
		if ( bActive )
		{
			pObj->SetFlag( Object::OBJFLAGS_ACTIVE );
		}
		else
		{
			pObj->ClearFlag( Object::OBJFLAGS_ACTIVE );
		}
	}
}

void ObjectManager::SetWorldBounds_API(u32 uID, float minX, float minY, float minZ, float maxX, float maxY, float maxZ)
{
	Object *pObj = GetObjectFromID(uID);
	if ( pObj )
	{
		pObj->SetWorldBounds( Vector3(minX, minY, minZ), Vector3(maxX, maxY, maxZ) );
	}
}

u32 ObjectManager::CreateObjectID(const char *pszName, s32 nSector)
{
	Object *pObj = CreateObject(pszName);
	if ( pObj )
	{
		//flag objects created outside the loaders as "dynamic"
		pObj->SetFlag( Object::OBJFLAGS_DYNAMIC );
		if ( nSector == 0xffff )
		{
			Terrain *pTerrain = m_pWorld->GetTerrain();
			if ( pTerrain )
			{
				pTerrain->AddDynamicObject( pObj->GetID() );
			}
		}

		return pObj->GetID();
	}
	return 0xffffffff;
}

void ObjectManager::MoveDynamicObjects(const Vector3& vMove)
{
	vector<Object *>::iterator iObj = m_ActiveObjects.begin();
	vector<Object *>::iterator eObj = m_ActiveObjects.end();
	for (; iObj != eObj; ++iObj)
	{
		Object *pObj = *iObj;
		if ( pObj->IsFlagSet( Object::OBJFLAGS_DYNAMIC ) )
		{
			//Update the object location.
			Vector3 vObjLoc;
			pObj->GetLoc(vObjLoc);
			vObjLoc = vObjLoc + vMove;
			pObj->SetLoc(vObjLoc);

			//Update the matrix.
			Matrix *pMtx = pObj->GetMatrixPtr();
			pMtx->m[12] = vObjLoc.x;
			pMtx->m[13] = vObjLoc.y;
			pMtx->m[14] = vObjLoc.z;

			//Update the world bounds.
			Vector3 vWorldMin, vWorldMax;
			pObj->GetWorldBounds(vWorldMin, vWorldMax);
			vWorldMin = vWorldMin + vMove;
			vWorldMax = vWorldMax + vMove;
			pObj->SetWorldBounds(vWorldMin, vWorldMax);
		}
	}
}

Object *ObjectManager::FindObject(const string& sName)
{
	vector<Object *>::iterator iObj = m_ActiveObjects.begin();
	vector<Object *>::iterator eObj = m_ActiveObjects.end();
	for (; iObj != eObj; ++iObj)
	{
		Object *pObj = *iObj;
		if ( pObj->GetName() == sName )
		{
			return pObj;
		}
	}
	return NULL;
}

Object *ObjectManager::GetObjectFromID(u32 uID)
{
	//1. Which bank is it in?
	u32 uBank  = uID / OBJCOUNT_PER_BANK;
	u32 uIndex = uID - uBank*OBJCOUNT_PER_BANK;

	Object *pool = m_ObjectPool[uBank];
	
	return &pool[uIndex];
}

void ObjectManager::FreeObject(Object *pObj)
{
	pObj->Reset();
	m_FreeObjects.push_back( pObj );

	//remove this from the active object list.
	vector<Object *>::iterator iObj = m_ActiveObjects.begin();
	vector<Object *>::iterator eObj = m_ActiveObjects.end();
	for (; iObj != eObj; ++iObj)
	{
		if ( (*iObj) == pObj )
		{
			m_ActiveObjects.erase( iObj );
			break;
		}
	}
}

void ObjectManager::FreeObjectID(u32 uID)
{
	Object *pObj = GetObjectFromID(uID);
	if ( pObj )
	{
		FreeObject(pObj);
	}
}

void ObjectManager::FreeAllObjects()
{
	m_FreeObjects.clear();
	u32 uStartObj = m_uReservedCount;
	vector<Object *>::iterator iBank = m_ObjectPool.begin();
	vector<Object *>::iterator eBank = m_ObjectPool.end();
	for (; iBank != eBank; ++iBank)
	{
		Object *pool = *iBank;
		for (u32 i=uStartObj; i<OBJCOUNT_PER_BANK; i++)
		{
			pool[i].Reset();
			m_FreeObjects.push_back( &pool[i] );
		}
		uStartObj = 0;
	}

	m_ActiveObjects.clear();
	//make only "reserved" objects active.
	Object *pool = m_ObjectPool[0];
	for (u32 i=0; i<m_uReservedCount; i++)
	{
		m_ActiveObjects.push_back( &pool[i] );
	}
}

void ObjectManager::Update()
{
	vector<Object *>::iterator iObj = m_ActiveObjects.begin();
	vector<Object *>::iterator eObj = m_ActiveObjects.end();
	for (; iObj != eObj; ++iObj)
	{
		(*iObj)->Update();
	}
}

void ObjectManager::AddLogicToObjID(u32 uID, const char *szLogicName)
{
	Object *pObj = GetObjectFromID(uID);
	if ( pObj )
	{
		Logic *pLogic = LogicManager::GetLogic(szLogicName);
		if ( pLogic )
		{
			pObj->AddLogic( pLogic );
		}
	}
}

ObjectPhysicsData *ObjectManager::GetObjectPhysicsData(u32 uID)
{
	Object *pObj = GetObjectFromID(uID);
	if ( pObj )
	{
		return pObj->GetPhysicsData();
	}
	return NULL;
}

void *ObjectManager::GetObjectGameData(u32 uID)
{
	Object *pObj = GetObjectFromID(uID);
	if ( pObj )
	{
		return pObj->GetGameData();
	}
	return NULL;
}

void ObjectManager::SetObjectGameData(u32 uID, void *pData)
{
	Object *pObj = GetObjectFromID(uID);
	if ( pObj )
	{
		pObj->SetGameData(pData);
		pObj->Init();
	}
}

void ObjectManager::SetObjectAngles(u32 uID, float x, float y, float z)
{
	Object *pObj = GetObjectFromID(uID);
	if ( pObj )
	{
		Vector3 vPos;
		pObj->GetLoc(vPos);

		Matrix mWorld;
		mWorld.Identity();
		mWorld.EulerToMatrix(x, y, z);
		
		mWorld.m[12] = vPos.x;
		mWorld.m[13] = vPos.y;
		mWorld.m[14] = vPos.z;

		pObj->SetMatrix(mWorld);
		pObj->ComputeTransformedBounds();
	}
}

void ObjectManager::SetObjectPos(u32 uID, float x, float y, float z)
{
	Object *pObj = GetObjectFromID(uID);
	if ( pObj )
	{
		pObj->SetLoc( Vector3(x,y,z) );

		Matrix *pWorldMtx = pObj->GetMatrixPtr();
		pWorldMtx->m[12] = x;
		pWorldMtx->m[13] = y;
		pWorldMtx->m[14] = z;

		pObj->ComputeTransformedBounds();
	}
}

void ObjectManager::EnableObjectCollision(u32 uID, s32 enable)
{
	Object *pObj = GetObjectFromID(uID);
	if ( pObj )
	{
		pObj->EnableCollision(enable ? true : false);
	}
}

void ObjectManager::SendMessage(u32 uID, u32 uMsg, f32 fValue)
{
	Object *pObj = GetObjectFromID(uID);
	if ( pObj )
	{
		pObj->SendMessage(uMsg, fValue);
	}
}
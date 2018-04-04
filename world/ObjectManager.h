#ifndef OBJECTMANAGER_H
#define OBJECTMANAGER_H

#include "../CommonTypes.h"
#include "Object.h"
#include <vector>
#include <string>

using namespace std;

class World;

class ObjectManager
{
public:
	static bool Init(World *pWorld);
	static void Destroy();
	static void FreeAllObjects();

	static void Update();

	//Set Always update to true if this object is constantly updated regardless of it's place in the map.
	//Only use this for "permanent" objects like the player.
	static Object *CreateObject(const string& sName);
	static Object *FindObject(const string& sName);
	static void MoveDynamicObjects(const Vector3& vMove);
	static void FreeObject(Object *pObj);
	static Object *GetObjectFromID(u32 uID);

	//Logic functions by ID - used by scripts and game code.
	static u32 CreateObjectID(const char *pszName, s32 nSector);
	static void FreeObjectID(u32 uID);
	static void AddLogicToObjID(u32 uID, const char *szLogicName);
	static ObjectPhysicsData *GetObjectPhysicsData(u32 uID);
	static void *GetObjectGameData(u32 uID);
	static void SetObjectGameData(u32 uID, void *pData);
	static void SetObjectAngles(u32 uID, float x, float y, float z);
	static void SetObjectPos(u32 uID, float x, float y, float z);
	static void EnableObjectCollision(u32 uID, s32 enable);
	static void SendMessage(u32 uID, u32 uMsg, f32 fValue);
	static void SetRenderComponent(u32 uID, const char *pszComponent);
	static void SetRenderTexture(u32 uID, TextureHandle hTex);
	static void SetRenderFlip(u32 uID, XL_BOOL bFlipX, XL_BOOL bFlipY);
	static void SetWorldBounds_API(u32 uID, float minX, float minY, float minZ, float maxX, float maxY, float maxZ);
	static void SetActive(u32 uID, XL_BOOL bActive);

	//How many objects are reserved.
	//These act as global objects that stick around between "levels" or loads.
	//A example would be the "Player"
	static void ReserveObjects(u32 uCount) { m_uReservedCount = uCount; }

private:
	static vector<Object *> m_ObjectPool;
	static vector<Object *> m_FreeObjects;
	static vector<Object *> m_ActiveObjects;
	static u32 m_uReservedCount;
	static World *m_pWorld;
};

#endif //OBJECTMANAGER_H

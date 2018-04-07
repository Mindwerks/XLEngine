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
	static Object *GetObjectFromID(uint32_t uID);

	//Logic functions by ID - used by scripts and game code.
	static uint32_t CreateObjectID(const char *pszName, int32_t nSector);
	static void FreeObjectID(uint32_t uID);
	static void AddLogicToObjID(uint32_t uID, const char *szLogicName);
	static ObjectPhysicsData *GetObjectPhysicsData(uint32_t uID);
	static void *GetObjectGameData(uint32_t uID);
	static void SetObjectGameData(uint32_t uID, void *pData);
	static void SetObjectAngles(uint32_t uID, float x, float y, float z);
	static void SetObjectPos(uint32_t uID, float x, float y, float z);
	static void EnableObjectCollision(uint32_t uID, int32_t enable);
	static void SendMessage(uint32_t uID, uint32_t uMsg, f32 fValue);
	static void SetRenderComponent(uint32_t uID, const char *pszComponent);
	static void SetRenderTexture(uint32_t uID, TextureHandle hTex);
	static void SetRenderFlip(uint32_t uID, XL_BOOL bFlipX, XL_BOOL bFlipY);
	static void SetWorldBounds_API(uint32_t uID, float minX, float minY, float minZ, float maxX, float maxY, float maxZ);
	static void SetActive(uint32_t uID, XL_BOOL bActive);

	//How many objects are reserved.
	//These act as global objects that stick around between "levels" or loads.
	//A example would be the "Player"
	static void ReserveObjects(uint32_t uCount) { m_uReservedCount = uCount; }

private:
	static vector<Object *> m_ObjectPool;
	static vector<Object *> m_FreeObjects;
	static vector<Object *> m_ActiveObjects;
	static uint32_t m_uReservedCount;
	static World *m_pWorld;
};

#endif //OBJECTMANAGER_H

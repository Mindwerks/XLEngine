#ifndef WORLD_H
#define WORLD_H

#include "../CommonTypes.h"
#include "../math/Vector2.h"
#include "../math/Vector3.h"
#include <vector>
#include <string>

using namespace std;
class WorldCell;
class IDriver3D;
class Object;
class Camera;
class Terrain;

class World
{
public:
	World();
	~World();

	void UnloadWorldCells();
	bool AddWorldCell(WorldCell *pCell);
	void RemoveWorldCell(WorldCell *pCell);

	void SetPlayer(Object *player) { m_Player = player; }
	Object *GetPlayer() { return m_Player; }

	void SetCamera(Camera *pCamera) { m_pCamera = pCamera; }
	Camera *GetCamera() { return m_pCamera; }

	void SetTerrain(Terrain *pTerrain) { m_pTerrain = pTerrain; }
	Terrain *GetTerrain() { return m_pTerrain; }

	bool Update(float dt, IDriver3D *pDriver);
	void Render(IDriver3D *pDriver);

	//Collision
	void Collide(Vector3 *p0, Vector3 *p1, u32& uSector, f32 fRadius, bool bPassThruAdjoins=false);
	void RayCastAndActivate(Vector3 *p0, Vector3 *p1, u32& uSector);
	bool Raycast(Vector3 *p0, Vector3 *p1, Vector3 *pInter);

	//
	bool IsSectorTypeVis(u32 uType) { return (m_uSectorTypeVis&uType)!=0; }

	//Get the world cell that the camera is currently occupying (if any).
	//Does not include the Terrain world cell.
	WorldCell *GetCameraWorldCell();

	//pathing.
	bool GetRandomNode(s32& nodeX, s32& nodeY, Vector3& outPos, s32& outWorldX, s32& outWorldY);
	void GetNodeVector(Vector2& vOffset, const Vector3& curPos, s32 wx, s32 wy, s32 nodeX, s32 nodeY);
	bool CheckNode(s32 nodeX, s32 nodeY);

	//Console commands
	static void CC_LockCamera(const vector<string>& args, void *pUserData);

protected:

	void ClearPathingNodes(s32 wx, s32 wy);
	void AddPathingNodes(s32 dwX, s32 dwY, u8 *pValidNodes);

	vector<WorldCell *> m_WorldCells;
	Object *m_Player;	//There's only one player (for now).
	Camera *m_pCamera;	//The current camera.
	Terrain *m_pTerrain; //World terrain.
	u32		 m_uSectorTypeVis;	//which sector type(s) is/are visible.
};

#endif //WORLD_H
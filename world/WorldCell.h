#ifndef WORLDCELL_H
#define WORLDCELL_H

#include "../CommonTypes.h"
#include "../math/Vector3.h"
#include "../render/Camera.h"
#include <vector>

using namespace std;
class Sector;
class IDriver3D;
class Camera;

class WorldCell
{
public:
	WorldCell();
	~WorldCell();

	void SetBounds(const Vector3& rvMin, const Vector3& rvMax);
	void GetBounds(Vector3& rvMin, Vector3& rvMax);

	u32 GetSectorCount();
	void AddSector(Sector *pSector);
	Sector *GetSector(u32 uIndex);
	const vector<Sector *>& GetSectors() { return m_Sectors; }
	s32 FindSector(const char *pszName);

	void SetWorldPos(s32 x, s32 y) { m_nWorldPosX = x; m_nWorldPosY = y; }
	void GetWorldPos(s32& x, s32& y) { x = m_nWorldPosX; y = m_nWorldPosY; }

	void Render(IDriver3D *pDriver, Camera *pCamera, u32 uSectorTypeVis);
	void Update(Camera *pCamera, float dt, u32 uSectorTypeVis);

	void LockCamera(Camera *pCamera, bool bLock);
	void Collide(Vector3 *p0, Vector3 *p1, u32& uSector, f32 fRadius, u32 uSectorTypeVis, bool bPassThruAdjoins, s32 worldX, s32 worldY);
	void RayCastAndActivate(Vector3 *p0, Vector3 *p1, u32& uSector, u32 uSectorTypeVis, s32 worldX, s32 worldY);
	bool Raycast(Vector3 *p0, Vector3 *p1, Vector3 *pInter, u32 uSectorTypeVis, s32 worldX, s32 worldY);

	void SetSkyTex(u32 uSkyIndex, TextureHandle hTex, u32 uWidth, u32 uHeight);
	TextureHandle GetSkyTex(u32 uSkyIndex) { return m_aSkyTex[uSkyIndex]; }
	u32 GetSkyTexCount() { return m_uSkyTexCnt; }
	void SetSkyTexCount(u32 uSkyTexCnt) { m_uSkyTexCnt = uSkyTexCnt; }

	void SetStartLoc(const Vector3& vStartLoc) { m_vStartLoc = vStartLoc; }
	void GetStartLoc(Vector3& vStartLoc) { vStartLoc = m_vStartLoc; }

	//type of cell - is it a town, graveyard, dungeon, etc.
	int  GetType() { return m_nType; }

protected:
	Vector3 m_Bounds[2];	//world space bounds [min,max]
	vector<Sector *> m_Sectors;
	s32 m_nWorldPosX, m_nWorldPosY;
	s32 m_nType;
	Camera m_LockCamera;
	bool m_bLockCamera;

	TextureHandle m_aSkyTex[16];
	u32 m_uSkyWidth, m_uSkyHeight;
	u32 m_uSkyTexCnt;
	s32 m_nCollisionIter;
	Vector3 m_vStartLoc;

	void Render_Sectors_25D(IDriver3D *pDriver, Camera *pCamera, Sector *pStart);
	void Collide_Recursive(Vector3 *p0, Vector3 *p1, u32& uSector, f32 fRadius, u32 uSectorTypeVis, bool bPassThruAdjoins, s32 worldX, s32 worldY);
};

#endif //WORLDCELL_H
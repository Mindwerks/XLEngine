#ifndef SECTOR_H
#define SECTOR_H

#include "../CommonTypes.h"
#include "../math/Vector3.h"
#include "../render/IDriver3D.h"
#include <vector>

using namespace std;

class IDriver3D;
class Camera;
struct CollisionPacket;
struct RaycastPacket;

#define INVALID_SECTOR 0xffff
#define SECTOR_TYPE_DUNGEON  1
#define SECTOR_TYPE_25D		 2
#define SECTOR_TYPE_INTERIOR 4
#define SECTOR_TYPE_EXTERIOR 8

//max 65535 sectors:
class Sector
{
public:
	Sector();
	virtual ~Sector();

	void AddObject(u32 uHandle);
	void RemoveObject(u32 uHandle);

	void AddLight(const Vector3& rvLoc) { m_Lights.push_back( xlNew LightObject(rvLoc) ); }
	void AddLight(LightObject *pLight) { m_Lights.push_back( pLight ); }
	const vector<LightObject *>& GetLightList() { return m_Lights; }

	virtual void Render(IDriver3D *pDriver, Camera *pCamera) {};
	virtual void Collide(CollisionPacket *packet, Vector3 *bounds, const Vector3& vOffset) {};
	virtual void Raycast(RaycastPacket *packet, const Vector3& vOffset) {};
	virtual void Update(float dt) {};

	u32 m_uTypeFlags;
	u32 m_uID;
	s32 m_x, m_y;
	bool m_bActive;
	Vector3 m_Bounds[2];	//24 bytes.
	vector<u32> m_Objects;	//4*cbjCnt + ~8 
	vector<LightObject *> m_Lights;
	char m_szName[64];
	u8 *m_pValidNodes;

	static u32 s_MaxSecDrawCnt;
};

#endif //SECTOR_H

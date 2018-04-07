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

	void AddObject(uint32_t uHandle);
	void RemoveObject(uint32_t uHandle);

	void AddLight(const Vector3& rvLoc) { m_Lights.push_back( xlNew LightObject(rvLoc) ); }
	void AddLight(LightObject *pLight) { m_Lights.push_back( pLight ); }
	const vector<LightObject *>& GetLightList() { return m_Lights; }

	virtual void Render(IDriver3D *pDriver, Camera *pCamera) {};
	virtual void Collide(CollisionPacket *packet, Vector3 *bounds, const Vector3& vOffset) {};
	virtual void Raycast(RaycastPacket *packet, const Vector3& vOffset) {};
	virtual void Update(float dt) {};

	uint32_t m_uTypeFlags;
	uint32_t m_uID;
	int32_t m_x, m_y;
	bool m_bActive;
	Vector3 m_Bounds[2];	//24 bytes.
	vector<uint32_t> m_Objects;	//4*cbjCnt + ~8
	vector<LightObject *> m_Lights;
	char m_szName[64];
	uint8_t *m_pValidNodes;

	static uint32_t s_MaxSecDrawCnt;
};

#endif //SECTOR_H

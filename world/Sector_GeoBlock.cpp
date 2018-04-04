#include "Sector_GeoBlock.h"
#include "ObjectManager.h"
#include "Object.h"
#include "../math/Vector4.h"
#include "../math/Math.h"
#include "../render/IDriver3D.h"
#include "../render/Camera.h"
#include "../render/RenderQue.h"
#include "WorldCell.h"

#define MAX_WORLD_UPDATE_RANGE 30

Sector_GeoBlock::Sector_GeoBlock() : Sector()
{
}

Sector_GeoBlock::~Sector_GeoBlock()
{
}

void Sector_GeoBlock::Render(IDriver3D *pDriver, Camera *pCamera)
{
	if ( !m_bActive )
		return;

	vector<u32>::iterator iObj = m_Objects.begin();
	vector<u32>::iterator eObj = m_Objects.end();
	for (; iObj != eObj; ++iObj)
	{
		Object *pObj = ObjectManager::GetObjectFromID( *iObj );
		assert(pObj);

		Vector3 vMin, vMax;
		pObj->GetWorldBounds(vMin, vMax);

		if ( pCamera->AABBInsideFrustum(vMin, vMax, pObj->GetWorldX(), pObj->GetWorldY()) != Camera::FRUSTUM_OUT )
		{
			int nLightCnt = 0;
			const LightObject **pLightList = pObj->GetLightList(nLightCnt);

			RenderQue::SetLightData( nLightCnt, pLightList );
			pObj->Render(pDriver, 1.0f, Vector3::Zero);
		}
	}
}

void Sector_GeoBlock::Collide(CollisionPacket *packet, Vector3 *bounds, const Vector3& vOffset)
{
	if ( !m_bActive )
		return;

	vector<u32>::iterator iObj = m_Objects.begin();
	vector<u32>::iterator eObj = m_Objects.end();
	for (; iObj != eObj; ++iObj)
	{
		Object *pObj = ObjectManager::GetObjectFromID( *iObj );
		assert(pObj);

		Vector3 vMin = pObj->GetWorldMin() + vOffset;
		Vector3 vMax = pObj->GetWorldMax() + vOffset;
		if ( Math::AABB_Overlap3D(vMin, vMax, bounds[0], bounds[1]) )
		{
			pObj->Collide(packet, vOffset);
		}
	}
}

void Sector_GeoBlock::Raycast(RaycastPacket *packet, const Vector3& vOffset)
{
	if ( !m_bActive )
		return;

	vector<u32>::iterator iObj = m_Objects.begin();
	vector<u32>::iterator eObj = m_Objects.end();
	for (; iObj != eObj; ++iObj)
	{
		Object *pObj = ObjectManager::GetObjectFromID( *iObj );
		assert(pObj);

		Vector3 vMin = pObj->GetWorldMin() + vOffset;
		Vector3 vMax = pObj->GetWorldMax() + vOffset;
		if ( Math::AABB_Overlap3D(vMin, vMax, packet->bounds[0], packet->bounds[1]) )
		{
			pObj->Raycast(packet, this, vOffset);
		}
	}
}

void Sector_GeoBlock::Update(float dt)
{
	if ( !m_bActive )
		return;

	vector<LightObject *>::iterator iLight = m_Lights.begin();
	vector<LightObject *>::iterator eLight = m_Lights.end();
	for (; iLight != eLight; ++iLight)
	{
		(*iLight)->Update( dt );
	}
}
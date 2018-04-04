#ifndef OBJECTDEF_H
#define OBJECTDEF_H

#include "../CommonTypes.h"
#include "../math/Vector3.h"

struct ObjectPhysicsData
{
	Vector3 m_Loc;
	Vector3 m_Dir;
	Vector3 m_Up;
	Vector3 m_Velocity;
	Vector3 m_Scale;

	u32 m_uSector;
	s32 m_worldX, m_worldY;
};

#endif //OBJECTDEF_H

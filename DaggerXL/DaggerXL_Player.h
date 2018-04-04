#ifndef DAGGERXL_PLAYER_H
#define DAGGERXL_PLAYER_H

#include "../plugin_framework/plugin.h"
#include "../world/ObjectDef.h"
#include <string>

class DaggerXL_Player
{
public:
	DaggerXL_Player(const XLEngine_Plugin_API *API);
	~DaggerXL_Player(void);

	void KeyDown(s32 key);

	void SetPos(Vector3& pos);
	void GetPos(Vector3& pos, s32& worldX, s32& worldY);

private:
	struct PlayerData
	{
		u32 m_HP;
		f32 m_fYaw;
		f32 m_fPitch;
	};

	const XLEngine_Plugin_API *m_pAPI;
	u32 m_uObjID;
	PlayerData m_PlayerData;
	ObjectPhysicsData *m_PhysicsData;
	Vector3 m_vLookDir;
	bool m_bPassthruAdjoins;
	s32  m_nFrameRotDelay;
	bool m_bOnGround;
	bool m_bAutoMove;

	void LogicSetup(u32 uObjID, u32 uParamCount, LogicParam *param);
	void ObjectSetup(u32 uObjID, u32 uParamCount, LogicParam *param);
	void Update(u32 uObjID, u32 uParamCount, LogicParam *param);
	void Message(u32 uObjID, u32 uParamCount, LogicParam *param);

	LOGIC_CB_FUNC();
};

#endif //DAGGERXL_PLAYER_H
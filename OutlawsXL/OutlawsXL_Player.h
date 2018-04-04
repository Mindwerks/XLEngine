#ifndef OUTLAWSXL_PLAYER_H
#define OUTLAWSXL_PLAYER_H

#include "../plugin_framework/plugin.h"
#include "../world/ObjectDef.h"
#include <string>

class OutlawsXL_Player
{
public:
	OutlawsXL_Player(const XLEngine_Plugin_API *API);
	~OutlawsXL_Player(void);

	void SetPassthruAdjoins(bool bPassthru) { m_bPassthruAdjoins = bPassthru; }

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
	bool m_bPassthruAdjoins;

	void LogicSetup(u32 uObjID, u32 uParamCount, LogicParam *param);
	void ObjectSetup(u32 uObjID, u32 uParamCount, LogicParam *param);
	void Update(u32 uObjID, u32 uParamCount, LogicParam *param);
	void Message(u32 uObjID, u32 uParamCount, LogicParam *param);

	LOGIC_CB_FUNC();
};

#endif //OUTLAWSXL_PLAYER_H
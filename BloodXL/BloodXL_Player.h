#ifndef BLOODXL_PLAYER_H
#define BLOODXL_PLAYER_H

#include "../plugin_framework/plugin.h"
#include "../world/ObjectDef.h"
#include <string>

class BloodXL_Player
{
public:
	BloodXL_Player(const XLEngine_Plugin_API *API);
	~BloodXL_Player(void);

	void SetPassthruAdjoins(bool bPassthru) { m_bPassthruAdjoins = bPassthru; }
	void KeyDown(int32_t key);

private:
	struct PlayerData
	{
		uint32_t m_HP;
		float m_fYaw;
		float m_fPitch;
	};

	const XLEngine_Plugin_API *m_pAPI;
	uint32_t m_uObjID;
	PlayerData m_PlayerData;
	ObjectPhysicsData *m_PhysicsData;
	bool m_bPassthruAdjoins;

	void LogicSetup(uint32_t uObjID, uint32_t uParamCount, LogicParam *param);
	void ObjectSetup(uint32_t uObjID, uint32_t uParamCount, LogicParam *param);
	void Update(uint32_t uObjID, uint32_t uParamCount, LogicParam *param);
	void Message(uint32_t uObjID, uint32_t uParamCount, LogicParam *param);

	LOGIC_CB_FUNC();
};

#endif //BLOODXL_PLAYER_H
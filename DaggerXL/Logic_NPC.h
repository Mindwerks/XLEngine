#ifndef LOGIC_NPC_H
#define LOGIC_NPC_H

#include "../plugin_framework/plugin.h"
#include "../world/ObjectDef.h"
#include <string>

class Logic_NPC
{
public:
	Logic_NPC(const XLEngine_Plugin_API *API);
	~Logic_NPC(void);

private:
	const XLEngine_Plugin_API *m_pAPI;

	void LogicSetup(u32 uObjID, u32 uParamCount, LogicParam *param);
	void ObjectSetup(u32 uObjID, u32 uParamCount, LogicParam *param);
	void Update(u32 uObjID, u32 uParamCount, LogicParam *param);
	void Message(u32 uObjID, u32 uParamCount, LogicParam *param);

	LOGIC_CB_FUNC();
};

class NPC
{
public:
	NPC(const XLEngine_Plugin_API *pAPI);
	~NPC(void);

	void Reset(const XLEngine_Plugin_API *pAPI, s32 NPC_file, float x, float y, float z, s32 worldX, s32 worldY, float dirx=0.0f, float diry=1.0f);
	void Enable(const XLEngine_Plugin_API *pAPI, bool bEnable);
	bool IsEnabled();

	void GetWorldPos(const XLEngine_Plugin_API *API, s32& x, s32& y);

public:
	struct GameData
	{
		u32 uObjID;
		TextureHandle ahTex[6];

		u32 uState;
	};
	GameData m_Data;
};

#endif //LOGIC_NPC_H
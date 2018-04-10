#ifndef LOGIC_NPC_H
#define LOGIC_NPC_H

#include "../plugin_framework/plugin.h"
#include "../world/ObjectDef.h"
#include <string>

class Logic_NPC
{
public:
    Logic_NPC(const XLEngine_Plugin_API *API);
    ~Logic_NPC();

private:
    const XLEngine_Plugin_API *m_pAPI;

    void LogicSetup(uint32_t uObjID, uint32_t uParamCount, LogicParam *param);
    void ObjectSetup(uint32_t uObjID, uint32_t uParamCount, LogicParam *param);
    void Update(uint32_t uObjID, uint32_t uParamCount, LogicParam *param);
    void Message(uint32_t uObjID, uint32_t uParamCount, LogicParam *param);

    LOGIC_CB_FUNC();
};

class NPC
{
public:
    NPC(const XLEngine_Plugin_API *pAPI);
    ~NPC();

    void Reset(const XLEngine_Plugin_API *pAPI, int32_t NPC_file, float x, float y, float z, int32_t worldX, int32_t worldY, float dirx=0.0f, float diry=1.0f);
    void Enable(const XLEngine_Plugin_API *pAPI, bool bEnable);
    bool IsEnabled();

    void GetWorldPos(const XLEngine_Plugin_API *API, int32_t& x, int32_t& y);

public:
    struct GameData
    {
        uint32_t uObjID;
        TextureHandle ahTex[6];

        uint32_t uState;
    };
    GameData m_Data;
};

#endif //LOGIC_NPC_H
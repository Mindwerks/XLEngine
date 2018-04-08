#ifndef DAGGERXL_PLAYER_H
#define DAGGERXL_PLAYER_H

#include "../plugin_framework/plugin.h"
#include "../world/ObjectDef.h"
#include <string>

class DaggerXL_Player {
public:
    DaggerXL_Player(const XLEngine_Plugin_API *API);

    ~DaggerXL_Player(void);

    void KeyDown(int32_t key);

    void SetPos(Vector3 &pos);

    void GetPos(Vector3 &pos, int32_t &worldX, int32_t &worldY);

private:
    struct PlayerData {
        uint32_t m_HP;
        float m_fYaw;
        float m_fPitch;
    };

    const XLEngine_Plugin_API *m_pAPI;
    uint32_t m_uObjID;
    PlayerData m_PlayerData;
    ObjectPhysicsData *m_PhysicsData;
    Vector3 m_vLookDir;
    bool m_bPassthruAdjoins;
    int32_t m_nFrameRotDelay;
    bool m_bOnGround;
    bool m_bAutoMove;

    void LogicSetup(uint32_t uObjID, uint32_t uParamCount, LogicParam *param);

    void ObjectSetup(uint32_t uObjID, uint32_t uParamCount, LogicParam *param);

    void Update(uint32_t uObjID, uint32_t uParamCount, LogicParam *param);

    void Message(uint32_t uObjID, uint32_t uParamCount, LogicParam *param);

    LOGIC_CB_FUNC();
};

#endif //DAGGERXL_PLAYER_H
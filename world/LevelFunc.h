#ifndef LEVELFUNC_H
#define LEVELFUNC_H

#include "../CommonTypes.h"
#include "../scriptsystem/ScriptSystem.h"
#include "../math/Vector3.h"

#include <vector>
#include <string>

class WorldCell;
class Object;

class LevelFunc
{
public:
    enum StateType_e
    {
        ST_TIME=0,
        ST_HOLD,
        ST_TERMINATE,
        ST_COMPLETE
    };

    enum
    {
        IMSG_TRIGGER=0,
        IMSG_GOTO_STOP,
        IMSG_NEXT_STOP,
        IMSG_PREV_STOP,
        IMSG_MASTER_ON,
        IMSG_MASTER_OFF,
        IMSG_CLEAR_BITS,
        IMSG_SET_BITS,
        IMSG_COMPLETE,
        IMSG_DONE,
        IMSG_WAKEUP,
        IMSG_LIGHTS,
        IMSG_NUDGE,
        IMSG_ACTIVATE,
        IMSG_SHOOT,
        IMSG_COUNT
    };

    enum
    {
        EMASK_ENEMY = 1,
        EMASK_WEAPON = 8,
        EMASK_PLAYER = 2147483648
    };

    typedef void (*LFunc_ActivateCB)(LevelFunc *, int32_t, int32_t, bool);
    typedef void (*LFunc_SetValueCB)(LevelFunc *, int32_t, float, bool);

    struct ClientObject
    {
        Object *pObj;
        Vector3 vInitialPos;
        uint32_t uFlags;
    };
    
public:
    //Setup
    LevelFunc(WorldCell *pWorldCell, int32_t nSector, int32_t nWall);
    ~LevelFunc();
    void SetActivateCB( LevelFunc::LFunc_ActivateCB pCB ) { m_ActivateCB = pCB; }
    void SetSetValueCB( LevelFunc::LFunc_SetValueCB pCB ) { m_SetValueCB = pCB; }

    //Level Load
    void AddState(float value, int32_t type=ST_TIME, int32_t delay=240);
    void AddClient(LevelFunc *pFunc)
    {
        m_Clients.push_back( pFunc );
    }
    void AddClientObj(Object *pObj, uint32_t uFlags);
    void SetSpeed(float speed) { m_fSpeed = speed; }
    void SetAccel(float accel=0.0f) { m_fAccel = accel; }
    
    //API
    //Override this function for triggers.
    void Activate(int32_t mask, int32_t items, bool bForce=false)
    {
        if ( m_ActivateCB )
             m_ActivateCB(this, mask, items, bForce);
    }
    //Override for sector effects.
    void SetValue(int32_t nSector, float value, bool bInstant=false)
    {
        if ( m_SetValueCB )
             m_SetValueCB(this, nSector, value, bInstant);
    }

    void SendMessage(int msg, int param0, int param1);
    void Update();
    void SetInitialState(int32_t state, bool bStartOnInit=false);

    WorldCell *GetWorldCell() { return m_pWorldCell; }
    uint32_t GetClientCount() { return (uint32_t)m_Clients.size(); }
    LevelFunc *GetClient(uint32_t clientID) { return m_Clients[clientID]; }

    uint32_t GetClientObjCount() { return (uint32_t)m_ClientObjects.size(); }
    ClientObject *GetClientObj(uint32_t clientID) { return m_ClientObjects[clientID]; }

    void SetDirection(const Vector3& vDir) { m_vDir = vDir; }
    const Vector3& GetDirection() { return m_vDir; }

    void SetPivot(const Vector3& vPivot) { m_vPivot = vPivot; }
    const Vector3& GetPivot() { return m_vPivot; }

protected:
    struct State
    {
        float value;
        int32_t delay;
        uint8_t  type;
    };
    std::vector<State *> m_States;
    std::vector<LevelFunc *> m_Clients;
    std::vector<ClientObject *> m_ClientObjects;
    std::vector<int32_t> m_Slaves;

    LevelFunc::LFunc_ActivateCB m_ActivateCB;
    LevelFunc::LFunc_SetValueCB m_SetValueCB;

    int32_t m_nInitialState;
    int32_t m_nCurState;
    int32_t m_nNextState;
    int32_t m_nSector;
    int32_t m_nWall;
    int32_t m_nKeyNeeded;
    int32_t m_nEventMask;
    int32_t m_nEntityMask;
    int32_t m_nEvent;
    int32_t m_nDelay;

    float m_fInterp;
    float m_fDelta;
    float m_fSpeed;
    float m_fAccel;
    float m_fVel;
    float m_fScale;

    Vector3 m_vDir;
    Vector3 m_vPivot;

    WorldCell *m_pWorldCell;

    bool m_bActive;
};

#endif //LEVELFUNC_H

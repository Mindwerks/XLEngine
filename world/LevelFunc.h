#ifndef LEVELFUNC_H
#define LEVELFUNC_H

#include "../CommonTypes.h"
#include "../scriptsystem/ScriptSystem.h"
#include "../math/Vector3.h"

#include <vector>
#include <string>
using namespace std;

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

	typedef void (*LFunc_ActivateCB)(LevelFunc *, s32, s32, bool);
	typedef void (*LFunc_SetValueCB)(LevelFunc *, s32, f32, bool);

	struct ClientObject
	{
		Object *pObj;
		Vector3 vInitialPos;
		u32 uFlags;
	};
	
public:
	//Setup
	LevelFunc(WorldCell *pWorldCell, s32 nSector, s32 nWall);
	~LevelFunc();
	void SetActivateCB( LevelFunc::LFunc_ActivateCB pCB ) { m_ActivateCB = pCB; }
	void SetSetValueCB( LevelFunc::LFunc_SetValueCB pCB ) { m_SetValueCB = pCB; }

	//Level Load
	void AddState(f32 value, s32 type=ST_TIME, s32 delay=240);
	void AddClient(LevelFunc *pFunc)
	{
		m_Clients.push_back( pFunc );
	}
	void AddClientObj(Object *pObj, u32 uFlags);
	void SetSpeed(f32 speed) { m_fSpeed = speed; }
	void SetAccel(f32 accel=0.0f) { m_fAccel = accel; }
	
	//API
	//Override this function for triggers.
	void Activate(s32 mask, s32 items, bool bForce=false)
	{
		if ( m_ActivateCB )
			 m_ActivateCB(this, mask, items, bForce);
	}
	//Override for sector effects.
	void SetValue(s32 nSector, f32 value, bool bInstant=false)
	{
		if ( m_SetValueCB )
		  	 m_SetValueCB(this, nSector, value, bInstant);
	}

	void SendMessage(int msg, int param0, int param1);
	void Update();
	void SetInitialState(s32 state, bool bStartOnInit=false);

	WorldCell *GetWorldCell() { return m_pWorldCell; }
	u32 GetClientCount() { return (u32)m_Clients.size(); }
	LevelFunc *GetClient(u32 clientID) { return m_Clients[clientID]; }

	u32 GetClientObjCount() { return (u32)m_ClientObjects.size(); }
	ClientObject *GetClientObj(u32 clientID) { return m_ClientObjects[clientID]; }

	void SetDirection(const Vector3& vDir) { m_vDir = vDir; }
	const Vector3& GetDirection() { return m_vDir; }

	void SetPivot(const Vector3& vPivot) { m_vPivot = vPivot; }
	const Vector3& GetPivot() { return m_vPivot; }

protected:
	struct State
	{
		f32 value;
		s32 delay;
		u8  type;
	};
	vector<State *> m_States;
	vector<LevelFunc *> m_Clients;
	vector<ClientObject *> m_ClientObjects;
	vector<s32> m_Slaves;

	LevelFunc::LFunc_ActivateCB m_ActivateCB;
	LevelFunc::LFunc_SetValueCB m_SetValueCB;

	s32 m_nInitialState;
	s32 m_nCurState;
	s32 m_nNextState;
	s32 m_nSector;
	s32 m_nWall;
	s32 m_nKeyNeeded;
	s32 m_nEventMask;
	s32 m_nEntityMask;
	s32 m_nEvent;
	s32 m_nDelay;

	f32 m_fInterp;
	f32 m_fDelta;
	f32 m_fSpeed;
	f32 m_fAccel;
	f32 m_fVel;
	f32 m_fScale;

	Vector3 m_vDir;
	Vector3 m_vPivot;

	WorldCell *m_pWorldCell;

	bool m_bActive;
};

#endif //LEVELFUNC_H

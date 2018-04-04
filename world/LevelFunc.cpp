#include "LevelFunc.h"
#include "LevelFuncMgr.h"
#include "Object.h"
#include <math.h>

LevelFunc::LevelFunc(WorldCell *pWorldCell, s32 nSector, s32 nWall)
{
	m_bActive = true;
	m_fInterp = 0.0f;
	m_nNextState = 0;
	m_fScale = 1.0f;
	m_fSpeed = 1.0f;
	m_fDelta = 0.0f;
	m_fAccel = 0.0f;
	m_fVel = 0.0f;
	m_nNextState = -1;
	m_nDelay = 0;

	m_nSector = nSector;
	m_nWall = nWall;
	m_pWorldCell = pWorldCell;

	m_ActivateCB = NULL;
	m_SetValueCB = NULL;
}

LevelFunc::~LevelFunc()
{
	vector<State *>::iterator iState = m_States.begin();
	vector<State *>::iterator eState = m_States.end();

	for (; iState != eState; ++iState)
	{
		xlDelete (*iState);
	}

	vector<ClientObject *>::iterator iCObj = m_ClientObjects.begin();
	vector<ClientObject *>::iterator eCObj = m_ClientObjects.end();

	for (; iCObj != eCObj; ++iCObj)
	{
		xlDelete (*iCObj);
	}
}

void LevelFunc::AddState(f32 value, s32 type, s32 delay)
{
	State *pState = xlNew State;
	m_States.push_back( pState );

	pState->value = value;
	pState->type  = type;
	pState->delay = delay;
}

void LevelFunc::AddClientObj(Object *pObj, u32 uFlags)
{
	ClientObject *pClientObj = xlNew ClientObject;
	pObj->GetLoc( pClientObj->vInitialPos );
	pClientObj->pObj = pObj;
	pClientObj->uFlags = uFlags;

	m_ClientObjects.push_back( pClientObj );
}

void LevelFunc::SendMessage(int msg, int param0, int param1)
{
	if ( msg == IMSG_ACTIVATE )
	{
		//param0 = items, param1 = mask
		if ( !m_bActive )
		{
			return;
		}

		if ( m_fInterp != 0.0f ) { return; }
		if ( param1 != 0 || param0 != 0 )
		{
			if ( !(param0&m_nKeyNeeded) && m_nKeyNeeded != 0 ) { return; }
			if ( param1 != 0 && (!(m_nEventMask&param1)) ) { return; }
		}

		if ( m_States.size() > 0 )
		{
			m_nNextState = (m_nCurState+1)%m_States.size();
			m_fDelta = m_fSpeed*m_fScale / fabsf(m_States[m_nNextState]->value - m_States[m_nCurState]->value);
		}
		else
		{
			m_fDelta = m_fSpeed*m_fScale;
		}
		m_fVel = (m_fAccel == 0.0f) ? 1.0f : 0.0f;
		m_fInterp = 0.000001f;
		m_nDelay = 0;

		LevelFuncMgr::AddToActiveList(this);
	}
}

void LevelFunc::Update()
{
	if ( !m_bActive )
	{
		LevelFuncMgr::RemoveFromActiveList(this);
		return;
	}

	if ( m_nDelay > 0 )
	{
		m_nDelay--;
		return;
	}
	else if ( m_nDelay == 0 )
	{
		m_nDelay = -1;
	}

	if ( m_States.size() > 0 )
	{
		f32 fPrevInterp = m_fInterp;
		if ( m_fSpeed == 0.0f )
		{
			m_fInterp = 1.0f;
		}
		else
		{
			m_fInterp += m_fDelta*m_fVel;
			m_fVel = min(1.0f, m_fVel+m_fAccel);
		}
		if ( m_fInterp > 1.0f ) { m_fInterp = 1.0f; }

		f32 value = (1.0f-m_fInterp)*m_States[m_nCurState]->value + m_fInterp*m_States[m_nNextState]->value;
		SetValue(m_nSector, value, (m_fSpeed==0.0f)?true:false);

		if ( m_fInterp >= 1.0f || m_fSpeed == 0.0f )
		{
			m_fInterp=0.0f;
			m_nCurState = m_nNextState;
			
			if ( m_States[m_nCurState]->type == ST_TIME )
			{
				m_nDelay = m_States[m_nCurState]->delay;
				m_nNextState = (m_nCurState+1)%m_States.size();
				m_fDelta = m_fSpeed*m_fScale / fabsf(m_States[m_nNextState]->value - m_States[m_nCurState]->value);
				m_fVel = (m_fAccel == 0.0f) ? 1.0f : 0.0f;
			}
			else
			{
				if ( m_States[m_nCurState]->type == ST_TERMINATE )
				{
					m_bActive = false;
				}

				LevelFuncMgr::RemoveFromActiveList(this);
			}
		}
	}
	else
	{
		//continuous animation.
		m_fInterp += m_fSpeed;
		SetValue(m_nSector, m_fInterp, false);
	}
}

void LevelFunc::SetInitialState(s32 state, bool bStartOnInit/*=false*/)
{
	if ( state == -1 )
		 state = 0;

	m_nInitialState = state;
	m_nCurState  = state;
	m_nNextState = state;

	if ( m_States.size() > 0 )
	{
		SetValue(m_nSector, m_States[state]->value, true);

		if ( m_States[state]->type != ST_HOLD || bStartOnInit )
		{
			m_nNextState = (m_nCurState+1)%m_States.size();
			m_fDelta = m_fSpeed*m_fScale / fabsf(m_States[m_nNextState]->value - m_States[m_nCurState]->value);
			m_fVel = (m_fAccel == 0.0f) ? 1.0f : 0.0f;
			m_fInterp = 0.000001f;
			m_nDelay = 0;
			if ( m_bActive )
			{
				LevelFuncMgr::AddToActiveList(this);
			}
		}
	}
	else if ( m_bActive )
	{
		LevelFuncMgr::AddToActiveList(this);
	}
}

#include "Logic.h"
#include "Object.h"

Logic *Logic::s_pCurLogic = NULL;

Logic::Logic(const string& sName, void *pOwner, uint32_t uType)
{
	m_sName = sName;
	m_pOwner = pOwner;
	m_uType = uType;
	m_uMsgMask = 0;

	//first initiliaze all the callbacks to NULL.
	for (uint32_t c=0; c<LCB_COUNT; c++)
	{
		if ( m_uType == LTYPE_CODE )
			m_Callbacks[c].codeCB = NULL;
		else
			m_Callbacks[c].scriptCB = -1;
	}

	if ( m_uType == LTYPE_SCRIPT )
	{
		//if this is a script logic, go through and find script callbacks...
	}
}

Logic::~Logic()
{
}

bool Logic::AddCallback(uint32_t uCallbackID, LogicFunction pCallback)
{
	m_Callbacks[ uCallbackID ].codeCB = pCallback;

	if ( uCallbackID == LCB_LOGIC_SETUP )
	{
		//store away this as the current logic, so the logic setup knows which logic to
		//reference.
		s_pCurLogic = this;
		m_Callbacks[ uCallbackID ].codeCB( m_pOwner, 0, 0, NULL );
		s_pCurLogic = NULL;
	}

	return true;
}

void Logic::InitObject(Object *parent)
{
	if ( m_uType == LTYPE_CODE && m_Callbacks[ LCB_OBJECT_SETUP ].codeCB )
	{
		m_Callbacks[ LCB_OBJECT_SETUP ].codeCB( m_pOwner, parent->GetID(), 0, NULL );
	}
	else if ( m_uType == LTYPE_SCRIPT && m_Callbacks[ LCB_OBJECT_SETUP ].scriptCB >= 0 )
	{
		ScriptSystem::ExecuteFunc(m_Callbacks[ LCB_OBJECT_SETUP ].scriptCB);
	}
}

void Logic::Update(Object *parent)
{
	if ( m_uType == LTYPE_CODE && m_Callbacks[ LCB_UPDATE ].codeCB )
	{
		m_Callbacks[ LCB_UPDATE ].codeCB( m_pOwner, parent->GetID(), 0, NULL );
	}
	else if ( m_uType == LTYPE_SCRIPT && m_Callbacks[ LCB_UPDATE ].scriptCB >= 0 )
	{
		ScriptSystem::ExecuteFunc(m_Callbacks[ LCB_UPDATE ].scriptCB);
	}
}

void Logic::SendMessage(Object *parent, uint32_t uMsgID, f32 fValue)
{
	if ( uMsgID & m_uMsgMask )
	{
		if ( m_uType == LTYPE_CODE && m_Callbacks[ LCB_MESSAGE ].codeCB )
		{
			m_ParamList[0].uParam = uMsgID;
			m_ParamList[1].fParam = fValue;
			m_Callbacks[ LCB_MESSAGE ].codeCB( m_pOwner, parent->GetID(), 2, m_ParamList );
		}
		else if ( m_uType == LTYPE_SCRIPT && m_Callbacks[ LCB_MESSAGE ].scriptCB >= 0 )
		{
			ScriptArgument paramList[2];
			paramList[0].uType  = SC_ARG_uint32_t;
			paramList[0].arguint32_t = uMsgID;
			paramList[1].uType  = SC_ARG_F32;
			paramList[1].argF32 = fValue;

			ScriptSystem::ExecuteFunc(m_Callbacks[ LCB_MESSAGE ].scriptCB, 2, paramList);
		}
	}
}

void Logic::SetMessageMask_CurLogic(uint32_t uMask)
{
	s_pCurLogic->m_uMsgMask = uMask;
}

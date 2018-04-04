#ifndef LOGIC_H
#define LOGIC_H

#include "../CommonTypes.h"
#include "../scriptsystem/ScriptSystem.h"
#include "LogicDef.h"

#include <string>
using namespace std;

class Object;

class Logic
{
public:
	Logic(const string& sName, void *pOwner, u32 uType=LTYPE_SCRIPT);
	virtual ~Logic();

	//used for code based logics.
	//script based logics do this automatically.
	bool AddCallback(u32 uCallbackID, LogicFunction pCallback);

	void Update(Object *parent);
	void InitObject(Object *parent);
	void SendMessage(Object *parent, u32 uMsgID, f32 fValue);

	static void SetMessageMask_CurLogic(u32 uMask);

protected:
	u32 m_uType;
	u32 m_uMsgMask;
	string m_sName;
	LogicParam m_ParamList[MAX_LOGIC_PARAM];
	void *m_pOwner;

	union LogicCallback
	{
		SHANDLE scriptCB;
		LogicFunction codeCB;
	};
	LogicCallback m_Callbacks[ LCB_COUNT ];

	static Logic *s_pCurLogic;
};

#endif //LOGIC_H

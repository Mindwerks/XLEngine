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
	Logic(const string& sName, void *pOwner, uint32_t uType=LTYPE_SCRIPT);
	virtual ~Logic();

	//used for code based logics.
	//script based logics do this automatically.
	bool AddCallback(uint32_t uCallbackID, LogicFunction pCallback);

	void Update(Object *parent);
	void InitObject(Object *parent);
	void SendMessage(Object *parent, uint32_t uMsgID, f32 fValue);

	static void SetMessageMask_CurLogic(uint32_t uMask);

protected:
	uint32_t m_uType;
	uint32_t m_uMsgMask;
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

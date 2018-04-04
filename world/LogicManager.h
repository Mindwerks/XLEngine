#ifndef LOGICMANAGER_H
#define LOGICMANAGER_H

#include "../CommonTypes.h"
#include "Logic.h"
#include <map>
#include <string>

using namespace std;

class LogicManager
{
public:
	static bool Init();
	static void Destroy();

	static Logic *GetLogic(const string& sName);
	static Logic *CreateLogicFromCode(const string& sName, void *pOwner, LogicFunction *pFunc);
	static void CreateLogicFromCode_API(const char *pszName, void *pOwner, LogicFunction *pFunc);

private:
	static map<string, Logic *> m_Logics;
};

#endif //LOGICMANAGER_H

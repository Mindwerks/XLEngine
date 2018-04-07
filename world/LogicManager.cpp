#include "LogicManager.h"

map<string, Logic *> LogicManager::m_Logics;

bool LogicManager::Init()
{
	return true;
}

void LogicManager::Destroy()
{
	map<string, Logic *>::iterator iLogic = m_Logics.begin();
	map<string, Logic *>::iterator eLogic = m_Logics.end();

	for (; iLogic != eLogic; ++iLogic)
	{
		xlDelete iLogic->second;
	}

	m_Logics.clear();
}

Logic *LogicManager::GetLogic(const string& sName)
{
	map<string, Logic *>::iterator iLogic = m_Logics.find(sName);
	if ( iLogic != m_Logics.end() )
	{
		return iLogic->second;
	}
	
	return NULL;
}

Logic *LogicManager::CreateLogicFromCode(const string& sName, void *pOwner, LogicFunction *pFunc)
{
	Logic *pLogic = xlNew Logic(sName, pOwner, LTYPE_CODE);
	if ( pLogic )
	{
		for (uint32_t cb=0; cb<LCB_COUNT; cb++)
		{
			pLogic->AddCallback(cb, pFunc[cb]);
		}
		m_Logics[sName] = pLogic;
	}
	return pLogic;
}

void LogicManager::CreateLogicFromCode_API(const char *pszName, void *pOwner, LogicFunction *pFunc)
{
	CreateLogicFromCode(pszName, pOwner, pFunc);
}
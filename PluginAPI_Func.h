#ifndef PLUGIN_API_FUNC_H
#define PLUGIN_API_FUNC_H

//Input, implemented in Input.cpp
extern int32_t Input_IsKeyDown(int32_t key);

extern float Input_GetMousePosX(void);

extern float Input_GetMousePosY(void);

extern int32_t Input_AddKeyDownCB(Input_KeyDownCB pCB, int32_t nFlags);

extern int32_t Input_AddCharDownCB(Input_KeyDownCB pCB);

//Console, implemented in XL_Console.cpp
extern void
Console_RegisterCommand(const char *pszItemName, void *ptr, uint32_t type, const char *pszItemHelp, void *pUserData);

//Scripting, implemented in ScriptSystem.cpp
extern int32_t ScriptSystem_RegisterScriptFunc(const char *decl, const asSFuncPtr &pFunc);

#endif //PLUGIN_API_FUNC_H
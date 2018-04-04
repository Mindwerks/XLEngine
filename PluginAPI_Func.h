#ifndef PLUGIN_API_FUNC_H
#define PLUGIN_API_FUNC_H

//Input, implemented in Input.cpp
extern s32 Input_IsKeyDown(s32 key);
extern f32 Input_GetMousePosX(void);
extern f32 Input_GetMousePosY(void);
extern s32 Input_AddKeyDownCB(Input_KeyDownCB pCB, s32 nFlags);
extern s32 Input_AddCharDownCB(Input_KeyDownCB pCB);

//Console, implemented in XL_Console.cpp
extern void Console_RegisterCommand(const char *pszItemName, void *ptr, u32 type, const char *pszItemHelp, void *pUserData);

//Scripting, implemented in ScriptSystem.cpp
extern s32 ScriptSystem_RegisterScriptFunc(const char *decl, const asSFuncPtr& pFunc);

#endif //PLUGIN_API_FUNC_H
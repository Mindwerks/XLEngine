#ifndef XLENGINE_PLUGIN_API_H
#define XLENGINE_PLUGIN_API_H

#include "../CommonTypes.h"
#include "../os/Input.h"
#include "../scriptsystem/ScriptSystem.h"
#include "../world/ObjectDef.h"
#include "../world/LogicDef.h"

#define UPDATE_STAGE_FIXED    0
#define UPDATE_STAGE_VARIABLE 1

#define RENDER_STAGE_PREWORLD  0
#define RENDER_STAGE_POSTWORLD 1

#ifdef __cplusplus
extern "C" {
#endif

struct XLEngine_Plugin_API;
typedef void (*XAPI_Game_Update)(s32, f32, XLEngine_Plugin_API*, void*);
typedef void (*XAPI_Game_Render)(s32, f32, XLEngine_Plugin_API*, void*);
typedef void (*XAPI_World_Update)(s32, s32, XLEngine_Plugin_API*, void*);
typedef void (*XAPI_Engine_SetCameraData)(f32 *, f32 *, f32, f32, u32);
typedef XL_BOOL (*XAPI_AllowPlayerControls)(void);

typedef const char* (*XAPI_Startup_GetStartMap)(void);
typedef s32 (*XAPI_IsServer)(void);

typedef s32 (*XAPI_Game_SetUpdateCB)(XAPI_Game_Update, void*);
typedef s32 (*XAPI_Game_SetRenderCB)(XAPI_Game_Render, void*);
typedef void (*XAPI_Game_SetGameInfo)(const char*, int, int);
typedef void (*XAPI_Game_LoadWorldMap)(void);
typedef s32 (*XAPI_World_SetUpdateCB)(XAPI_World_Update, void*);
typedef s32 (*XAPI_Input_KeyDown)(s32);
typedef f32 (*XAPI_Input_MousePos)(void);
typedef f32 (*XAPI_Input_GetMouseDelta)(void);
typedef s32 (*XAPI_Input_GameKeyDownCB)(Input_KeyDownCB, s32);
typedef s32 (*XAPI_Input_GameCharDownCB)(Input_KeyDownCB);
typedef void (*XAPI_Input_EnableMouseLocking)(XL_BOOL);

typedef void (*XAPI_Console_RegisterCmd)(const char*, void *, u32, const char *, void *);
typedef void (*XAPI_Console_PrintF)(const char *, ...);
typedef void (*XAPI_SetConsoleColor)(f32, f32, f32, f32);

typedef void (*XAPI_Start_Script)(const char *);
typedef s32 (*XAPI_ScriptSystem_RegisterFunc)(const char *, const asSFuncPtr&);

typedef s32 (*XAPI_GameFile_Open)(u32, const char *, const char *);
typedef u32 (*XAPI_GameFile_Length)(void);
typedef void (*XAPI_GameFile_Read)(void *, u32);
typedef void (*XAPI_GameFile_Close)(void);

typedef s32 (*XAPI_SysFile_Open)(const char *);
typedef u32 (*XAPI_SysFile_Length)(void);
typedef void (*XAPI_SysFile_Read)(void *, u32, u32);
typedef void (*XAPI_SysFile_Close)(void);

typedef void (*XAPI_Parser_SetData)(char *, u32, u32);
typedef s32 (*XAPI_Parser_SearchKeyword_S32)(const char *, s32&);
typedef u32 (*XAPI_Parser_GetFilePtr)(void);

typedef void (*XAPI_MoviePlayer_SetPlayer)(u32);
typedef void (*XAPI_MoviePlayer_SetArchives)(u32, const char *, const char *);
typedef s32 (*XAPI_MoviePlayer_Start)(const char *, u32, s32);
typedef s32 (*XAPI_MoviePlayer_Update)(void);
typedef void (*XAPI_MoviePlayer_Stop)(void);
typedef void (*XAPI_MoviePlayer_Render)(f32);

typedef void (*XAPI_SetGamePalette)(u8, u8 *, u32, u32);
typedef void (*XAPI_SetColormap)(u8, u8 *, s32);
typedef void (*XAPI_World_CreateTerrain)(s32, s32);
typedef void (*XAPI_World_UnloadAllCells)(void);
typedef void (*XAPI_World_LoadCell)(u32, u32, const char *, const char *, s32, s32);
typedef void (*XAPI_World_Collide)(void *, void *, u32& , f32, s32);
typedef void (*XAPI_World_Activate)(void *, void *, u32&);
typedef XL_BOOL (*XAPI_World_Raycast)(void *, void *, void *);
typedef XL_BOOL (*XAPI_World_IsPointInWater)(void *);
typedef void (*XAPI_World_GetCellWorldPos)(char *, int *, int *);
typedef void (*XAPI_World_GetCellStartPos)(char *, float *, float *, float *);

typedef u32 ( *XAPI_Object_Create)(const char *, s32);
typedef void (*XAPI_Object_Free)(u32);
typedef void (*XAPI_Object_AddLogic)(u32, const char *);
typedef ObjectPhysicsData* (*XAPI_Object_GetPhysicsData)(u32);
typedef void* (*XAPI_Object_GetGameData)(u32);
typedef void (*XAPI_Object_SetGameData)(u32, void *);
typedef void (*XAPI_Object_ReserveObjects)(u32);
typedef void (*XAPI_Object_FreeAllObjects)(void);
typedef void (*XAPI_SetObjectAngles)(u32, float, float, float);
typedef void (*XAPI_ObjectEnableCollision)(u32, s32);
typedef void (*XAPI_Object_SendMessage)(u32, u32, f32);
typedef void (*XAPI_Object_SetRenderComponent)(u32, const char *);
typedef void (*XAPI_Object_SetRenderTexture)(u32, TextureHandle);
typedef void (*XAPI_Object_SetRenderFlip)(u32, XL_BOOL, XL_BOOL);
typedef void (*XAPI_Object_SetWorldBounds)(u32, float, float, float, float, float, float);
typedef void (*XAPI_Object_GetCameraVector)(u32, float&, float&, float&);
typedef void (*XAPI_Object_SetActive)(u32, XL_BOOL);

typedef void (*XAPI_Logic_CreateFromCode)(const char *, void *, LogicFunction *);
typedef void (*XAPI_Logic_SetMessageMask)(u32);

typedef TextureHandle (*XAPI_Texture_LoadTexList)(u32, u32, u32, const char *, const char *, s32, XL_BOOL);
typedef void (*XAPI_Texture_GetSize)(s32&, s32&, u32&, u32&, float&, float&);
typedef void* (*XAPI_Texture_GetExtraData)();

typedef XL_BOOL (*XAPI_Pathing_GetRandomNode)(s32&, s32&, float&, float&, float&, s32&, s32&);
typedef XL_BOOL (*XAPI_Pathing_CheckNode)(s32, s32);
typedef void (*XAPI_Pathing_GetNodeVector)(float&, float&, float, float, s32, s32, s32, s32);


//Cross platform "C" interface.
struct XLEngine_Plugin_API
{
	//Game callbacks.
	XAPI_Game_SetUpdateCB  SetGameUpdateCallback;	//s32 SetGameUpdateCallback( Game_Update pCB, void *pUserData );
	XAPI_Game_SetUpdateCB  SetGameRenderCallback;	//s32 SetGameRenderCallback( Game_Render pCB, void *pUserData );
	XAPI_Game_SetGameInfo  SetGameData;				//void SetGameData(const char *pszName, int versionMajor, int versionMinor);
	XAPI_Game_LoadWorldMap LoadWorldMap;			//void LoadWorldMap();
	XAPI_World_SetUpdateCB World_SetUpdateCallback;	//void World_SetUpdateCallback(XAPI_World_Update *pCB, void *pUserData);

	//Engine
	XAPI_Engine_SetCameraData Engine_SetCameraData; //void Engine_SetCameraData)(f32 *pos, f32 *dir);
	XAPI_AllowPlayerControls  Engine_AllowPlayerControls; //XL_BOOL Engine_AllowPlayerControls();

	//Get Startup options.
	XAPI_Startup_GetStartMap Startup_GetStartMap;	//const char *Startup_GetStartMap();
	XAPI_IsServer IsServer;							//XL_BOOL IsServer();

	//Input
	XAPI_Input_KeyDown    IsKeyDown;				//s32 IsKeyDown(s32 key);
	XAPI_Input_MousePos   GetMouseX;				//f32 GetMouseX(void);
	XAPI_Input_MousePos   GetMouseY;				//f32 GetMouesY(void);
	XAPI_Input_GetMouseDelta GetMouseDx;			//f32 GetMouseDx(void)
	XAPI_Input_GetMouseDelta GetMouseDy;			//f32 GetMouseDy(void)
	XAPI_Input_GameKeyDownCB  AddKeyDownCallback;	//s32 AddKeyDownCallback(  Input_KeyDownCB pCB, s32 nFlags );
	XAPI_Input_GameCharDownCB AddCharDownCallback;	//s32 AddCharDownCallback( Input_KeyDownCB pCB );
	XAPI_Input_EnableMouseLocking EnableMouseLocking; //void EnableMouseLocking( XL_BOOL bEnable );

	//Console
	XAPI_Console_RegisterCmd RegisterConsoleCmd;		//void RegisterConsoleCmd(const char *pszItemName, void *ptr, u32 type, const char *pszItemHelp);
	XAPI_Console_PrintF PrintToConsole;				    //void PrintToConsole(const char *pszString, ...);
	XAPI_SetConsoleColor SetConsoleColor;				//void SetConsoleColor(f32 fRed, f32 fGreen, f32 fBlue, f32 fRed);

	//Scripts
	XAPI_Start_Script Start_UI_Script;				    //void Start_UI_Script(const char *pszFile);
	XAPI_ScriptSystem_RegisterFunc RegisterScriptFunc;  //s32 RegisterScriptFunc(const char *decl, const asSFuncPtr& pFunc);

	//Gamefile manipulation
	XAPI_GameFile_Open   GameFile_Open;					//s32 GameFile_Open(u32 archiveType, const char *pszArchive, const char *pszFileName);
	XAPI_GameFile_Length GameFile_GetLength;			//u32 GameFile_Length();
	XAPI_GameFile_Read   GameFile_Read;					//void GameFile_Read(void *pData, u32 length);
	XAPI_GameFile_Close  GameFile_Close;				//void GameFile_Close();

	//System file manipulation. This allows the plugin to load individual files.
	XAPI_SysFile_Open   SysFile_Open;					//s32 SysFile_Open(const char *pszFileName);
	XAPI_SysFile_Length SysFile_GetLength;				//u32 SysFile_Length();
	XAPI_SysFile_Read   SysFile_Read;					//void SysFile_Read(void *pData, u32 start, u32 length);
	XAPI_SysFile_Close  SysFile_Close;					//void SysFile_Close();

	//Parser
	XAPI_Parser_SetData		      Parser_SetData;			//void Parser_SetData(char *pMemory, u32 length, u32 filePtr);
	XAPI_Parser_SearchKeyword_S32 Parser_SearchKeyword_S32; //s32 Parser_SearchKeyword_S32(const char *pszKeyword, s32& value);
	XAPI_Parser_GetFilePtr	      Parser_GetFilePtr;		//u32 Parser_GetFilePtr();

	//(Movie) Cutscene Player
	XAPI_MoviePlayer_SetPlayer	  MoviePlayer_SetPlayer;		//
	XAPI_MoviePlayer_SetArchives  MoviePlayer_SetArchives;		//
	XAPI_MoviePlayer_Start		  MoviePlayer_Start;			//
	XAPI_MoviePlayer_Update		  MoviePlayer_Update;			//
	XAPI_MoviePlayer_Stop		  MoviePlayer_Stop;				//
	XAPI_MoviePlayer_Render		  MoviePlayer_Render;			//

	//Palette
	XAPI_SetGamePalette			  SetGamePalette;				//void SetGamePalette(u8 *pData, u32 uSize, u32 uTransparentIndex);
	XAPI_SetColormap			  SetColormap;					//void SetColormap(u8 index, u8 *pData, s32 numLightLevels);

	//World
	XAPI_World_CreateTerrain	  World_CreateTerrain;			//void World_CreateTerrain(int width, int height)
	XAPI_World_UnloadAllCells	  World_UnloadAllCells;			//void World_UnloadAllCells(void);
	XAPI_World_LoadCell			  World_LoadCell;				//void World_LoadCell(u32 cellType, u32 archiveType, const char *pszArchive, const char *pszFile, s32 worldX, s32 worldY);
	XAPI_World_Collide			  World_Collide;				//void World_Collision(Vector3 *p0, Vector3 *p1, u32& uSector, f32 fRadius, s32 nPassThruAdjoins=0);
	XAPI_World_Activate			  World_Activate;				//void World_Activate(Vector3 *p0, Vector3 *p1, u32& uSector);
	XAPI_World_Raycast			  World_Raycast;				//XL_BOOL World_Raycast(void *p0, void *p1, void *pInter);
	XAPI_World_IsPointInWater	  World_IsPointInWater;			//XL_BOOL World_IsPointInWater(void *p0);
	XAPI_World_GetCellWorldPos    World_GetCellWorldPos;		//void World_GetCellWorldPos(char *pszName, int *x, int *y)
	XAPI_World_GetCellStartPos    World_GetCellStartPos;	    //void World_GetCellStartPos(char *pszName, float *x, float *y, float *z)

	//Object
	XAPI_Object_Create			  Object_Create;				//u32 Object_Create(const char *pszName, s32 nSector);
	XAPI_Object_Free			  Object_Free;					//void Object_Free(u32 uID);
	XAPI_Object_AddLogic		  Object_AddLogic;				//void Object_AddLogic(u32 uID, const char *pszLogic);
	XAPI_Object_GetPhysicsData	  Object_GetPhysicsData;		//ObjPhysicsData *Object_GetPhysicsData(u32 uID);
	XAPI_Object_GetGameData		  Object_GetGameData;			//void *Object_GetGameData(u32 uID);
	XAPI_Object_SetGameData		  Object_SetGameData;			//void Object_SetGameData(u32 uID, void *pData);
	XAPI_Object_ReserveObjects	  Object_ReserveObjects;		//void Object_ReserveObjects(u32 uReserveCount);
	XAPI_Object_FreeAllObjects	  Object_FreeAllObjects;		//void Object_FreeAllObjects();
	XAPI_SetObjectAngles		  Object_SetAngles;				//void Object_SetAngles(u32 uID, float x, float y, float z);
	XAPI_SetObjectAngles		  Object_SetPos;				//void Object_SetPos(u32 uID, float x, float y, float z);
	XAPI_ObjectEnableCollision    Object_EnableCollision;		//void Object_EnableCollision(u32 uID, s32 enable);
	XAPI_Object_SendMessage		  Object_SendMessage;			//void Object_SendMessage(u32 uID, u32 msg, f32 value);
	XAPI_Object_SetRenderComponent Object_SetRenderComponent;	//void Object_SetRenderComponent(u32 uID, const char *pszComponentName);
	XAPI_Object_SetRenderTexture  Object_SetRenderTexture;		//void Object_SetRenderTexture(u32 uID, TextureHandle hTex);
	XAPI_Object_SetRenderFlip	  Object_SetRenderFlip;			//void Object_SetRenderFlip(u32 uID, XL_BOOL bFlipX, XL_BOOL bFlipY);
	XAPI_Object_SetWorldBounds	  Object_SetWorldBounds;		//void Object_SetWorldBounds( u32 uID, vMin.x, vMin.y, vMin.z, vMax.x, vMax.y, vMax.z );
	XAPI_Object_GetCameraVector   Object_GetCameraVector;		//void Object_GetCameraVector(u32 uObjID, float& x, float& y, float& z)
	XAPI_Object_SetActive		  Object_SetActive;				//void Object_SetActive(u32 uObjID, XL_BOOL bActive);

	//Logics
	XAPI_Logic_CreateFromCode	  Logic_CreateFromCode;			//void Logic_CreateFromCode(const char *pszName, void *pOwner, LogicFunction functions[]);
	XAPI_Logic_SetMessageMask	  Logic_SetMessageMask;			//void Logic_SetMessageMask(u32 uMask);

	//Texture Handling.
	XAPI_Texture_LoadTexList	  Texture_LoadTexList;			//void Texture_LoadTexList(u32 uTextureType, u32 uPalIndex, u32 uArchiveType, const char *pszArchive, const char *pszFile, s32 nRecord, bool bGenMips);
	XAPI_Texture_GetSize		  Texture_GetSize;				//void Texture_GetSize(s32& ox, s32& oy, u32& w, u32& h, float& fw, float& fh);
	XAPI_Texture_GetExtraData	  Texture_GetExtraData;			//void *Texture_GetExtraData();

	//Pathing
	XAPI_Pathing_GetRandomNode	  Pathing_GetRandomNode;		//XL_BOOL Pathing_GetRandomNode(s32& nodeX, s32& nodeY, float& x, float& y, float& z, s32& sx, s32& sy)
	XAPI_Pathing_CheckNode		  Pathing_CheckNode;			//XL_BOOL Pathing_CheckNode(s32 nodeX, s32 nodeY)
	XAPI_Pathing_GetNodeVector	  Pathing_GetNodeVector;		//void Pathing_GetNodeVector(float& vx, float& vy, float cx, float cy, s32 wx, s32 wy, s32 nodeX, s32 nodeY)
};

//Helper MACRO to map from "C" Logic callbacks to "C++" member functions.
#define LOGIC_CB_MAP(c) \
void c::LogicSetupCB(void *pOwner, u32 uObjID, u32 uParamCount, LogicParam *param) { ((c *)pOwner)->LogicSetup(uObjID, uParamCount, param); }	\
void c::ObjectSetupCB(void *pOwner, u32 uObjID, u32 uParamCount, LogicParam *param) { ((c *)pOwner)->ObjectSetup(uObjID, uParamCount, param); }	\
void c::UpdateCB(void *pOwner, u32 uObjID, u32 uParamCount, LogicParam *param) { ((c *)pOwner)->Update(uObjID, uParamCount, param); }	\
void c::MessageCB(void *pOwner, u32 uObjID, u32 uParamCount, LogicParam *param) { ((c *)pOwner)->Message(uObjID, uParamCount, param); }

//Define the static Callback functions in the class definition.
#define LOGIC_CB_FUNC() \
static void LogicSetupCB(void *pOwner, u32 uObjID, u32 uParamCount, LogicParam *param); \
static void ObjectSetupCB(void *pOwner, u32 uObjID, u32 uParamCount, LogicParam *param); \
static void UpdateCB(void *pOwner, u32 uObjID, u32 uParamCount, LogicParam *param); \
static void MessageCB(void *pOwner, u32 uObjID, u32 uParamCount, LogicParam *param); 

#define LOGIC_FUNC_LIST(f) \
LogicFunction f[]={ LogicSetupCB,	ObjectSetupCB, UpdateCB, MessageCB }

#ifdef  __cplusplus
}
#endif

#endif // XLENGINE_PLUGIN_API_H

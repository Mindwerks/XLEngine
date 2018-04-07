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
typedef void (*XAPI_Game_Update)(int32_t, f32, XLEngine_Plugin_API*, void*);
typedef void (*XAPI_Game_Render)(int32_t, f32, XLEngine_Plugin_API*, void*);
typedef void (*XAPI_World_Update)(int32_t, int32_t, XLEngine_Plugin_API*, void*);
typedef void (*XAPI_Engine_SetCameraData)(f32 *, f32 *, f32, f32, uint32_t);
typedef XL_BOOL (*XAPI_AllowPlayerControls)(void);

typedef const char* (*XAPI_Startup_GetStartMap)(void);
typedef int32_t (*XAPI_IsServer)(void);

typedef int32_t (*XAPI_Game_SetUpdateCB)(XAPI_Game_Update, void*);
typedef int32_t (*XAPI_Game_SetRenderCB)(XAPI_Game_Render, void*);
typedef void (*XAPI_Game_SetGameInfo)(const char*, int, int);
typedef void (*XAPI_Game_LoadWorldMap)(void);
typedef int32_t (*XAPI_World_SetUpdateCB)(XAPI_World_Update, void*);
typedef int32_t (*XAPI_Input_KeyDown)(int32_t);
typedef f32 (*XAPI_Input_MousePos)(void);
typedef f32 (*XAPI_Input_GetMouseDelta)(void);
typedef int32_t (*XAPI_Input_GameKeyDownCB)(Input_KeyDownCB, int32_t);
typedef int32_t (*XAPI_Input_GameCharDownCB)(Input_KeyDownCB);
typedef void (*XAPI_Input_EnableMouseLocking)(XL_BOOL);

typedef void (*XAPI_Console_RegisterCmd)(const char*, void *, uint32_t, const char *, void *);
typedef void (*XAPI_Console_PrintF)(const char *, ...);
typedef void (*XAPI_SetConsoleColor)(f32, f32, f32, f32);

typedef void (*XAPI_Start_Script)(const char *);
typedef int32_t (*XAPI_ScriptSystem_RegisterFunc)(const char *, const asSFuncPtr&);

typedef int32_t (*XAPI_GameFile_Open)(uint32_t, const char *, const char *);
typedef uint32_t (*XAPI_GameFile_Length)(void);
typedef void (*XAPI_GameFile_Read)(void *, uint32_t);
typedef void (*XAPI_GameFile_Close)(void);

typedef int32_t (*XAPI_SysFile_Open)(const char *);
typedef uint32_t (*XAPI_SysFile_Length)(void);
typedef void (*XAPI_SysFile_Read)(void *, uint32_t, uint32_t);
typedef void (*XAPI_SysFile_Close)(void);

typedef void (*XAPI_Parser_SetData)(char *, uint32_t, uint32_t);
typedef int32_t (*XAPI_Parser_SearchKeyword_int32_t)(const char *, int32_t&);
typedef uint32_t (*XAPI_Parser_GetFilePtr)(void);

typedef void (*XAPI_MoviePlayer_SetPlayer)(uint32_t);
typedef void (*XAPI_MoviePlayer_SetArchives)(uint32_t, const char *, const char *);
typedef int32_t (*XAPI_MoviePlayer_Start)(const char *, uint32_t, int32_t);
typedef int32_t (*XAPI_MoviePlayer_Update)(void);
typedef void (*XAPI_MoviePlayer_Stop)(void);
typedef void (*XAPI_MoviePlayer_Render)(f32);

typedef void (*XAPI_SetGamePalette)(uint8_t, uint8_t *, uint32_t, uint32_t);
typedef void (*XAPI_SetColormap)(uint8_t, uint8_t *, int32_t);
typedef void (*XAPI_World_CreateTerrain)(int32_t, int32_t);
typedef void (*XAPI_World_UnloadAllCells)(void);
typedef void (*XAPI_World_LoadCell)(uint32_t, uint32_t, const char *, const char *, int32_t, int32_t);
typedef void (*XAPI_World_Collide)(void *, void *, uint32_t& , f32, int32_t);
typedef void (*XAPI_World_Activate)(void *, void *, uint32_t&);
typedef XL_BOOL (*XAPI_World_Raycast)(void *, void *, void *);
typedef XL_BOOL (*XAPI_World_IsPointInWater)(void *);
typedef void (*XAPI_World_GetCellWorldPos)(char *, int *, int *);
typedef void (*XAPI_World_GetCellStartPos)(char *, float *, float *, float *);

typedef uint32_t ( *XAPI_Object_Create)(const char *, int32_t);
typedef void (*XAPI_Object_Free)(uint32_t);
typedef void (*XAPI_Object_AddLogic)(uint32_t, const char *);
typedef ObjectPhysicsData* (*XAPI_Object_GetPhysicsData)(uint32_t);
typedef void* (*XAPI_Object_GetGameData)(uint32_t);
typedef void (*XAPI_Object_SetGameData)(uint32_t, void *);
typedef void (*XAPI_Object_ReserveObjects)(uint32_t);
typedef void (*XAPI_Object_FreeAllObjects)(void);
typedef void (*XAPI_SetObjectAngles)(uint32_t, float, float, float);
typedef void (*XAPI_ObjectEnableCollision)(uint32_t, int32_t);
typedef void (*XAPI_Object_SendMessage)(uint32_t, uint32_t, f32);
typedef void (*XAPI_Object_SetRenderComponent)(uint32_t, const char *);
typedef void (*XAPI_Object_SetRenderTexture)(uint32_t, TextureHandle);
typedef void (*XAPI_Object_SetRenderFlip)(uint32_t, XL_BOOL, XL_BOOL);
typedef void (*XAPI_Object_SetWorldBounds)(uint32_t, float, float, float, float, float, float);
typedef void (*XAPI_Object_GetCameraVector)(uint32_t, float&, float&, float&);
typedef void (*XAPI_Object_SetActive)(uint32_t, XL_BOOL);

typedef void (*XAPI_Logic_CreateFromCode)(const char *, void *, LogicFunction *);
typedef void (*XAPI_Logic_SetMessageMask)(uint32_t);

typedef TextureHandle (*XAPI_Texture_LoadTexList)(uint32_t, uint32_t, uint32_t, const char *, const char *, int32_t, XL_BOOL);
typedef void (*XAPI_Texture_GetSize)(int32_t&, int32_t&, uint32_t&, uint32_t&, float&, float&);
typedef void* (*XAPI_Texture_GetExtraData)();

typedef XL_BOOL (*XAPI_Pathing_GetRandomNode)(int32_t&, int32_t&, float&, float&, float&, int32_t&, int32_t&);
typedef XL_BOOL (*XAPI_Pathing_CheckNode)(int32_t, int32_t);
typedef void (*XAPI_Pathing_GetNodeVector)(float&, float&, float, float, int32_t, int32_t, int32_t, int32_t);


//Cross platform "C" interface.
struct XLEngine_Plugin_API
{
	//Game callbacks.
	XAPI_Game_SetUpdateCB  SetGameUpdateCallback;	//int32_t SetGameUpdateCallback( Game_Update pCB, void *pUserData );
	XAPI_Game_SetUpdateCB  SetGameRenderCallback;	//int32_t SetGameRenderCallback( Game_Render pCB, void *pUserData );
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
	XAPI_Input_KeyDown    IsKeyDown;				//int32_t IsKeyDown(int32_t key);
	XAPI_Input_MousePos   GetMouseX;				//f32 GetMouseX(void);
	XAPI_Input_MousePos   GetMouseY;				//f32 GetMouesY(void);
	XAPI_Input_GetMouseDelta GetMouseDx;			//f32 GetMouseDx(void)
	XAPI_Input_GetMouseDelta GetMouseDy;			//f32 GetMouseDy(void)
	XAPI_Input_GameKeyDownCB  AddKeyDownCallback;	//int32_t AddKeyDownCallback(  Input_KeyDownCB pCB, int32_t nFlags );
	XAPI_Input_GameCharDownCB AddCharDownCallback;	//int32_t AddCharDownCallback( Input_KeyDownCB pCB );
	XAPI_Input_EnableMouseLocking EnableMouseLocking; //void EnableMouseLocking( XL_BOOL bEnable );

	//Console
	XAPI_Console_RegisterCmd RegisterConsoleCmd;		//void RegisterConsoleCmd(const char *pszItemName, void *ptr, uint32_t type, const char *pszItemHelp);
	XAPI_Console_PrintF PrintToConsole;				    //void PrintToConsole(const char *pszString, ...);
	XAPI_SetConsoleColor SetConsoleColor;				//void SetConsoleColor(f32 fRed, f32 fGreen, f32 fBlue, f32 fRed);

	//Scripts
	XAPI_Start_Script Start_UI_Script;				    //void Start_UI_Script(const char *pszFile);
	XAPI_ScriptSystem_RegisterFunc RegisterScriptFunc;  //int32_t RegisterScriptFunc(const char *decl, const asSFuncPtr& pFunc);

	//Gamefile manipulation
	XAPI_GameFile_Open   GameFile_Open;					//int32_t GameFile_Open(uint32_t archiveType, const char *pszArchive, const char *pszFileName);
	XAPI_GameFile_Length GameFile_GetLength;			//uint32_t GameFile_Length();
	XAPI_GameFile_Read   GameFile_Read;					//void GameFile_Read(void *pData, uint32_t length);
	XAPI_GameFile_Close  GameFile_Close;				//void GameFile_Close();

	//System file manipulation. This allows the plugin to load individual files.
	XAPI_SysFile_Open   SysFile_Open;					//int32_t SysFile_Open(const char *pszFileName);
	XAPI_SysFile_Length SysFile_GetLength;				//uint32_t SysFile_Length();
	XAPI_SysFile_Read   SysFile_Read;					//void SysFile_Read(void *pData, uint32_t start, uint32_t length);
	XAPI_SysFile_Close  SysFile_Close;					//void SysFile_Close();

	//Parser
	XAPI_Parser_SetData		      Parser_SetData;			//void Parser_SetData(char *pMemory, uint32_t length, uint32_t filePtr);
	XAPI_Parser_SearchKeyword_int32_t Parser_SearchKeyword_int32_t; //int32_t Parser_SearchKeyword_int32_t(const char *pszKeyword, int32_t& value);
	XAPI_Parser_GetFilePtr	      Parser_GetFilePtr;		//uint32_t Parser_GetFilePtr();

	//(Movie) Cutscene Player
	XAPI_MoviePlayer_SetPlayer	  MoviePlayer_SetPlayer;		//
	XAPI_MoviePlayer_SetArchives  MoviePlayer_SetArchives;		//
	XAPI_MoviePlayer_Start		  MoviePlayer_Start;			//
	XAPI_MoviePlayer_Update		  MoviePlayer_Update;			//
	XAPI_MoviePlayer_Stop		  MoviePlayer_Stop;				//
	XAPI_MoviePlayer_Render		  MoviePlayer_Render;			//

	//Palette
	XAPI_SetGamePalette			  SetGamePalette;				//void SetGamePalette(uint8_t *pData, uint32_t uSize, uint32_t uTransparentIndex);
	XAPI_SetColormap			  SetColormap;					//void SetColormap(uint8_t index, uint8_t *pData, int32_t numLightLevels);

	//World
	XAPI_World_CreateTerrain	  World_CreateTerrain;			//void World_CreateTerrain(int width, int height)
	XAPI_World_UnloadAllCells	  World_UnloadAllCells;			//void World_UnloadAllCells(void);
	XAPI_World_LoadCell			  World_LoadCell;				//void World_LoadCell(uint32_t cellType, uint32_t archiveType, const char *pszArchive, const char *pszFile, int32_t worldX, int32_t worldY);
	XAPI_World_Collide			  World_Collide;				//void World_Collision(Vector3 *p0, Vector3 *p1, uint32_t& uSector, f32 fRadius, int32_t nPassThruAdjoins=0);
	XAPI_World_Activate			  World_Activate;				//void World_Activate(Vector3 *p0, Vector3 *p1, uint32_t& uSector);
	XAPI_World_Raycast			  World_Raycast;				//XL_BOOL World_Raycast(void *p0, void *p1, void *pInter);
	XAPI_World_IsPointInWater	  World_IsPointInWater;			//XL_BOOL World_IsPointInWater(void *p0);
	XAPI_World_GetCellWorldPos    World_GetCellWorldPos;		//void World_GetCellWorldPos(char *pszName, int *x, int *y)
	XAPI_World_GetCellStartPos    World_GetCellStartPos;	    //void World_GetCellStartPos(char *pszName, float *x, float *y, float *z)

	//Object
	XAPI_Object_Create			  Object_Create;				//uint32_t Object_Create(const char *pszName, int32_t nSector);
	XAPI_Object_Free			  Object_Free;					//void Object_Free(uint32_t uID);
	XAPI_Object_AddLogic		  Object_AddLogic;				//void Object_AddLogic(uint32_t uID, const char *pszLogic);
	XAPI_Object_GetPhysicsData	  Object_GetPhysicsData;		//ObjPhysicsData *Object_GetPhysicsData(uint32_t uID);
	XAPI_Object_GetGameData		  Object_GetGameData;			//void *Object_GetGameData(uint32_t uID);
	XAPI_Object_SetGameData		  Object_SetGameData;			//void Object_SetGameData(uint32_t uID, void *pData);
	XAPI_Object_ReserveObjects	  Object_ReserveObjects;		//void Object_ReserveObjects(uint32_t uReserveCount);
	XAPI_Object_FreeAllObjects	  Object_FreeAllObjects;		//void Object_FreeAllObjects();
	XAPI_SetObjectAngles		  Object_SetAngles;				//void Object_SetAngles(uint32_t uID, float x, float y, float z);
	XAPI_SetObjectAngles		  Object_SetPos;				//void Object_SetPos(uint32_t uID, float x, float y, float z);
	XAPI_ObjectEnableCollision    Object_EnableCollision;		//void Object_EnableCollision(uint32_t uID, int32_t enable);
	XAPI_Object_SendMessage		  Object_SendMessage;			//void Object_SendMessage(uint32_t uID, uint32_t msg, f32 value);
	XAPI_Object_SetRenderComponent Object_SetRenderComponent;	//void Object_SetRenderComponent(uint32_t uID, const char *pszComponentName);
	XAPI_Object_SetRenderTexture  Object_SetRenderTexture;		//void Object_SetRenderTexture(uint32_t uID, TextureHandle hTex);
	XAPI_Object_SetRenderFlip	  Object_SetRenderFlip;			//void Object_SetRenderFlip(uint32_t uID, XL_BOOL bFlipX, XL_BOOL bFlipY);
	XAPI_Object_SetWorldBounds	  Object_SetWorldBounds;		//void Object_SetWorldBounds( uint32_t uID, vMin.x, vMin.y, vMin.z, vMax.x, vMax.y, vMax.z );
	XAPI_Object_GetCameraVector   Object_GetCameraVector;		//void Object_GetCameraVector(uint32_t uObjID, float& x, float& y, float& z)
	XAPI_Object_SetActive		  Object_SetActive;				//void Object_SetActive(uint32_t uObjID, XL_BOOL bActive);

	//Logics
	XAPI_Logic_CreateFromCode	  Logic_CreateFromCode;			//void Logic_CreateFromCode(const char *pszName, void *pOwner, LogicFunction functions[]);
	XAPI_Logic_SetMessageMask	  Logic_SetMessageMask;			//void Logic_SetMessageMask(uint32_t uMask);

	//Texture Handling.
	XAPI_Texture_LoadTexList	  Texture_LoadTexList;			//void Texture_LoadTexList(uint32_t uTextureType, uint32_t uPalIndex, uint32_t uArchiveType, const char *pszArchive, const char *pszFile, int32_t nRecord, bool bGenMips);
	XAPI_Texture_GetSize		  Texture_GetSize;				//void Texture_GetSize(int32_t& ox, int32_t& oy, uint32_t& w, uint32_t& h, float& fw, float& fh);
	XAPI_Texture_GetExtraData	  Texture_GetExtraData;			//void *Texture_GetExtraData();

	//Pathing
	XAPI_Pathing_GetRandomNode	  Pathing_GetRandomNode;		//XL_BOOL Pathing_GetRandomNode(int32_t& nodeX, int32_t& nodeY, float& x, float& y, float& z, int32_t& sx, int32_t& sy)
	XAPI_Pathing_CheckNode		  Pathing_CheckNode;			//XL_BOOL Pathing_CheckNode(int32_t nodeX, int32_t nodeY)
	XAPI_Pathing_GetNodeVector	  Pathing_GetNodeVector;		//void Pathing_GetNodeVector(float& vx, float& vy, float cx, float cy, int32_t wx, int32_t wy, int32_t nodeX, int32_t nodeY)
};

//Helper MACRO to map from "C" Logic callbacks to "C++" member functions.
#define LOGIC_CB_MAP(c) \
void c::LogicSetupCB(void *pOwner, uint32_t uObjID, uint32_t uParamCount, LogicParam *param) { ((c *)pOwner)->LogicSetup(uObjID, uParamCount, param); }	\
void c::ObjectSetupCB(void *pOwner, uint32_t uObjID, uint32_t uParamCount, LogicParam *param) { ((c *)pOwner)->ObjectSetup(uObjID, uParamCount, param); }	\
void c::UpdateCB(void *pOwner, uint32_t uObjID, uint32_t uParamCount, LogicParam *param) { ((c *)pOwner)->Update(uObjID, uParamCount, param); }	\
void c::MessageCB(void *pOwner, uint32_t uObjID, uint32_t uParamCount, LogicParam *param) { ((c *)pOwner)->Message(uObjID, uParamCount, param); }

//Define the static Callback functions in the class definition.
#define LOGIC_CB_FUNC() \
static void LogicSetupCB(void *pOwner, uint32_t uObjID, uint32_t uParamCount, LogicParam *param); \
static void ObjectSetupCB(void *pOwner, uint32_t uObjID, uint32_t uParamCount, LogicParam *param); \
static void UpdateCB(void *pOwner, uint32_t uObjID, uint32_t uParamCount, LogicParam *param); \
static void MessageCB(void *pOwner, uint32_t uObjID, uint32_t uParamCount, LogicParam *param);

#define LOGIC_FUNC_LIST(f) \
LogicFunction f[]={ LogicSetupCB,	ObjectSetupCB, UpdateCB, MessageCB }

#ifdef  __cplusplus
}
#endif

#endif // XLENGINE_PLUGIN_API_H

#include "Engine.h"
#include "EngineSettings.h"
#include "render/IDriver3D.h"
#include "render/Driver3D_OGL.h"
#include "render/Driver3D_Soft.h"
#if PLATFORM_WIN
    #include "render/Win/Driver3D_OGL_Win.h"
#elif PLATFORM_LINUX
    #include "render/linux/Driver3D_OGL_Linux.h"
	#include <memory.h>
	#include <malloc.h>
#endif
#include "render/Camera.h"
#include "render/TextureCache.h"
#include "render/MeshCache.h"
#include "render/FontManager.h"
#include "render/RenderQue.h"
#include "os/Input.h"
#include "os/Clock.h"
#include "ui/XL_Console.h"
#include "ui/UI_System.h"
#include "fileformats/ArchiveManager.h"
#include "fileformats/Parser.h"
#include "fileformats/TextureLoader.h"
#include "fileformats/CellManager.h"
#include "fileformats/Location_Daggerfall.h"
#include "movieplayback/MovieManager.h"
#include "scriptsystem/ScriptSystem.h"
#include "memory/ScratchPad.h"
#include "world/World.h"
#include "world/WorldCell.h"
#include "world/Terrain.h"
#include "world/ObjectManager.h"
#include "world/LogicManager.h"
#include "world/LevelFuncMgr.h"
#include "world/Sector.h"
#include "procedural/Noise.h"
#include "plugin_framework/XLEngine_Plugin_API.h"
#include "plugin_framework/PluginManager.h"
#include "networking/NetworkMgr.h"
#include "math/Math.h"
#include "PluginAPI_Func.h"

//Game Loop
#define MAX_LOOP_ITER 8
const f32 m_fFixedLoopsPerSec = 60.0f;
const f32 m_fFixedLoopTime = (1.0f/m_fFixedLoopsPerSec);

//Plugin API.
Engine *m_pEngineForAPI=NULL;
XAPI_Game_Update m_pGameUpdate=NULL;
XAPI_Game_Render m_pGameRender=NULL;
XAPI_World_Update m_pWorldUpdate=NULL;
void *m_pGameUpdate_UD=NULL;
void *m_pGameRender_UD=NULL;
void *m_pWorldUpdate_UD=NULL;
char m_szGameName[64]="";
int m_anVersion[2]={0,0};

//Messages to display for debugging or chatting.
struct DisplayMessage
{
	char msg[128];
	f32 displayTime;
	Vector4 color;
};
#define MAX_MESSAGE_COUNT 32
DisplayMessage m_aMessages[MAX_MESSAGE_COUNT];
int32_t m_nMessageCnt=0;

IDriver3D *g_pDriver3D = NULL;

#define _MAX_TIMEOUT 5.0f
#define _FRAME_COUNT_BEFORE_EXIT 60

//
bool Engine::m_bContinueLoop=true;

/*********** Implementation ****************/

Engine::Engine()
{
    m_pDriver3D = NULL;
	m_pCamera = NULL;
	m_pPluginAPI = NULL;
	m_fTotalTime = 0.0f;
	m_pSystemFont16 = NULL;
	m_pSystemFont24 = NULL;
	m_pSystemFont32 = NULL;
	m_pWorld = NULL;
	m_FPS = 60.0f;
}

Engine::~Engine()
{
	Destroy();
}

bool Engine::Init(void **winParam, int32_t paramCnt, int32_t w, int32_t h)
{
    //Initialize 3D.
	if ( EngineSettings::IsServer() == false )
	{
		//Create the low-level renderer
		int32_t nRenderer = EngineSettings::GetRenderer();
		if ( nRenderer == EngineSettings::RENDERER_OPENGL )
		{
			m_pDriver3D = xlNew Driver3D_OGL();
		}
		else if ( nRenderer == EngineSettings::RENDERER_SOFT32 )
		{
			m_pDriver3D = xlNew Driver3D_Soft();
			((Driver3D_Soft *)m_pDriver3D)->SetBitDepth(32);
		}
		else if ( nRenderer == EngineSettings::RENDERER_SOFT8 )
		{
			m_pDriver3D = xlNew Driver3D_Soft();
			((Driver3D_Soft *)m_pDriver3D)->SetBitDepth(8);
		}

		if ( m_pDriver3D == NULL )
			return false;

		g_pDriver3D = m_pDriver3D;
	}

	//Create the default camera.
	m_pCamera = xlNew Camera();
	if ( m_pCamera == NULL )
		return false;

    //pick the correct platform based on OS.
	if ( EngineSettings::IsServer() == false )
	{
#if PLATFORM_WIN
		m_pDriver3D->SetPlatform( xlNew Driver3D_OGL_Win() );
#elif PLATFORM_LINUX
		m_pDriver3D->SetPlatform( xlNew Driver3D_OGL_Linux() );
#endif

		m_pDriver3D->SetWindowData(paramCnt, winParam);
		m_pDriver3D->Init(w, h);
	}

	//ScratchPad
	ScratchPad::Init();

	//Setup the script system.
	ScriptSystem::Init();

    //Initialize Input.
	Input::Init();

	//Initialize the Clock.
	Clock::Init();

	//Initialize the Noise System.
	Noise::Init();

	//Initialize the world map.
	WorldMap::Init();

	//Initialie the Render Que.
	if ( EngineSettings::IsServer() == false )
	{
		RenderQue::Init( m_pDriver3D );

		//Initialize Texture Cache.
		TextureCache::Init( m_pDriver3D );

		//Initialize the Mesh Cache.
		MeshCache::Init();

		//Initialize the Font Manager.
		FontManager::Init( "fonts/", m_pDriver3D );

		//Load the basic system fonts.
		m_pSystemFont16 = FontManager::LoadFont( "Verdana16.fnt" );
		m_pSystemFont24 = FontManager::LoadFont( "Verdana24.fnt" );
		m_pSystemFont32 = FontManager::LoadFont( "Verdana32.fnt" );
	}

	//Setup the console.
	XL_Console::Init( m_pDriver3D );

	//Register Engine script functions.
	ScriptSystem::RegisterFunc("void Game_Exit()", asFUNCTION(PostExitMessage));
	ScriptSystem::RegisterFunc("float math_modf(float, float)", asFUNCTION(fmodf));
	ScriptSystem::RegisterFunc("float math_absf(float)", asFUNCTION(fabsf));
	ScriptSystem::RegisterFunc("float math_cosf(float)", asFUNCTION(cosf));
	ScriptSystem::RegisterFunc("float math_sinf(float)", asFUNCTION(sinf));

	//Create world.
	m_pWorld = xlNew World();
	m_pWorld->SetCamera( m_pCamera );

	if ( EngineSettings::IsServer() == false )
	{
		//Initialize the UI System
		UI_System::Init( m_pDriver3D, this );
	}

	//Initialize the Archive manager
	ArchiveManager::Init();

	if ( EngineSettings::IsServer() == false )
	{
		//Initialize the movie playback system.
		MovieManager::Init( m_pDriver3D );
	}

	//Initialize World
	CellManager::Init();

	//Initialize Objects and Logics
	ObjectManager::Init(m_pWorld);

	//Initialize Level Functions.
	LevelFuncMgr::Init();

	//Setup the default camera.
	Vector3 eye(0.0f, 0.0f, 0.0f), dir(0.0f, -1.0f, 0.0f);
	m_pCamera->SetLoc(eye);
	m_pCamera->SetDir(dir);
	//make FOV changeable by game. Default is 65.
	//Change for Blood temporarily.
	m_pCamera->SetFOV(60.0f/*75.0f*/, (f32)w / (f32)h);

	m_nWidth  = w;
	m_nHeight = h;

	if ( EngineSettings::IsServer() || EngineSettings::IsClient_MP() )
	{
		uint32_t uWaitFrame = 0;
		int32_t nExitFrameCnt = -2;

		if ( NetworkMgr::Init(this) == false )
		{
			if ( EngineSettings::IsClient_MP() )
			{
				//delay before exiting so an error message can be presented.
				nExitFrameCnt = _FRAME_COUNT_BEFORE_EXIT;
			}
			else
			{
				//the server just exits immediately.
				return false;
			}
		}
		if ( EngineSettings::IsServer() == false && EngineSettings::IsClient_MP() )
		{
			Clock::StartTimer(1);
			//now wait for the client/server connection and startup.
			while ( NetworkMgr::HasRecievedStartupMsg() == false )
			{
				m_pDriver3D->Clear( true );

				//setup orthographic view, used for things like UI.
				Matrix projMtx;
				projMtx.ProjOrtho((f32)m_nWidth, (f32)m_nHeight);

                Vector3 pLoc = Vector3(0,0,0);
                Vector3 pDir = Vector3(0,0,1);

				m_pDriver3D->SetProjMtx( &projMtx );
				m_pDriver3D->SetViewMatrix( &Matrix::s_Identity, &pLoc, &pDir);
				m_pDriver3D->SetWorldMatrix( &Matrix::s_Identity, 0, 0 );

				//now only wait so long...
				f32 fDeltaTime = Clock::GetDeltaTime(_MAX_TIMEOUT, 1);
				if ( fDeltaTime >= _MAX_TIMEOUT && nExitFrameCnt < 0 )
				{
					XL_Console::PrintF("^1Error: The XL Engine timed out waiting for a server response. Aborting.");
					nExitFrameCnt = _FRAME_COUNT_BEFORE_EXIT;
				}

				//render something here?
				XLFont *pFont = GetSystemFont(24);
				if ( pFont )
				{
					FontManager::BeginTextRendering();
					{
						char szWaitMsg[128];
						int32_t xOffs;

						if ( nExitFrameCnt < 0 )
						{
							uint32_t uDotCnt = (uWaitFrame>>3)%10;
							strcpy(szWaitMsg, "Waiting for server response");
							size_t l = strlen(szWaitMsg);
							uint32_t d=0;
							for (; d<uDotCnt; d++)
							{
								szWaitMsg[l+d] = '.';
							}
							szWaitMsg[l+d] = 0;
							xOffs = 140;
						}
						else
						{
							strcpy(szWaitMsg, "Error: The XL Engine timed out waiting for a server response. Aborting.");
							xOffs = 360;
						}
						FontManager::RenderString((m_nWidth>>1)-xOffs, (m_nHeight>>1)-12, szWaitMsg, pFont, &Vector4::One);
					}
					FontManager::EndTextRendering();
				}

				m_pDriver3D->Present();
				uWaitFrame++;

				//After deciding to exit, there is a delay of _FRAME_COUNT_BEFORE_EXIT
				//so that users can actually read the error message.
				if ( nExitFrameCnt > 0 )
					nExitFrameCnt--;

				if ( nExitFrameCnt == 0 )
				{
					return false;
				}
			};
		}
	}
	SetupPluginAPI();

    return true;
}

void Engine::Destroy()
{
	if ( EngineSettings::IsServer() || EngineSettings::IsClient_MP() )
	{
		NetworkMgr::Destroy();
	}

	if ( m_pCamera )
	{
		xlDelete m_pCamera;
		m_pCamera = NULL;
	}
	if ( m_pPluginAPI )
	{
		xlFree( m_pPluginAPI );
		m_pPluginAPI = NULL;
	}
	MovieManager::Destroy();
    Input::Destroy();
	Clock::Destroy();
	WorldMap::Destroy();
	XL_Console::Destroy();
	ScriptSystem::Destroy();
	UI_System::Destroy();
	ArchiveManager::Destroy();
	PluginManager::Destroy();
	FontManager::Destroy();
	TextureCache::Destroy();
	MeshCache::Destroy();
	RenderQue::Destroy();
	CellManager::Destroy();
	ObjectManager::Destroy();
	LevelFuncMgr::Destroy();

	if ( m_pDriver3D )
    {
        xlDelete m_pDriver3D;
        m_pDriver3D = NULL;
    }

	//ScratchPad
	ScratchPad::Destroy();

	if ( m_pWorld )
	{
		xlDelete m_pWorld;
		m_pWorld = NULL;
	}
}

void Engine::InitGame(const char *pszGameLib)
{
	PluginManager::InitGame( pszGameLib );
}

XLFont *Engine::GetSystemFont(int32_t size)
{
	XLFont *pFont = NULL;

	if ( size == 16 )
		pFont = m_pSystemFont16;
	else if ( size == 24 )
		pFont = m_pSystemFont24;
	else if ( size == 32 )
		pFont = m_pSystemFont32;

	return pFont;
}

void Engine::AddDisplayMessage(const char *pszMsg, Vector4 *color, f32 fShowTime)
{
	if ( m_nMessageCnt >= MAX_MESSAGE_COUNT )
	{
		//must remove the first message.
		for (int32_t m=0; m<m_nMessageCnt-1; m++)
		{
			m_aMessages[m] = m_aMessages[m+1];
		}
		m_nMessageCnt--;
	}

	// todo: think about using std::string for safe string
	//strcpy_s(m_aMessages[ m_nMessageCnt ].msg, 127, pszMsg);
	strcpy(m_aMessages[ m_nMessageCnt ].msg, pszMsg);
	m_aMessages[ m_nMessageCnt ].displayTime = (fShowTime == 0.0f) ? 3.0f : fShowTime;
	if ( color )
	{
		m_aMessages[ m_nMessageCnt ].color = *color;
	}
	else
	{
		m_aMessages[ m_nMessageCnt ].color.Set(0.75f, 0.75f, 0.75f, 1.0f);
	}
	m_nMessageCnt++;

	//Also route to the console.
	XL_Console::PrintF("^5%s", pszMsg);
}

f32 Engine::GetCurrentBrightness()
{
	Object *player = m_pWorld->GetPlayer();
	if ( player == NULL )
		return 1.0f;

	return player->GetBrightness();
}

f32 Engine::GetCurrentSpeed()
{
	return m_pCamera->GetSpeed();
}

//XL Engine Plugin API layer.
int32_t Game_SetUpdateCallback(XAPI_Game_Update pGameUpdate, void *pUserData)
{
	m_pGameUpdate    = pGameUpdate;
	m_pGameUpdate_UD = pUserData;

	return 1;
}

int32_t Game_SetRenderCallback(XAPI_Game_Render pGameRender, void *pUserData)
{
	m_pGameRender    = pGameRender;
	m_pGameRender_UD = pUserData;

	return 1;
}

int32_t World_SetUpdateCallback(XAPI_World_Update pWorldUpdate, void *pUserData)
{
	m_pWorldUpdate    = pWorldUpdate;
	m_pWorldUpdate_UD = pUserData;

	return 1;
}

void Engine::WorldUpdate(int32_t newWorldX, int32_t newWorldY)
{
	if ( m_pWorldUpdate )
	{
		m_pWorldUpdate( newWorldX, newWorldY, m_pPluginAPI, m_pWorldUpdate_UD );
	}
}

void Game_SetGameData(const char *pszName, int versionMajor, int versionMinor)
{
	strcpy(m_szGameName, pszName);
	m_anVersion[0] = versionMajor;
	m_anVersion[1] = versionMinor;

	XL_Console::SetGameInfo( m_szGameName, m_anVersion[0], m_anVersion[1] );
	char szGameDir[260];
	sprintf(szGameDir, "%s/", m_szGameName);
	ScriptSystem::SetGameDir( szGameDir );
	TextureCache::SetPath( szGameDir );
}

void Game_LoadWorldMap(void)
{
	WorldMap::Load();
}

void Engine::Engine_SetCameraData(f32 *pos, f32 *dir, f32 fSkew, f32 fSpeed, uint32_t uSector)
{
	Camera *pCamera = m_pEngineForAPI->m_pCamera;
	Vector3 vLoc = Vector3(pos[0], pos[1], pos[2]);
	pCamera->SetLoc( vLoc );
	Vector3 vDir = Vector3(dir[0], dir[1], dir[2]);
	pCamera->SetDir( vDir );
	pCamera->SetSkew(fSkew);
	pCamera->SetSpeed( fSpeed );
	pCamera->SetSector( uSector );
}

XL_BOOL Engine::Engine_AllowPlayerControls(void)
{
	return (XL_Console::IsActive() == true || XL_Console::IsChatActive() == true ) ? XL_FALSE : XL_TRUE;
}

void UnloadAllWorldCells(void)
{
	m_pEngineForAPI->GetWorld()->UnloadWorldCells();
}

void LoadWorldCell(uint32_t cellType, uint32_t archiveType, const char *pszArchive, const char *pszFile, int32_t worldX, int32_t worldY)
{
	Archive *pArchive = ArchiveManager::OpenArchive(archiveType, pszArchive);
	if ( pArchive )
	{
		WorldCell *pCell = CellManager::LoadCell(g_pDriver3D, m_pEngineForAPI->GetWorld(), cellType, pArchive, pszFile, worldX, worldY);
		if ( pCell )
		{
			m_pEngineForAPI->GetWorld()->AddWorldCell( pCell );
		}
	}
	Object *player = ObjectManager::FindObject("PLAYER");
	m_pEngineForAPI->GetWorld()->SetPlayer( player );
}

void World_CreateTerrain(int32_t width, int32_t height)
{
	//Create the terrain.
	Terrain *pTerrain = xlNew Terrain( m_pEngineForAPI->GetDriver(), m_pEngineForAPI->GetWorld() );
	//Initialize it to the proper coordinates.
	pTerrain->Update(100<<3, 100<<3);
	//For now activite it, don't do this in the future.
	pTerrain->Activate(true);

	//Update the world.
	m_pEngineForAPI->GetWorld()->SetTerrain( pTerrain );
}

XL_BOOL World_IsPointInWater(void *p0)
{
	Vector3 *pos = (Vector3 *)p0;
	World *pWorld = m_pEngineForAPI->GetWorld();

	if ( !pWorld->IsSectorTypeVis(SECTOR_TYPE_EXTERIOR) )
		return XL_FALSE;

	return pWorld->GetTerrain()->IsPointInWater(pos->x, pos->y);
}

void Engine::Object_GetCameraVector(uint32_t uObjID, float& x, float& y, float& z)
{
	Object *pObj = ObjectManager::GetObjectFromID(uObjID);
	if ( pObj )
	{
		Vector3 vPos;
		pObj->GetLoc(vPos);

		vPos.x += (float)(pObj->GetWorldX() - m_pEngineForAPI->m_pCamera->GetWorldPosX()) * 1024.0f;
		vPos.y += (float)(pObj->GetWorldY() - m_pEngineForAPI->m_pCamera->GetWorldPosY()) * 1024.0f;

		const Vector3& vCamLoc = m_pEngineForAPI->m_pCamera->GetLoc();
		x = vPos.x - vCamLoc.x;
		y = vPos.y - vCamLoc.y;
		z = vPos.z - vCamLoc.z;
	}
}

XL_BOOL Pathing_GetRandomNode(int32_t& nodeX, int32_t& nodeY, float& x, float& y, float& z, int32_t& sx, int32_t& sy)
{
	Vector3 outPos;
	bool bFound = m_pEngineForAPI->GetWorld()->GetRandomNode(nodeX, nodeY, outPos, sx, sy);

	x = outPos.x; y = outPos.y; z = outPos.z;

	return bFound ? XL_TRUE : XL_FALSE;
}

XL_BOOL Pathing_CheckNode(int32_t nodeX, int32_t nodeY)
{
	return m_pEngineForAPI->GetWorld()->CheckNode(nodeX, nodeY);
}

void Pathing_GetNodeVector(float& vx, float& vy, float cx, float cy, int32_t wx, int32_t wy, int32_t nodeX, int32_t nodeY)
{
	Vector3 curPos(cx, cy, 0.0f);
	Vector2 vOffset;
	m_pEngineForAPI->GetWorld()->GetNodeVector(vOffset, curPos, wx, wy, nodeX, nodeY);
	vx = vOffset.x;
	vy = vOffset.y;
}

void World_Collide(void *p0, void *p1, uint32_t& uSector, f32 fRadius, int32_t bPassThruAdjoins)
{
	m_pEngineForAPI->GetWorld()->Collide((Vector3 *)p0, (Vector3 *)p1, uSector, fRadius, bPassThruAdjoins ? true : false);
}

void World_Activate(void *p0, void *p1, uint32_t& uSector)
{
	m_pEngineForAPI->GetWorld()->RayCastAndActivate((Vector3 *)p0, (Vector3 *)p1, uSector);
}

XL_BOOL World_Raycast(void *p0, void *p1, void *pInter)
{
	bool bFound = m_pEngineForAPI->GetWorld()->Raycast((Vector3 *)p0, (Vector3 *)p1, (Vector3 *)pInter);
	return bFound ? XL_TRUE : XL_FALSE;
}

void World_GetCellWorldPos(char *pszName, int *x, int *y)
{
	Location_Daggerfall *pLocation = WorldMap::GetLocation(pszName);
	*x = (int)pLocation->m_x;
	*y = (int)pLocation->m_y;
}

void World_GetCellStartPos(char *pszName, float *x, float *y, float *z)
{
	Location_Daggerfall *pLocation = WorldMap::GetLocation(pszName);
	int wx = ((int)pLocation->m_x)>>3;
	int wy = ((int)pLocation->m_y)>>3;
	WorldCell *pCell = WorldMap::GetWorldCell(wx, wy);

	Vector3 vStartLoc;
	pCell->GetStartLoc(vStartLoc);

	*x = vStartLoc.x;
	*y = vStartLoc.y;
	*z = vStartLoc.z;
}

void Engine::SetupPluginAPI()
{
	m_pPluginAPI = (XLEngine_Plugin_API *)xlMalloc( sizeof(XLEngine_Plugin_API) );
	memset(m_pPluginAPI, 0, sizeof(XLEngine_Plugin_API));
	//there can only be one engine/API connection at a time
	m_pEngineForAPI = this;
	//General
	m_pPluginAPI->SetGameUpdateCallback = Game_SetUpdateCallback;
	m_pPluginAPI->SetGameRenderCallback = Game_SetRenderCallback;
	m_pPluginAPI->SetGameData			= Game_SetGameData;
	m_pPluginAPI->LoadWorldMap          = Game_LoadWorldMap;
	m_pPluginAPI->World_SetUpdateCallback= World_SetUpdateCallback;
	//Engine
	m_pPluginAPI->Engine_SetCameraData  = Engine::Engine_SetCameraData;
	m_pPluginAPI->Engine_AllowPlayerControls = Engine::Engine_AllowPlayerControls;
	//Startup Options
	m_pPluginAPI->Startup_GetStartMap   = EngineSettings::GetStartMap;
	m_pPluginAPI->IsServer				= EngineSettings::IsServer;
	//Input
	m_pPluginAPI->IsKeyDown = Input_IsKeyDown;
	m_pPluginAPI->GetMouseX = Input_GetMousePosX;
	m_pPluginAPI->GetMouseY = Input_GetMousePosY;
	m_pPluginAPI->GetMouseDx = Input::GetMouseDx;
	m_pPluginAPI->GetMouseDy = Input::GetMouseDy;
	m_pPluginAPI->AddKeyDownCallback  = Input_AddKeyDownCB;
	m_pPluginAPI->AddCharDownCallback = Input_AddCharDownCB;
	m_pPluginAPI->EnableMouseLocking  = Input::EnableMouseLocking;
	//Console
	m_pPluginAPI->RegisterConsoleCmd  = Console_RegisterCommand;
	m_pPluginAPI->PrintToConsole      = XL_Console::PrintF;
	m_pPluginAPI->SetConsoleColor	  = XL_Console::SetConsoleColor;
	//Scripts
	m_pPluginAPI->Start_UI_Script	  = UI_System::StartScript;
	m_pPluginAPI->RegisterScriptFunc  = ScriptSystem_RegisterScriptFunc;
	//Game File Manipulation
	m_pPluginAPI->GameFile_Open		  = ArchiveManager::GameFile_Open;
	m_pPluginAPI->GameFile_GetLength  = ArchiveManager::GameFile_GetLength;
	m_pPluginAPI->GameFile_Read		  = ArchiveManager::GameFile_Read;
	m_pPluginAPI->GameFile_Close	  = ArchiveManager::GameFile_Close;
	//System File Manipulation
	m_pPluginAPI->SysFile_Open		  = ArchiveManager::File_Open;
	m_pPluginAPI->SysFile_GetLength   = ArchiveManager::File_GetLength;
	m_pPluginAPI->SysFile_Read		  = ArchiveManager::File_Read;
	m_pPluginAPI->SysFile_Close		  = ArchiveManager::File_Close;
	//Parser
	m_pPluginAPI->Parser_SetData	     = Parser::SetData;
	m_pPluginAPI->Parser_SearchKeyword_int32_t = Parser::SearchKeyword_int32_t;
	m_pPluginAPI->Parser_GetFilePtr	     = Parser::GetFilePtr;
	//Movie playback
	m_pPluginAPI->MoviePlayer_SetPlayer  = MovieManager::SetPlayerType;
	m_pPluginAPI->MoviePlayer_SetArchives= MovieManager::SetPlayerArchives;
	m_pPluginAPI->MoviePlayer_Start	     = MovieManager::StartMovie;
	m_pPluginAPI->MoviePlayer_Stop       = MovieManager::StopMovie;
	m_pPluginAPI->MoviePlayer_Render     = MovieManager::RenderMovie;
	m_pPluginAPI->MoviePlayer_Update     = MovieManager::UpdateMovie;
	//Palette
	m_pPluginAPI->SetGamePalette	     = TextureLoader::SetPalette;
	m_pPluginAPI->SetColormap		     = TextureLoader::SetColormap;
	//World
	m_pPluginAPI->World_CreateTerrain    = World_CreateTerrain;
	m_pPluginAPI->World_UnloadAllCells   = UnloadAllWorldCells;		//void World_UnloadAllCells(void);
	m_pPluginAPI->World_LoadCell	     = LoadWorldCell;			//void World_LoadCell(uint32_t cellType, uint32_t archiveType, const char *pszArchive, const char *pszFile, int32_t worldX, int32_t worldY);
	m_pPluginAPI->World_Collide		     = World_Collide;
	m_pPluginAPI->World_Activate	     = World_Activate;
	m_pPluginAPI->World_Raycast		     = World_Raycast;
	m_pPluginAPI->World_IsPointInWater   = World_IsPointInWater;
	m_pPluginAPI->World_GetCellWorldPos  = World_GetCellWorldPos;
	m_pPluginAPI->World_GetCellStartPos  = World_GetCellStartPos;
	//Object
	m_pPluginAPI->Object_Create		     = ObjectManager::CreateObjectID;
	m_pPluginAPI->Object_Free		     = ObjectManager::FreeObjectID;
	m_pPluginAPI->Object_AddLogic	     = ObjectManager::AddLogicToObjID;
	m_pPluginAPI->Object_GetPhysicsData  = ObjectManager::GetObjectPhysicsData;
	m_pPluginAPI->Object_GetGameData     = ObjectManager::GetObjectGameData;
	m_pPluginAPI->Object_SetGameData     = ObjectManager::SetObjectGameData;
	m_pPluginAPI->Object_ReserveObjects  = ObjectManager::ReserveObjects;
	m_pPluginAPI->Object_FreeAllObjects  = ObjectManager::FreeAllObjects;
	m_pPluginAPI->Object_SetAngles	     = ObjectManager::SetObjectAngles;
	m_pPluginAPI->Object_SetPos		     = ObjectManager::SetObjectPos;
	m_pPluginAPI->Object_EnableCollision = ObjectManager::EnableObjectCollision;
	m_pPluginAPI->Object_SendMessage     = ObjectManager::SendMessage;
	m_pPluginAPI->Object_SetRenderComponent = ObjectManager::SetRenderComponent;
	m_pPluginAPI->Object_SetRenderTexture= ObjectManager::SetRenderTexture;
	m_pPluginAPI->Object_SetRenderFlip	 = ObjectManager::SetRenderFlip;
	m_pPluginAPI->Object_SetWorldBounds  = ObjectManager::SetWorldBounds_API;
	m_pPluginAPI->Object_GetCameraVector = Object_GetCameraVector;
	m_pPluginAPI->Object_SetActive		 = ObjectManager::SetActive;
	//Logic
	m_pPluginAPI->Logic_CreateFromCode   = LogicManager::CreateLogicFromCode_API;
	m_pPluginAPI->Logic_SetMessageMask   = Logic::SetMessageMask_CurLogic;
	//Textures
	m_pPluginAPI->Texture_LoadTexList    = TextureCache::GameFile_LoadTexture_TexList_API;
	m_pPluginAPI->Texture_GetSize	     = TextureCache::GetTextureSize;
	m_pPluginAPI->Texture_GetExtraData   = TextureCache::GetTexExtraData;
	//Pathing
	m_pPluginAPI->Pathing_GetRandomNode  = Pathing_GetRandomNode;
	m_pPluginAPI->Pathing_CheckNode      = Pathing_CheckNode;
	m_pPluginAPI->Pathing_GetNodeVector  = Pathing_GetNodeVector;

	//Initialize the Plugin Manager
	PluginManager::Init( m_pPluginAPI );
}

void Engine::ChangeWindowSize(int32_t w, int32_t h)
{
    m_pDriver3D->ChangeWindowSize(w, h);
}

bool Engine::Loop(f32 fDeltaTime, bool bFullspeed)
{
	m_FPS = fDeltaTime > 0.0f ? 1.0f/fDeltaTime : 0.0f;

	//hack...
	if ( Input::IsKeyDown(XL_F1) )
	{
		((Driver3D_Soft *)m_pDriver3D)->SetBitDepth(8);
	}
	else if ( Input::IsKeyDown(XL_F2) )
	{
		((Driver3D_Soft *)m_pDriver3D)->SetBitDepth(32);
	}

	//call update on game systems.
	if ( m_pGameUpdate && XL_Console::IsActive() == false )
	{
		//loop with fixed time intervals.
		int nLoopCount = 0;
		m_fTotalTime += fDeltaTime*m_fFixedLoopsPerSec;
		while (m_fTotalTime >= 1.0f)
		{
			//networking - which also occurs at 60fps.
			NetworkMgr::Loop();

			//Game specific update.
			m_pGameUpdate( UPDATE_STAGE_FIXED, m_fFixedLoopTime, m_pPluginAPI, m_pGameUpdate_UD );

			//update the script system at fixed intervals.
			ScriptSystem::Update();

			//update the world.
			ObjectManager::Update();
			if ( m_pWorld->Update(fDeltaTime, m_pDriver3D) )
			{
				WorldUpdate( m_pCamera->GetWorldPosX(), m_pCamera->GetWorldPosY() );
			}
			LevelFuncMgr::Update();

			//update the UI.
			UI_System::Update();

			//clear the mouse delta, which is accumulated across unused frames.
			Input::SetMouseDelta(0,0);

			m_fTotalTime -= 1.0f;
			nLoopCount++;

			//if we've spent too much time in this loop, then we'll just pretend that we've done enough
			//and bail...
			if ( nLoopCount > MAX_LOOP_ITER )
			{
				XL_Console::PrintF("^1Error: Fixed time step loop count exceeded the limit: %d. time = %2.2f", nLoopCount, m_fTotalTime);
				m_fTotalTime = m_fTotalTime - floorf(m_fTotalTime);
				break;
			}
		};

		//variable deltatime update.
		m_pGameUpdate( UPDATE_STAGE_VARIABLE, fDeltaTime, m_pPluginAPI, m_pGameUpdate_UD );
	}
    //Update the UI System. Note that if the Console is up, the UI System does not get
	//updates but does still render.
	if ( XL_Console::IsActive() == false && XL_Console::IsChatActive() == false )
	{
		m_pCamera->Update(fDeltaTime);
	}

	if ( EngineSettings::IsServer() == false )
	{
		//-FIX ME! Later have a better way of determining this.
		//basically only clear when no 3D is rendered.
		m_pDriver3D->Clear( true );//Input::LockMouse() ? false : true );

		{
			m_pDriver3D->EnableDepthRead(true);
			m_pDriver3D->EnableDepthWrite(true);
			m_pDriver3D->EnableCulling(false);
			m_pDriver3D->SetColor();

			//setup camera.
			m_pCamera->Set(m_pDriver3D);
			//set default world matrix.
			m_pDriver3D->SetWorldMatrix( &Matrix::s_Identity, 0, 0 );

			//call game PreWorld Render
			if ( m_pGameRender )
			{
				m_pGameRender( RENDER_STAGE_PREWORLD, fDeltaTime, m_pPluginAPI, m_pGameRender_UD );
			}

			//do world render stuff here...
			m_pWorld->Render( m_pDriver3D );

			//setup orthographic view, used for things like UI.
			Matrix projMtx;
			projMtx.ProjOrtho((f32)m_nWidth, (f32)m_nHeight);

			Vector3 pDir = Vector3(0,0,0);
			Vector3 pLoc = Vector3(0,0,1);

			m_pDriver3D->SetProjMtx( &projMtx );
			m_pDriver3D->SetViewMatrix( &Matrix::s_Identity, &pDir, &pLoc );
			m_pDriver3D->SetWorldMatrix( &Matrix::s_Identity, 0, 0 );

			//call game PostWorld Render (usually UI).
			if ( m_pGameRender )
			{
				m_pGameRender( RENDER_STAGE_POSTWORLD, fDeltaTime, m_pPluginAPI, m_pGameRender_UD );
			}

			//Engine UI System
			UI_System::Render();

			//Render any messages.
			DisplayMessages(fDeltaTime);

			//Render the console on top...
			XL_Console::Render();

			m_pDriver3D->EnableDepthRead(true);
			m_pDriver3D->EnableDepthWrite(true);
		}

		m_pDriver3D->Present();
	}

	return m_bContinueLoop;
}

void Engine::DisplayMessages(f32 fDeltaTime)
{
	//1. remove old messages. Only one at a time though.
	for (int32_t m=0; m<m_nMessageCnt; m++)
	{
		m_aMessages[m].displayTime -= fDeltaTime;
	}
	XLFont *pFont = GetSystemFont(16);
	if ( m_nMessageCnt > 0 && m_aMessages[0].displayTime <= 0 )
	{
		//must remove the first message.
		for (int32_t m=0; m<m_nMessageCnt-1; m++)
		{
			m_aMessages[m] = m_aMessages[m+1];
		}
		m_nMessageCnt--;
	}
	//2. now display the messages.
	if ( pFont )
	{
		FontManager::BeginTextRendering();
		{
			//display FPS
			int32_t y=10;
			/*
			char szFPS[64];
			sprintf(szFPS, "FPS %2.2f", m_FPS);
			Vector4 fc(1.0f, 1.0f, 1.0f, 1.0f);
			Vector4 bc(0.0f, 0.0f, 0.0f, 1.0f);
			FontManager::RenderString(12, y+2, szFPS, pFont, &bc);
			FontManager::RenderString(10, y, szFPS, pFont, &fc);

			y += 24;
			*/

			for (int32_t m=0; m<m_nMessageCnt; m++, y+= 18)
			{
				f32 alpha = m_aMessages[m].color.w * Math::clamp(m_aMessages[m].displayTime, 0.0f, 1.0f);
				Vector4 color(m_aMessages[m].color.x, m_aMessages[m].color.y, m_aMessages[m].color.z, alpha);
				Vector4 shadow(0.0f, 0.0f, 0.0f, alpha);
				FontManager::RenderString(12, y+2, m_aMessages[m].msg, pFont, &shadow);
				FontManager::RenderString(10, y, m_aMessages[m].msg, pFont, &color);
			}
		}
		FontManager::EndTextRendering();

		m_pDriver3D->SetBlendMode();
		m_pDriver3D->EnableAlphaTest(false);
		m_pDriver3D->EnableCulling(true);
		m_pDriver3D->EnableDepthRead(true);
		m_pDriver3D->EnableDepthWrite(true);
	}
}

void Engine::PostExitMessage()
{
	m_bContinueLoop = false;
}

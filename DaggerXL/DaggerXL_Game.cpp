#include "DaggerXL_Game.h"
#include "DaggerXL_Player.h"
#include "Logic_Door.h"
#include "Logic_Obj_Action.h"
#include "Logic_NPC.h"
#include "../fileformats/ArchiveTypes.h"
#include "../fileformats/CellTypes.h"
#include "../ui/Console.h"

//Game instance used for console commands.
DaggerXL_Game *DaggerXL_Game::s_pGame_Console=NULL;
#define NPC_COUNT 32

static DaggerXL_Game *s_pGamePtr = NULL;

DaggerXL_Game::DaggerXL_Game(const XLEngine_Plugin_API *API)
{
    m_pAPI = API;

    m_nVersionMajor = 0;
    m_nVersionMinor = 10;
    m_bNPCs_Created = false;

    m_pAPI->SetConsoleColor(0.20f, 0.025f, 0.025f, 0.85f);

    m_pAPI->SetGameData("DaggerXL", m_nVersionMajor, m_nVersionMinor);
    m_pAPI->PrintToConsole("Starting DaggerXL version %d.%03d", m_nVersionMajor, m_nVersionMinor);

    //Add game specific console commands.
    s_pGame_Console = this;
    m_pAPI->RegisterConsoleCmd("g_version", (void *)CC_GameVersion, Console::CTYPE_FUNCTION, "Prints out the name and version of the currently loaded game.", this);

    m_pAPI->RegisterScriptFunc("void Game_NewGame()", asFUNCTION(SC_Game_NewGame)); 

    //We only worry about the UI scripts, palettes, color maps and so on if we're running a client.
    if ( m_pAPI->IsServer() == false )
    {
        //Load the WorldMap.
        m_pAPI->LoadWorldMap();

        //Start the UI script for title screen.
        m_pAPI->Start_UI_Script( "DaggerXL/CoreUI.as" );
    
        //load the game palette so that the UI is correct.
        LoadPals();
    }

    m_pAPI->World_CreateTerrain(1000, 500);

    //m_Player = xlNew DaggerXL_Player(API);
    m_DoorLogic = xlNew Logic_Door(API);
    m_ObjActionLogic = xlNew Logic_Obj_Action(API);
    m_NPC_Logic = xlNew Logic_NPC(API);
    m_Player = xlNew DaggerXL_Player(API);

    m_pAPI->World_SetUpdateCallback( DaggerXL_Game::WorldUpdate, NULL );

    s_pGamePtr = this;
}

DaggerXL_Game::~DaggerXL_Game(void)
{
    if ( m_Player )
    {
        xlDelete m_Player;
        m_Player = NULL;
    }
    if ( m_DoorLogic )
    {
        xlDelete m_DoorLogic;
        m_DoorLogic = NULL;
    }
    if ( m_ObjActionLogic )
    {
        xlDelete m_ObjActionLogic;
        m_ObjActionLogic = NULL;
    }
    if ( m_NPC_Logic )
    {
        xlDelete m_NPC_Logic;
        m_NPC_Logic = NULL;
    }
    for (uint32_t n=0; n<NPC_COUNT; n++)
    {
        xlDelete m_NPC_List[n];
    }
}

void DaggerXL_Game::FixedUpdate()
{
}

void DaggerXL_Game::VariableUpdate(float dt)
{
}

void DaggerXL_Game::PreRender(float dt)
{
}

void DaggerXL_Game::PostRender(float dt)
{
}

//385 - 394

void DaggerXL_Game::KeyDown(int32_t key)
{
    m_Player->KeyDown(key);

    //test
    if ( key == XL_N )
    {
        PlaceNPC();
    }
}

void DaggerXL_Game::CreateNPCs()
{
    if ( m_bNPCs_Created )
        return;

    for (uint32_t n=0; n<NPC_COUNT; n++)
    {
        m_NPC_List[n] = new NPC(m_pAPI);
    }

    m_bNPCs_Created = true;
}

void DaggerXL_Game::WorldUpdate(int32_t newWorldX, int32_t newWorldY, XLEngine_Plugin_API *API, void *pUserData)
{
    s_pGamePtr->CreateNPCs();

    //first remove NPCs that are out of range.
    for (uint32_t n=0; n<NPC_COUNT; n++)
    {
        NPC *pNPC = s_pGamePtr->m_NPC_List[n];
        if ( pNPC->IsEnabled() )
        {
            int32_t wx, wy;
            pNPC->GetWorldPos(s_pGamePtr->m_pAPI, wx, wy);

            int32_t dx = wx - newWorldX;
            int32_t dy = wy - newWorldY;

            if ( dx < 0 ) dx = -dx;
            if ( dy < 0 ) dy = -dy;

            if ( dx > 1 || dy > 1 )
            {
                pNPC->Enable(s_pGamePtr->m_pAPI, false);
            }
        }
    }

    //then add in new NPCs that are in range.
    for (uint32_t n=0; n<NPC_COUNT; n++)
    {
        NPC *pNPC = s_pGamePtr->m_NPC_List[n];
        if ( !pNPC->IsEnabled() )
        {
            s_pGamePtr->PlaceNPC(n);
        }
    }
}

bool DaggerXL_Game::PlaceNPC(int32_t newNPC)
{
    bool bNPC_Added = false;
    //find next inactive NPC.
    if ( newNPC == -1 )
    {
        for (int32_t n=0; n<NPC_COUNT; n++)
        {
            if ( !m_NPC_List[n]->IsEnabled() )
            {
                newNPC = n;
                break;
            }
        }
    }

    if ( newNPC > -1 )
    {
        int32_t nodeX, nodeY;
        float x, y, z;
        int32_t sx, sy;
        if ( m_pAPI->Pathing_GetRandomNode(nodeX, nodeY, x, y, z, sx, sy) )
        {
            float fAngle = (float)(rand()%360) * 0.01745329252f;
            float dirX   = cosf(fAngle);
            float dirY   = sinf(fAngle);

            NPC *pNPC = m_NPC_List[newNPC];
            pNPC->Reset(m_pAPI, 385 + (rand()%9), x, y, z, sx, sy, dirX, dirY);
            pNPC->Enable(m_pAPI, true);
        }

        bNPC_Added = true;
    }

    return bNPC_Added;
}

void DaggerXL_Game::NewGame()
{
    //m_pAPI->World_UnloadAllCells();
    //m_pAPI->World_LoadCell( CELLTYPE_DAGGERFALL, ARCHIVETYPE_BSA, "", "Privateer's Hold", 0, 0 );
    //m_pAPI->World_LoadCell( CELLTYPE_DAGGERFALL, ARCHIVETYPE_BSA, "", "Daggerfall", 0, 0 );
    //m_pAPI->World_LoadCell( CELLTYPE_DAGGERFALL, ARCHIVETYPE_BSA, "", "Ruins of Copperhart Orchard", 0, 0 );
    //one new game only, set the proper start location.
    //m_Player->SetPos( Vector3(120.0f, 430.0f, 8.6f) );
}

/************************
 *** Script Commands  ***
 ************************/
void DaggerXL_Game::SC_Game_NewGame()
{
    s_pGame_Console->NewGame();
}

/************************
 *** Console commands ***
 ************************/
void DaggerXL_Game::CC_GameVersion(const std::vector<std::string>& args, void *pUserData)
{
    s_pGame_Console->m_pAPI->PrintToConsole("DaggerXL version %d.%03d", s_pGame_Console->m_nVersionMajor, s_pGame_Console->m_nVersionMinor);
}

/************************
 ******* Palettes *******
 ************************/
enum
{
    PAL_MAP=0,
    PAL_OLDMAP,
    PAL_OLDPAL,
    PAL_ART,
    PAL_DANKBMAP,
    PAL_FMAP,
    PAL_NIGHTSKY,
    PAL_PAL,
    PAL_COUNT
};

const char *_apszPalFiles[]=
{
    "Map.Pal",
    "OldMap.Pal",
    "OldPal.Pal"
};
int _Pal_Count = 3;

const char *_apszColFiles[]=
{
    "Art_Pal.Col",
    "DankBmap.Col",
    "Fmap_Pal.Col",
    "NightSky.Col",
    "Pal.Pal",
};
int _Col_Count = 5;
int _Num_Colormap_Levels = 64;

void DaggerXL_Game::LoadPal(struct Color *pPalData, const char *pszFile)
{
    memset(pPalData, 0, sizeof(Color)*256);
    if ( m_pAPI->SysFile_Open(pszFile) )
    {
        uint32_t len = m_pAPI->SysFile_GetLength();
        m_pAPI->SysFile_Read(pPalData, 0, len);
        m_pAPI->SysFile_Close();
    }
}

void DaggerXL_Game::LoadCol(Color *pPalData, const char *pszFile)
{
    int sizeOfColor = sizeof(Color);
    memset(pPalData, 0, sizeOfColor*256);
    if ( m_pAPI->SysFile_Open(pszFile) )
    {
        uint32_t len = m_pAPI->SysFile_GetLength();
        m_pAPI->SysFile_Read(pPalData, sizeof(ColHeader), len-sizeof(ColHeader));
        m_pAPI->SysFile_Close();
    }
}

void DaggerXL_Game::LoadColormap(uint8_t *pColormap, const char *pszFile)
{
    memset(pColormap, 0, 256*_Num_Colormap_Levels);
    if ( m_pAPI->SysFile_Open(pszFile) )
    {
        uint32_t len = m_pAPI->SysFile_GetLength();
        m_pAPI->SysFile_Read(pColormap, 0, len);
        m_pAPI->SysFile_Close();
    }
}

void DaggerXL_Game::ClearColorToColor(uint8_t *pColormap, uint32_t color, uint32_t newColor)
{
    for (int32_t nLevel=0; nLevel<_Num_Colormap_Levels; nLevel++)
    {
        pColormap[ (nLevel<<8)+color ] = pColormap[ (nLevel<<8)+newColor ];
    }
}

void DaggerXL_Game::LoadPals()
{
    uint8_t *new_pal    = xlNew uint8_t[768*3+1];
    uint8_t *new_colmap = xlNew uint8_t[256*_Num_Colormap_Levels];

    for (int i=0; i<_Pal_Count; i++)
    {
        LoadPal( (Color *)new_pal, _apszPalFiles[i] );
        m_pAPI->SetGamePalette( i, new_pal, 768, 0 );
    }
    for (int i=0; i<_Col_Count; i++)
    {
        LoadCol( (Color *)new_pal, _apszColFiles[i] );
        m_pAPI->SetGamePalette( i+_Pal_Count, new_pal, 768, 0 );
    }
    LoadColormap( new_colmap, "SHADE.000" );
    ClearColorToColor(new_colmap, 0, 223);
    m_pAPI->SetColormap(0, new_colmap, _Num_Colormap_Levels);
    LoadColormap( new_colmap, "SHADE.001" );
    ClearColorToColor(new_colmap, 0, 223);
    m_pAPI->SetColormap(1, new_colmap, _Num_Colormap_Levels);
    LoadColormap( new_colmap, "HAZE.000" );
    ClearColorToColor(new_colmap, 0, 223);
    m_pAPI->SetColormap(2, new_colmap, _Num_Colormap_Levels);
    LoadColormap( new_colmap, "HAZE.001" );
    ClearColorToColor(new_colmap, 0, 223);
    m_pAPI->SetColormap(3, new_colmap, _Num_Colormap_Levels);

    xlDelete [] new_pal;
    xlDelete [] new_colmap;
}

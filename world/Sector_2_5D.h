#ifndef SECTOR_2_5DH
#define SECTOR_2_5DH

#include "Sector.h"
#include "../math/Vector2.h"
#include "../math/Vector3.h"
#include "../render/Camera.h"
#include "../world/LevelFunc.h"

/*********************************************
2.5D Sector.
Current limits (per sector):
	Vertex Count: 65536
	WallCount: 65535
Limits per level:
	Total Sector Count: 65535
 *********************************************/

//SOLID_WALL = no mirror or adjoin, this is a solid wall (the default).
#define SOLID_WALL 0xffff
#define MAX_ADJOIN_COUNT 2

class IDriver3D;
class Object;
class WorldCell;

class Wall
{
public:
	enum WallTextures_e
	{
		WALL_TEX_MID=0,
		WALL_TEX_TOP,
		WALL_TEX_BOT,
		WALL_TEX_SIGN,
		WALL_TEX_COUNT
	};

	enum WallFlags
	{
		WALL_FLAGS_NONE = 0,
		WALL_FLAGS_SOLIDTEX = (1<<0),	//draw a solid texture regardless of connectivity.
		WALL_FLAGS_XFLIP = (1<<1),		//texture: flip on x.
		WALL_FLAGS_YFLIP = (1<<2),
		WALL_FLAGS_MASKWALL = (1<<3),
		WALL_FLAGS_TRANS = (1<<4),
		WALL_FLAGS_SKY   = (1<<5),
		WALL_FLAGS_MORPH = (1<<6),
		WALL_FLAGS_INV_MORPH = (1<<7),
	};
public:
	uint16_t m_idx[2];					//vertex indices.
	uint16_t m_adjoin[MAX_ADJOIN_COUNT];	//dual-adjoin support
	uint16_t m_mirror[MAX_ADJOIN_COUNT];	//
	
	uint32_t m_flags;					//wall flags
	uint32_t m_mapFlags;					//map flags (used by the automap)
	int16_t m_lightDelta;
	TextureHandle m_textures[WALL_TEX_COUNT];

	Vector2 m_texOffset[WALL_TEX_COUNT];
	Vector2 m_texScale[WALL_TEX_COUNT];

	f32 m_wallLen;					//used for computing texture coordinates.

	LevelFunc *m_pFunc;
};

class Sector_2_5D : public Sector
{
public:
	Sector_2_5D();
	~Sector_2_5D();

	void Render(IDriver3D *pDriver, Camera *pCamera);
	bool PointInsideSector(f32 x, f32 y);
	f32 GetZ_Floor(f32 x, f32 y, const vector<Sector *>& Sectors);
	f32 GetZ_Ceil(f32 x, f32 y, const vector<Sector *>& Sectors);

	static void RenderSectors(IDriver3D *pDriver, WorldCell *pCell, Camera *pCamera, Sector_2_5D *pStart, const vector<Sector *>& Sectors);
	static void Collide(Vector3 *p0, Vector3 *p1, uint32_t& uSector, f32 fRadius, const vector<Sector *>& Sectors, bool bPassThruAdjoins);
	static void RayCastAndActivate(Vector3 *p0, Vector3 *p1, uint32_t& uSector, const vector<Sector *>& Sectors);
public:
	enum
	{
		SEC_FLAGS_NONE		  = 0,
		SEC_FLAGS_EXTERIOR	  = (1<<0),
		SEC_FLAGS_FLOOR_SLOPE = (1<<1),
		SEC_FLAGS_CEIL_SLOPE  = (1<<2),
		SEC_FLAGS_FLOOR_FLIP  = (1<<3),
		SEC_FLAGS_CEIL_FLIP   = (1<<4),
		SEC_FLAGS_FLOORWATER  = (1<<5),
		SEC_FLAGS_UNDERWATER  = (1<<6),
		SEC_FLAGS_SKYFLOOR    = (1<<7),
		SEC_FLAGS_ALLOW_NONSOLID_ACTIVATE = (1<<8),	//activate walls even if they aren't solid.
	};

	//A sector can currently have up to 65536 vertices.
	uint16_t m_uVertexCount;
	uint16_t m_uWallCount;
	uint32_t m_uFlags;

	uint8_t m_uLayer;
	uint8_t m_uAmbientFloor;
	uint8_t m_uAmbientCeil;
	uint8_t m_pad[2];
	
	Vector2 *m_pVertexBase;
	Vector2 *m_pVertexCur;
	Vector2 m_ZRangeBase, m_ZRangeCur;
	Vector2 m_FloorTexScale;
	Vector2 m_CeilTexScale;
	f32 m_fFloorSlope;
	f32 m_fCeilSlope;
	int32_t m_aLightFX[3];	//floor, ceiling, walls.
	uint16_t m_auSlopeSector[2];
	uint16_t m_auSlopeAnchor[2];
	Vector2 m_texOffset[2];

	Wall *m_Walls;
	uint16_t m_vAdjoin[2];					//vertical adjoins [top/bottom].
	Vector3 m_vAdjOffset[2];

	TextureHandle m_hFloorTex;
	TextureHandle m_hCeilTex;

	LevelFunc *m_pFunc;

protected:
	struct VisStack
	{
		Vector2 fL, fR;
		Vector3 offset;
		uint32_t uStartX, uEndX;
		bool bUsePortalClip;
		Sector_2_5D *pNext;
		f32 *depth;	//ignored by primary sectors.
	};

	static Vector2 m_nearPlane[2];
	static Vector3 s_vCurrentOffs;
	static bool m_bUpdateVis;

	static VisStack m_visStack[];
	static VisStack m_visStack_VAdjoin[];
	static int32_t m_visStackCnt;
	static int32_t m_visStackIdx;
	static int32_t m_visStackCnt_VAdjoin;
	static f32 s_fFogRange;

	static Camera m_Camera2D;
	
	static void VisStack_Push(Vector2& fL, Vector2& fR, uint32_t uStartX, uint32_t uEndX, Sector_2_5D *pNext, bool bUsePortalClip=true, Vector3& vOffset=s_vCurrentOffs);
	static VisStack *VisStack_Pop();
	static void VisStack_Clear();
	static void Visibility2D(const Vector3& cPos, Vector2 fL, Vector2 fR, uint32_t uStartX, uint32_t uEndX, Sector_2_5D *pCurSec, const vector<Sector *>& Sectors, bool bUsePortalClip, IDriver3D *pDriver);
	static void WallRasterizer(const Vector3& cPos, uint32_t uStartX, uint32_t uEndX, Sector_2_5D *pCurSec, const vector<Sector *>& Sectors);

	static void _DrawWall(IDriver3D *pDriver, Sector_2_5D *pCurSec, Sector_2_5D *pNextSec, Sector_2_5D *pBotSec, Sector_2_5D *pTopSec, uint16_t w, Vector2 *worldPos, const vector<Sector *>& Sectors);
	static void _DrawFloor(IDriver3D *pDriver, Sector_2_5D *pCurSec, const Vector2 *worldPos, const Vector2& a, const Vector2& n0, const Vector2& n1, const vector<Sector *>& Sectors);
	static void _SetupCameraParameters(const Vector3& cPos, const Vector3& cDir, Vector2 fL, Vector2 fR);

	static void _AddSectorToList(int32_t s, Vector3 *p0, Vector2& vPathMin, Vector2& vPathMax, const vector<Sector *>& Sectors);
	static void AddObjectToRender(Object *pObj, f32 fIntensity, const Vector3& vOffs);
	static void RenderObjects(IDriver3D *pDriver);

	static void RenderSky(IDriver3D *pDriver, WorldCell *pCell);
};

#endif //SECTOR_2_5DH

#ifndef TERRAIN_H
#define TERRAIN_H

#include "../CommonTypes.h"
#include "../math/Vector3.h"
#include "../math/Vector4.h"
#include "../render/IDriver3D.h"
#include "../render/VertexBuffer.h"
#include "../render/IndexBuffer.h"
#include <vector>
#include <map>

using namespace std;

class IDriver3D;
class Camera;
class SkyLoader;
class Location_Daggerfall;
class Sector_GeoBlock;
class WorldCell;
class World;
struct CollisionPacket;
struct RaycastPacket;

#define INVALID_SECTOR 0xffff
#define LOC_KEY(x,y) (u32)(y*1000+x)

#define TERRAIN_DESERT		 0
#define TERRAIN_MOUNTAIN   100
#define TERRAIN_TEMPERATE  300
#define TERRAIN_SWAMP	   400
#define TERRAIN_NONE	  1000

struct LocationRect
{
	Location_Daggerfall *pLoc;
	Vector4 vRectInner;
	Vector4 vRectOuter;
	float fHeight;
};

class Terrain
{
public:
	Terrain(IDriver3D *pDriver, World *pWorld);
	~Terrain();

	//Add a dynamic object to the terrain Sector.
	//This is done for objects that are fully dynamic and do not belong to specific cells.
	void AddDynamicObject(u32 uID);

	void SetHeightmap(s32 width, s32 height, u8 *pData, float fScale, float fBias);

	//Whether the terrain is currently active or not.
	//If its not active, then no rendering or collision takes place.
	void Activate(bool bActive);
	bool IsActive() { return m_bActive; }

	//Update the terrain mesh and texturing based on the world position.
	bool Update(s32 x, s32 y, s32 nRectCnt=0, LocationRect *pRects=NULL);

	//Render the terrain.
	void Render(Camera *pCamera);

	//Get the terrain height from the current position.
	float GetHeight(float xPos, float yPos);

	//Get the terrain height at the current "map scale"
	float GetHeight_MapScale(s32 x, s32 y);

	//Get the climate for a map cell.
	u32 GetClimate_MapScale(s32 x, s32 y);

	//Is the current terrain point in water?
	bool IsPointInWater(float xPos, float yPos);

public:
	enum
	{
		CLIMATE_TEMPERATE=0,
		CLIMATE_MOUNTAIN,
		CLIMATE_DESERT,
		CLIMATE_SWAMP,
		CLIMATE_WATER,
		CLIMATE_COUNT
	};

private:
	void RenderLOD(Camera *pCamera, s32 lod);
	void RenderChunk(Camera *pCamera, s32 lod, s32 chunkNum);
	void BuildTerrainMeshes();
	void BuildHeightmap(s32 newX, s32 newY, s32 prevX, s32 prevY, s32 nRectCnt, LocationRect *pRects);
	void Animate();
	void GenTextureTileMapping();
	float SampleBaseHeightmap(int lod, int x, int y);
	float SampleCoastalDistance(int x, int y);
	void ComputeNormal(s32 x, s32 y);
	s32 GetClimate(int x, int y, int *pnFlat=NULL);
	s32 GetSkyIndex(int x, int y);

	void RenderSky(s32 skyIndex, s32 timeIndex, Camera *pCamera);
	
	void LoadHeightmap();
	void BinLocations();
	void FilterHeightMap(u8 *pAltMap, float *pAltMapF, float *pCoastalDist);

	enum
	{
		CHUNK_TILE_WIDTH = 15,
		CHUNK_COUNT = CHUNK_TILE_WIDTH*CHUNK_TILE_WIDTH,
		TILE_QUAD_WIDTH = 16,
		TILE_QUAD_COUNT = TILE_QUAD_WIDTH*TILE_QUAD_WIDTH,
		LOD_COUNT = 2,
	};

	IDriver3D *m_pDriver;
	bool m_bActive;
	bool m_bMeshBuilt;

	s32 m_nWidth;
	s32 m_nHeight;
	s32 m_x;
	s32 m_y;
	float *m_afHeightmap;
	float *m_afCoastalDist;
	u8 *m_pClimate;
	Sector_GeoBlock *m_pSector;
	WorldCell *m_pWorldCell;
	World *m_pWorld;

	struct AABB
	{
		Vector3 vMin;
		Vector3 vMax;
		Vector3 vCen;
	};

	struct Chunk
	{
		IndexBuffer *m_pChunkIB;
		u16 m_TileTexArray[TILE_QUAD_COUNT];
		AABB m_aChunkBounds;
		float dist;
	};

	struct LOD
	{
		float *m_pLocalHM;
		float *m_pHM_Flags;
		Vector3 *m_pLocalNM;

		VertexBuffer *m_pVB;
		IndexBuffer  *m_pGlobalIB;
		Chunk		  m_aChunks[CHUNK_COUNT];
	};

	typedef map<u32, Location_Daggerfall *> LocationMap;

	LOD m_LOD[2];
	vector<Chunk *> m_ChunkRenderList;
	LocationMap m_LocationMap;
	SkyLoader *m_pSkyLoader;

	inline Location_Daggerfall *FindLocation(s32 x, s32 y)
	{
		LocationMap::iterator iLoc = m_LocationMap.find( LOC_KEY(x, y) );
		Location_Daggerfall *pLoc = NULL;
		if ( iLoc != m_LocationMap.end() )
		{
			pLoc = iLoc->second;
		}
		return pLoc;
	}
	
	static bool SortCB_Chunks(Chunk*& d1, Chunk*& d2);

	static Vector3 m_vCamDir, m_vCamLoc;
	static const int c_numVertexEdge;
	static const float c_fTotalWidth;
	static const float c_fTotalHeight;
	static const Vector3 c_startPos;
	static s32 s_anMapClimate[];
	static s32 s_anMapFlat[];
};

#endif //TERRAIN_H

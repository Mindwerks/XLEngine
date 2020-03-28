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
#define LOC_KEY(x,y) (uint32_t)(y*1000+x)

#define TERRAIN_DESERT       0
#define TERRAIN_MOUNTAIN   100
#define TERRAIN_TEMPERATE  300
#define TERRAIN_SWAMP      400
#define TERRAIN_NONE      1000

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
    void AddDynamicObject(uint32_t uID);

    void SetHeightmap(int32_t width, int32_t height, uint8_t *pData, float fScale, float fBias);

    //Whether the terrain is currently active or not.
    //If its not active, then no rendering or collision takes place.
    void Activate(bool bActive);
    bool IsActive() { return m_bActive; }

    //Update the terrain mesh and texturing based on the world position.
    bool Update(int32_t x, int32_t y, int32_t nRectCnt=0, LocationRect *pRects=nullptr);

    //Render the terrain.
    void Render(Camera *pCamera);

    //Get the terrain height from the current position.
    float GetHeight(float xPos, float yPos);

    //Get the terrain height at the current "map scale"
    float GetHeight_MapScale(int32_t x, int32_t y);

    //Get the climate for a map cell.
    uint32_t GetClimate_MapScale(int32_t x, int32_t y);

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
    void RenderLOD(Camera *pCamera, int32_t lod);
    void RenderChunk(Camera *pCamera, int32_t lod, int32_t chunkNum);
    void BuildTerrainMeshes();
    void BuildHeightmap(int32_t newX, int32_t newY, int32_t prevX, int32_t prevY, int32_t nRectCnt, LocationRect *pRects);
    void Animate();
    void GenTextureTileMapping();
    float SampleBaseHeightmap(int lod, int x, int y);
    float SampleCoastalDistance(int x, int y);
    void ComputeNormal(int32_t x, int32_t y);
    int32_t GetClimate(int x, int y, int *pnFlat=nullptr);
    int32_t GetSkyIndex(int x, int y);

    void RenderSky(int32_t skyIndex, int32_t timeIndex, Camera *pCamera);

    void LoadHeightmap();
    void BinLocations();
    void FilterHeightMap(uint8_t *pAltMap, float *pAltMapF, float *pCoastalDist);

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

    int32_t m_nWidth;
    int32_t m_nHeight;
    int32_t m_x;
    int32_t m_y;
    float *m_afHeightmap;
    float *m_afCoastalDist;
    uint8_t *m_pClimate;
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
        uint16_t m_TileTexArray[TILE_QUAD_COUNT];
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
        Chunk         m_aChunks[CHUNK_COUNT];
    };

    typedef std::map<uint32_t, Location_Daggerfall *> LocationMap;

    LOD m_LOD[2];
    std::vector<Chunk *> m_ChunkRenderList;
    LocationMap m_LocationMap;
    SkyLoader *m_pSkyLoader;

    inline Location_Daggerfall *FindLocation(int32_t x, int32_t y)
    {
        LocationMap::iterator iLoc = m_LocationMap.find( LOC_KEY(x, y) );
        Location_Daggerfall *pLoc = nullptr;
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
    static int32_t s_anMapClimate[];
    static int32_t s_anMapFlat[];


    TextureHandle *m_pTexArray;
    uint16_t *m_pTexIndex;
};

#endif //TERRAIN_H

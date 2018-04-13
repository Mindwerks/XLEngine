#ifndef LOCATION_DAGGERFALL_H
#define LOCATION_DAGGERFALL_H

#include "../CommonTypes.h"
#include <memory>
#include <string>
#include <map>

class Location_Daggerfall;
class WorldCell;

using NameLocationMap = std::map<std::string, Location_Daggerfall*>;
using LocationMap = std::map<uint64_t, Location_Daggerfall*>;

class Location_Daggerfall
{
public:
    Location_Daggerfall();
    ~Location_Daggerfall();

    void LoadLoc(const char *pData, int index, const int RegIdx, LocationMap &mapLoc, NameLocationMap &mapNames);

public:
    struct LocName
    {
        char szName[32];
    };

    struct DungeonBlock
    {
        char szName[16];
        int16_t  x, y;
    };

    char  m_szName[32];
    float m_x;
    float m_y;
    int32_t   m_OrigX;
    int32_t   m_OrigY;
    int32_t   m_waterHeight;
    int32_t   m_Long;
    int32_t   m_Lat;
    uint32_t   m_LocationID;
    int32_t   m_BlockWidth;
    int32_t   m_BlockHeight;
    int16_t   m_locType;
    int16_t   m_locCat;
    bool  m_bLoaded;

    std::unique_ptr<LocName[]> m_pBlockNames;
    //dungeon data...
    int16_t m_dungeonBlockCnt;
    int16_t m_startDungeonBlock;    //start point.
    std::unique_ptr<DungeonBlock[]> m_pDungeonBlocks;

    //filled when loaded.
    std::unique_ptr<uint8_t[]> m_pTexData;
};

class Region_Daggerfall
{
public:
    Region_Daggerfall();
    ~Region_Daggerfall();

public:
    uint32_t m_uLocationCount;
    std::unique_ptr<Location_Daggerfall[]> m_pLocations;
};

class WorldMap
{
public:
    static void Init();
    static void Destroy();

    //load data from disk.
    static bool Load();

    static Location_Daggerfall *GetLocation(int32_t x, int32_t y);
    static Location_Daggerfall *GetLocation(const char *pszName);

    static void SetWorldCell(int32_t x, int32_t y, WorldCell *pCell);
    static WorldCell *GetWorldCell(int32_t x, int32_t y);
        
private:
    //generate the cached data.
    static bool Cache();

public:
    static uint32_t m_uRegionCount;
    static std::unique_ptr<Region_Daggerfall[]> m_pRegions;
    static LocationMap m_MapLoc;
    static std::map<uint64_t, WorldCell *> m_MapCell;
    static NameLocationMap m_MapNames;
    static bool m_bMapLoaded;
};

#endif //LOCATION_DAGGERFALL_H

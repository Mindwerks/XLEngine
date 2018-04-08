#ifndef LOCATION_DAGGERFALL_H
#define LOCATION_DAGGERFALL_H

#include "../CommonTypes.h"
#include <string>
#include <map>

using namespace std;

class WorldCell;

class Location_Daggerfall
{
public:
    Location_Daggerfall();
    ~Location_Daggerfall();

    //Save and Load cached data.
    void Save(FILE *f);
    bool Load(FILE *f, map<uint64_t, Location_Daggerfall *>& mapLoc, map<string, Location_Daggerfall *>& mapNames);

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

    LocName *m_pBlockNames;
    //dungeon data...
    int16_t m_dungeonBlockCnt;
    int16_t m_startDungeonBlock;    //start point.
    DungeonBlock *m_pDungeonBlocks;

    //filled when loaded.
    uint8_t *m_pTexData;
};

class Region_Daggerfall
{
public:
    Region_Daggerfall();
    ~Region_Daggerfall();

    //Save and load cached data.
    void Save(FILE *f);
    bool Load(FILE *f, map<uint64_t, Location_Daggerfall *>& mapLoc, map<string, Location_Daggerfall *>& mapNames);

public:
    uint32_t m_uLocationCount;
    Location_Daggerfall *m_pLocations;
};

class WorldMap
{
public:
    static void Init();
    static void Destroy();

    //load cached data from disk if present.
    static bool Load();

    static Location_Daggerfall *GetLocation(int32_t x, int32_t y);
    static Location_Daggerfall *GetLocation(const char *pszName);

    static void SetWorldCell(int32_t x, int32_t y, WorldCell *pCell);
    static WorldCell *GetWorldCell(int32_t x, int32_t y);
        
private:
    //generate the cached data.
    static bool Cache();
    //save cache data to disk.
    static void Save();

public:
    static uint32_t m_uRegionCount;
    static Region_Daggerfall *m_pRegions;
    static map<uint64_t, Location_Daggerfall *> m_MapLoc;
    static map<uint64_t, WorldCell *> m_MapCell;
    static map<string, Location_Daggerfall *> m_MapNames;
    static bool m_bMapLoaded;
};

#endif //LOCATION_DAGGERFALL_H
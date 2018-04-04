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
	bool Load(FILE *f, map<u64, Location_Daggerfall *>& mapLoc, map<string, Location_Daggerfall *>& mapNames);

public:
	struct LocName
	{
		char szName[32];
	};

	struct DungeonBlock
	{
		char szName[16];
		s16  x, y;
	};

	char  m_szName[32];
	float m_x;
	float m_y;
	s32   m_OrigX;
	s32   m_OrigY;
	s32   m_waterHeight;
	s32   m_Long;
	s32   m_Lat;
	u32   m_LocationID;
	s32   m_BlockWidth;
	s32   m_BlockHeight;
	s16   m_locType;
	s16   m_locCat;
	bool  m_bLoaded;

	LocName *m_pBlockNames;
	//dungeon data...
	s16 m_dungeonBlockCnt;
	s16 m_startDungeonBlock;	//start point.
	DungeonBlock *m_pDungeonBlocks;

	//filled when loaded.
	u8 *m_pTexData;
};

class Region_Daggerfall
{
public:
	Region_Daggerfall();
	~Region_Daggerfall();

	//Save and load cached data.
	void Save(FILE *f);
	bool Load(FILE *f, map<u64, Location_Daggerfall *>& mapLoc, map<string, Location_Daggerfall *>& mapNames);

public:
	u32 m_uLocationCount;
	Location_Daggerfall *m_pLocations;
};

class WorldMap
{
public:
	static void Init();
	static void Destroy();

	//load cached data from disk if present.
	static bool Load();

	static Location_Daggerfall *GetLocation(s32 x, s32 y);
	static Location_Daggerfall *GetLocation(const char *pszName);

	static void SetWorldCell(s32 x, s32 y, WorldCell *pCell);
	static WorldCell *GetWorldCell(s32 x, s32 y);
		
private:
	//generate the cached data.
	static bool Cache();
	//save cache data to disk.
	static void Save();

public:
	static u32 m_uRegionCount;
	static Region_Daggerfall *m_pRegions;
	static map<u64, Location_Daggerfall *> m_MapLoc;
	static map<u64, WorldCell *> m_MapCell;
	static map<string, Location_Daggerfall *> m_MapNames;
	static bool m_bMapLoaded;
};

#endif //LOCATION_DAGGERFALL_H
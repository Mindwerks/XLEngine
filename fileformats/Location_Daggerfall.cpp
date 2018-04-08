#include "Location_Daggerfall.h"
#include "../EngineSettings.h"
#include "../fileformats/ArchiveTypes.h"
#include "../fileformats/ArchiveManager.h"
#include "../memory/ScratchPad.h"
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <assert.h>

#define CACHED_FILE_VERSION 0x01

uint32_t WorldMap::m_uRegionCount;
Region_Daggerfall *WorldMap::m_pRegions;
map<uint64_t, Location_Daggerfall *> WorldMap::m_MapLoc;
map<uint64_t, WorldCell *> WorldMap::m_MapCell;
map<string, Location_Daggerfall *> WorldMap::m_MapNames;
bool WorldMap::m_bMapLoaded = false;

///////////////////////////////////////////////////////
// Location
///////////////////////////////////////////////////////

Location_Daggerfall::Location_Daggerfall()
{
    m_bLoaded           = false;
    m_pBlockNames       = NULL;
    m_pDungeonBlocks    = NULL;
    m_pTexData          = NULL;
    m_dungeonBlockCnt   = 0;
    m_startDungeonBlock = 0;
}

Location_Daggerfall::~Location_Daggerfall()
{
    SafeDeleteArr(m_pBlockNames);
    SafeDeleteArr(m_pDungeonBlocks);
    SafeDeleteArr(m_pTexData);
}

//Save and Load cached data.
void Location_Daggerfall::Save(FILE *f)
{
    fwrite(m_szName,       1, 32, f);
    fwrite(&m_x,           1, sizeof(float), f);
    fwrite(&m_y,           1, sizeof(float), f);
    fwrite(&m_OrigX,       1, sizeof(int32_t), f);
    fwrite(&m_OrigY,       1, sizeof(int32_t), f);
    fwrite(&m_waterHeight, 1, sizeof(int32_t), f);
    fwrite(&m_Long,        1, sizeof(int32_t), f);
    fwrite(&m_Lat,         1, sizeof(int32_t), f);
    fwrite(&m_LocationID,  1, sizeof(uint32_t), f);
    fwrite(&m_BlockWidth,  1, sizeof(int32_t), f);
    fwrite(&m_BlockHeight, 1, sizeof(int32_t), f);
    fwrite(&m_locType,     1, sizeof(int16_t), f);
    fwrite(&m_locCat,      1, sizeof(int16_t), f);

    for (int b=0; b<m_BlockWidth*m_BlockHeight; b++)
    {
        fwrite(m_pBlockNames[b].szName, 1, 32, f);
    }

    //dungeon data...
    fwrite(&m_dungeonBlockCnt,   1, sizeof(int16_t), f);
    fwrite(&m_startDungeonBlock, 1, sizeof(int16_t), f);

    for (int b=0; b<m_dungeonBlockCnt; b++)
    {
        fwrite(m_pDungeonBlocks[b].szName, 1, 16, f);
        fwrite(&m_pDungeonBlocks[b].x, 1, sizeof(int16_t), f);
        fwrite(&m_pDungeonBlocks[b].y, 1, sizeof(int16_t), f);
    }
}

bool Location_Daggerfall::Load(FILE *f, map<uint64_t, Location_Daggerfall *>& mapLoc, map<string, Location_Daggerfall *>& mapNames)
{
    fread(m_szName,       1, 32, f);
    fread(&m_x,           1, sizeof(float), f);
    fread(&m_y,           1, sizeof(float), f);
    fread(&m_OrigX,       1, sizeof(int32_t), f);
    fread(&m_OrigY,       1, sizeof(int32_t), f);
    fread(&m_waterHeight, 1, sizeof(int32_t), f);
    fread(&m_Long,        1, sizeof(int32_t), f);
    fread(&m_Lat,         1, sizeof(int32_t), f);
    fread(&m_LocationID,  1, sizeof(uint32_t), f);
    fread(&m_BlockWidth,  1, sizeof(int32_t), f);
    fread(&m_BlockHeight, 1, sizeof(int32_t), f);
    fread(&m_locType,     1, sizeof(int16_t), f);
    fread(&m_locCat,      1, sizeof(int16_t), f);

    m_pBlockNames = new Location_Daggerfall::LocName[m_BlockWidth*m_BlockHeight];
    for (int b=0; b<m_BlockWidth*m_BlockHeight; b++)
    {
        fread(m_pBlockNames[b].szName, 1, 32, f);
    }

    //dungeon data...
    fread(&m_dungeonBlockCnt,   1, sizeof(int16_t), f);
    fread(&m_startDungeonBlock, 1, sizeof(int16_t), f);

    m_pDungeonBlocks = new DungeonBlock[m_dungeonBlockCnt];
    for (int b=0; b<m_dungeonBlockCnt; b++)
    {
        fread( m_pDungeonBlocks[b].szName, 1, 16, f);
        fread(&m_pDungeonBlocks[b].x, 1, sizeof(int16_t), f);
        fread(&m_pDungeonBlocks[b].y, 1, sizeof(int16_t), f);
    }

    //Add to map.
    uint64_t uKey     = (uint64_t)((int32_t)m_y>>3)<<32ULL | (uint64_t)((int32_t)m_x>>3);
    mapLoc[uKey] = this;
    mapNames[m_szName] = this;

    return true;
}


///////////////////////////////////////////////////////
// Region
///////////////////////////////////////////////////////
Region_Daggerfall::Region_Daggerfall()
{
    m_pLocations = NULL;
}

Region_Daggerfall::~Region_Daggerfall()
{
    SafeDeleteArr(m_pLocations);
}

//Save and load cached data.
void Region_Daggerfall::Save(FILE *f)
{
    fwrite(&m_uLocationCount, 1, sizeof(uint32_t), f);
    for (uint32_t l=0; l<m_uLocationCount; l++)
    {
        m_pLocations[l].Save(f);
    }
}

bool Region_Daggerfall::Load(FILE *f, map<uint64_t, Location_Daggerfall *>& mapLoc, map<string, Location_Daggerfall *>& mapNames)
{
    fread(&m_uLocationCount, 1, sizeof(uint32_t), f);
    m_pLocations = new Location_Daggerfall[ m_uLocationCount ];
    for (uint32_t l=0; l<m_uLocationCount; l++)
    {
        m_pLocations[l].Load(f, mapLoc, mapNames);
    }

    return true;
}


///////////////////////////////////////////////////////
// World Map
///////////////////////////////////////////////////////
void WorldMap::Init()
{
    m_pRegions = NULL;
}

void WorldMap::Destroy()
{
    SafeDeleteArr(m_pRegions);
}

//load cached data from disk if present.
bool WorldMap::Load()
{
    if ( m_bMapLoaded )
    {
        return true;
    }

    // TODO: Disabled until a proper cache location can be handled.
    bool bSuccess;
    FILE *f = NULL;//fopen(szCachedFile, "rb");
    if ( f )
    {
        int version;
        fread(&version, 1, sizeof(int), f);
        //if the version doesn't match, the data needs to be re-cached.
        if ( version != CACHED_FILE_VERSION )
        {
            fclose(f);
            return Cache();
        }
        fread(&m_uRegionCount, 1, sizeof(uint32_t), f);
        m_pRegions = new Region_Daggerfall[ m_uRegionCount ];

        for (uint32_t r=0; r<m_uRegionCount; r++)
        {
            m_pRegions[r].Load(f, m_MapLoc, m_MapNames);
        }

        fclose(f);
        bSuccess = true;
    }
    else
    {
        bSuccess = Cache();
    }

    m_bMapLoaded = true;
    return bSuccess;
}

//save cache data to disk.
void WorldMap::Save()
{
    // TODO: Disabled until a proper cache location can be handled.
    FILE *f = NULL;//fopen(szCachedFile, "wb");
    if ( f )
    {
        int version = CACHED_FILE_VERSION;
        fwrite(&version, 1, sizeof(int), f);
        fwrite(&m_uRegionCount, 1, sizeof(uint32_t), f);

        for (uint32_t r=0; r<m_uRegionCount; r++)
        {
            m_pRegions[r].Save(f);  
        }

        fclose(f);
    }
}

#pragma pack(push)
#pragma pack(1)

struct LocationRec
{
    int32_t oneValue;
    int16_t NullValue1;
    int8_t  NullValue2;
    int32_t XPosition;
    int32_t NullValue3;
    int32_t YPosition;
    int32_t LocType;
    int32_t Unknown2;
    int32_t Unknown3;
    int16_t oneValue2;
    uint16_t LocationID;
    int32_t NullValue4;
    int16_t Unknown4;
    int32_t Unknown5;
    char NullValue[26];
    char LocationName[32];
    char Unknowns[9];
    int16_t PostRecCount;
};

#pragma pack(pop)

const char *_aszRMBHead[]=
{
    "TVRN",
    "GENR",
    "RESI",
    "WEAP",
    "ARMR",
    "ALCH",
    "BANK",
    "BOOK",
    "CLOT",
    "FURN",
    "GEMS",
    "LIBR",
    "PAWN",
    "TEMP",
    "TEMP",
    "PALA",
    "FARM",
    "DUNG",
    "CAST",
    "MANR",
    "SHRI",
    "RUIN",
    "SHCK",
    "GRVE",
    "FILL",
    "KRAV",
    "KDRA",
    "KOWL",
    "KMOO",
    "KCAN",
    "KFLA",
    "KHOR",
    "KROS",
    "KWHE",
    "KSCA",
    "KHAW",
    "MAGE",
    "THIE",
    "DARK",
    "FIGH",
    "CUST",
    "WALL",
    "MARK",
    "SHIP",
    "WITC"
};

const char *_aszRMBTemple[]=
{
    "A0",
    "B0",
    "C0",
    "D0",
    "E0",
    "F0",
    "G0",
    "H0",
};

const char *_aszRMBChar[]=
{
    "AA",
    "AA",
    "AA",
    "AA",
    /*
    "DA",
    "DA",
    "DA",
    "DA",
    "DA",
    */
    "AA",
    "AA",
    "AA",
    "AA",
    "AA",
    //
    "AL",
    "DL",
    "AM",
    "DM",
    "AS",
    "DS",
    "AA",
    "DA",
};

const char *_aszRMBCharQ[]=
{
    "AA",
    "BA",
    "AL",
    "BL",
    "AM",
    "BM",
    "AS",
    "BS",
    "GA",
    "GL",
    "GM",
    "GS"
};

//generate the cached data.
bool WorldMap::Cache()
{
    //step 1: compute the total number of locations for the entire world...
    char szFileName[64];
    uint32_t TotalLocCount=0;
    ScratchPad::FreeFrame();
    char *pData = (char *)ScratchPad::AllocMem( 1024*1024 );

    //Load global data.
    m_uRegionCount = 62;
    m_pRegions = new Region_Daggerfall[ m_uRegionCount ];
    for (int r=0; r<62; r++)
    {
        sprintf(szFileName, "MAPNAMES.0%02d", r);
        if ( ArchiveManager::GameFile_Open(ARCHIVETYPE_BSA, "MAPS.BSA", szFileName) )
        {
            m_pRegions[r].m_uLocationCount = 0;
            m_pRegions[r].m_pLocations     = NULL;

            uint32_t uLength = ArchiveManager::GameFile_GetLength();
            if ( uLength == 0 )
            {
                ArchiveManager::GameFile_Close();
                continue;
            }

            ArchiveManager::GameFile_Read(pData, uLength);
            ArchiveManager::GameFile_Close();

            m_pRegions[r].m_uLocationCount = *((unsigned int *)pData);
            m_pRegions[r].m_pLocations     = new Location_Daggerfall[ m_pRegions[r].m_uLocationCount ];
            TotalLocCount += m_pRegions[r].m_uLocationCount;
        }
    }
    ScratchPad::FreeFrame();

    for (int r=0; r<(int)m_uRegionCount; r++)
    {
        sprintf(szFileName, "MAPTABLE.0%02d", r);
        if ( ArchiveManager::GameFile_Open(ARCHIVETYPE_BSA, "MAPS.BSA", szFileName) )
        {
            uint32_t uLength = ArchiveManager::GameFile_GetLength();
            if ( uLength == 0 )
            {   
                ArchiveManager::GameFile_Close();
                continue;
            }
            char *pData = (char *)ScratchPad::AllocMem(uLength);
            if ( pData )
            {
                ArchiveManager::GameFile_Read(pData, uLength);
                ArchiveManager::GameFile_Close();

                int index = 0;
                for (uint32_t i=0; i<m_pRegions[r].m_uLocationCount; i++)
                {
                    int mapID = *((int *)&pData[index]); index += 4;
                    uint8_t U1 = *((uint8_t *)&pData[index]); index++;
                    uint32_t longType = *((uint32_t *)&pData[index]); index += 4;
                    m_pRegions[r].m_pLocations[i].m_Lat   = ( *((short *)&pData[index]) )&0xFFFF; index += 2;
                    m_pRegions[r].m_pLocations[i].m_Long  = longType&0xFFFF;
                    uint16_t U2 = *((uint16_t *)&pData[index]); index += 2; //unknown
                    uint32_t U3 = *((uint32_t *)&pData[index]); index += 4; //unknown

                    m_pRegions[r].m_pLocations[i].m_locType = longType >> 17;

                }
            }
            ScratchPad::FreeFrame();
        }

        sprintf(szFileName, "MAPPITEM.0%02d", r);
        if ( ArchiveManager::GameFile_Open(ARCHIVETYPE_BSA, "MAPS.BSA", szFileName) )
        {
            uint32_t uLength = ArchiveManager::GameFile_GetLength();
            if ( uLength == 0 )
            {   
                ArchiveManager::GameFile_Close();
                continue;
            }
            char *pData = (char *)ScratchPad::AllocMem(uLength);
            if ( pData )
            {
                ArchiveManager::GameFile_Read(pData, uLength);
                ArchiveManager::GameFile_Close();
                
                //pData = region data.
                uint32_t *offsets = (uint32_t *)pData;
                int nLocationCount = (int)m_pRegions[r].m_uLocationCount;
                int base_index = 4*nLocationCount;
                int index;
                for (int i=0; i<nLocationCount; i++)
                {
                    index = base_index + offsets[i];
                    int nPreRecCount = *((int *)&pData[index]); index += 4;
                    uint8_t *PreRecords = NULL;
                    if ( nPreRecCount > 0 )
                    {
                        PreRecords = (uint8_t *)&pData[index]; index += nPreRecCount*6;
                    }
                    LocationRec *pLocation = (LocationRec *)&pData[index];
                    index += sizeof(LocationRec);
                    m_pRegions[r].m_pLocations[i].m_LocationID = pLocation->LocationID;
                    m_pRegions[r].m_pLocations[i].m_OrigX      = pLocation->XPosition;
                    m_pRegions[r].m_pLocations[i].m_OrigY      = pLocation->YPosition;
                    m_pRegions[r].m_pLocations[i].m_x          = (float)pLocation->XPosition / 4096.0f;
                    m_pRegions[r].m_pLocations[i].m_y          = (float)pLocation->YPosition / 4096.0f;
                    strcpy(m_pRegions[r].m_pLocations[i].m_szName, pLocation->LocationName);

                    uint64_t uKey       = (uint64_t)m_pRegions[r].m_pLocations[i].m_y<<32ULL | (uint64_t)m_pRegions[r].m_pLocations[i].m_x;
                    m_MapLoc[uKey] = &m_pRegions[r].m_pLocations[i];

                    m_pRegions[r].m_pLocations[i].m_BlockWidth  = 0;
                    m_pRegions[r].m_pLocations[i].m_BlockHeight = 0;
                    m_pRegions[r].m_pLocations[i].m_pBlockNames = NULL;

                    if ( pLocation->LocType == 0x00008000 )
                    {
                        //town or exterior location.
                        index += 5; //unknown...
                        index += 26*pLocation->PostRecCount;    //post records.
                        index += 32;    //"Another name"
                        int locID2 = *((int *)&pData[index]); index += 4;
                        index += 4; //unknowns
                        char BlockWidth  = pData[index]; index++;
                        char BlockHeight = pData[index]; index++;
                        index += 7; //unknowns
                        char *BlockFileIndex  = &pData[index]; index += 64;
                        char *BlockFileNumber = &pData[index]; index += 64;
                        uint8_t *BlockFileChar = (uint8_t *)&pData[index]; index += 64;
                        m_pRegions[r].m_pLocations[i].m_BlockWidth  = BlockWidth;
                        m_pRegions[r].m_pLocations[i].m_BlockHeight = BlockHeight;
                        m_pRegions[r].m_pLocations[i].m_pBlockNames = new Location_Daggerfall::LocName[BlockWidth*BlockHeight];
                        char szName[64];
                        for (int by=0; by<BlockHeight; by++)
                        {
                            for (int bx=0; bx<BlockWidth; bx++)
                            {
                                int bidx = by*BlockWidth + bx;
                                if ( BlockFileIndex[bidx] == 13 || BlockFileIndex[bidx] == 14 )
                                {
                                    if ( BlockFileChar[bidx] > 0x07 )
                                    {
                                        sprintf(szName, "%s%s%s.RMB", _aszRMBHead[ BlockFileIndex[bidx] ], "GA", _aszRMBTemple[BlockFileNumber[bidx]&0x07]);
                                    }
                                    else
                                    {
                                        sprintf(szName, "%s%s%s.RMB", _aszRMBHead[ BlockFileIndex[bidx] ], "AA", _aszRMBTemple[BlockFileNumber[bidx]&0x07]);
                                    }

                                    if ( !ArchiveManager::GameFile_Open(ARCHIVETYPE_BSA, "BLOCKS.BSA", szName) )
                                    {
                                        char szNameTmp[32];
                                        sprintf(szNameTmp, "%s??%s.RMB", _aszRMBHead[ BlockFileIndex[bidx] ], _aszRMBTemple[BlockFileNumber[bidx]&0x07]);
                                        ArchiveManager::GameFile_SearchForFile(ARCHIVETYPE_BSA, "BLOCKS.BSA", szNameTmp, szName);
                                    }
                                    ArchiveManager::GameFile_Close();
                                }
                                else
                                {
                                    int Q = BlockFileChar[bidx]/16;
                                    if ( BlockFileIndex[bidx] == 40 )   //"CUST"
                                    {
                                        if ( r == 20 )  //Sentinel logic
                                        {
                                            Q = 8;
                                        }
                                        else
                                        {
                                            Q = 0;
                                        }
                                    }
                                    else if ( r == 23 )
                                    {
                                        if (Q>0) Q--;
                                    }
                                    sprintf(szName, "%s%s%02d.RMB", _aszRMBHead[ BlockFileIndex[bidx] ], _aszRMBCharQ[Q], BlockFileNumber[bidx]);
                                    assert(Q < 12);
                                    //does this file exist?
                                    if ( !ArchiveManager::GameFile_Open(ARCHIVETYPE_BSA, "BLOCKS.BSA", szName) )
                                    {
                                        char szNameTmp[32];
                                        sprintf(szNameTmp, "%s??%02d.RMB", _aszRMBHead[ BlockFileIndex[bidx] ], BlockFileNumber[bidx]);
                                        ArchiveManager::GameFile_SearchForFile(ARCHIVETYPE_BSA, "BLOCKS.BSA", szNameTmp, szName);
                                    }
                                    ArchiveManager::GameFile_Close();
                                }
                                strcpy(m_pRegions[r].m_pLocations[i].m_pBlockNames[bidx].szName, szName);
                            }
                        }
                    }
                }
            }
        }
        ScratchPad::FreeFrame();

        sprintf(szFileName, "MAPDITEM.0%02d", r);
        if ( ArchiveManager::GameFile_Open(ARCHIVETYPE_BSA, "MAPS.BSA", szFileName) )
        {
            uint32_t uLength = ArchiveManager::GameFile_GetLength();
            if ( uLength == 0 )
            {   
                ArchiveManager::GameFile_Close();
                continue;
            }
            char *pData = (char *)ScratchPad::AllocMem(uLength);
            if ( pData )
            {
                ArchiveManager::GameFile_Read(pData, uLength);
                ArchiveManager::GameFile_Close();

                //pData = region data.
                int index = 0;
                uint32_t count = *((uint32_t*)&pData[index]); index += 4;
                struct DungeonOffset
                {
                    uint32_t Offset;
                    uint16_t IsDungeon;
                    uint16_t ExteriorLocID;
                };
                DungeonOffset *pOffsets = (DungeonOffset *)&pData[index]; index += 8*count;
                int base_index = index;
                for (int i=0; i<(int)count; i++)
                {
                    index = base_index + pOffsets[i].Offset;

                    //find matching exterior record...
                    Location_Daggerfall *pCurLoc=NULL;
                    for (int e=0; e<(int)m_pRegions[r].m_uLocationCount; e++)
                    {
                        if ( m_pRegions[r].m_pLocations[e].m_LocationID == pOffsets[i].ExteriorLocID )
                        {
                            pCurLoc = &m_pRegions[r].m_pLocations[e];
                            break;
                        }
                    }
                    assert(pCurLoc);

                    int nPreRecCount = *((int *)&pData[index]); index += 4;
                    uint8_t *PreRecords = NULL;
                    if ( nPreRecCount > 0 )
                    {
                        PreRecords = (uint8_t *)&pData[index]; index += nPreRecCount*6;
                    }
                    LocationRec *pLocation = (LocationRec *)&pData[index];
                    index += sizeof(LocationRec);
                    assert( pLocation->LocType == 0 );
                    if ( pLocation->LocType == 0 )
                    {
                        pCurLoc->m_waterHeight = (short)pLocation->Unknown5;
                        if ( pLocation->Unknown3&1 )
                        {
                            pCurLoc->m_waterHeight = -256000;
                        }
                        index += 8;//unknowns...
                        uint16_t blockCnt = *((uint16_t *)&pData[index]); index += 2;
                        index += 5; //unknown

                        pCurLoc->m_dungeonBlockCnt = blockCnt;
                        pCurLoc->m_startDungeonBlock = 0;
                        pCurLoc->m_pDungeonBlocks = new Location_Daggerfall::DungeonBlock[blockCnt];
                        bool bStartFound = false;
                        for (uint32_t b=0; b<blockCnt; b++)
                        {
                            char X = *((uint8_t *)&pData[index]); index++;
                            char Y = *((uint8_t *)&pData[index]); index++;
                            uint16_t  BlockNumStartIndex = *((uint16_t *)&pData[index]); index+=2;
                            int  blockNum = BlockNumStartIndex&1023;
                            bool IsStartBlock = (BlockNumStartIndex&1024) ? true : false;
                            uint32_t  blockIndex = (BlockNumStartIndex>>11)&0xff;
                            //now decode the string name...
                            char bIdxTable[] = { 'N', 'W', 'L', 'S', 'B', 'M' };
                            char szName[32];
                            sprintf(szName, "%c%07d.RDB", bIdxTable[blockIndex], blockNum);

                            if ( IsStartBlock )
                            {
                                pCurLoc->m_startDungeonBlock = b;
                                bStartFound = true;
                            }

                            strcpy( pCurLoc->m_pDungeonBlocks[b].szName, szName );
                            pCurLoc->m_pDungeonBlocks[b].x = X;
                            pCurLoc->m_pDungeonBlocks[b].y = Y;
                        }
                        assert( bStartFound );
                    }
                }
            }
            ScratchPad::FreeFrame();
        }
    }

    //just to make sure...
    ScratchPad::FreeFrame();
    m_bMapLoaded = true;

    Save();

    return true;
}

Location_Daggerfall *WorldMap::GetLocation(int32_t x, int32_t y)
{
    Location_Daggerfall *pLoc = NULL;

    uint64_t uKey = (uint64_t)y<<32ULL | (uint64_t)x;
    map<uint64_t, Location_Daggerfall *>::iterator iLoc = m_MapLoc.find(uKey);
    if ( iLoc != m_MapLoc.end() )
    {
        pLoc = iLoc->second;
    }

    return pLoc;
}

Location_Daggerfall *WorldMap::GetLocation(const char *pszName)
{
    Location_Daggerfall *pLoc = NULL;

    map<string, Location_Daggerfall *>::iterator iLoc = m_MapNames.find(pszName);
    if ( iLoc != m_MapNames.end() )
    {
        pLoc = iLoc->second;
    }

    return pLoc;
}

void WorldMap::SetWorldCell(int32_t x, int32_t y, WorldCell *pCell)
{
    uint64_t uKey = (uint64_t)y<<32ULL | (uint64_t)x;
    m_MapCell[uKey] = pCell;
}

WorldCell *WorldMap::GetWorldCell(int32_t x, int32_t y)
{
    WorldCell *pCell = NULL;
    uint64_t uKey = (uint64_t)y<<32ULL | (uint64_t)x;
    map<uint64_t, WorldCell *>::iterator iLoc = m_MapCell.find(uKey);

    if ( iLoc != m_MapCell.end() )
    {
        pCell = iLoc->second;
    }

    return pCell;
}

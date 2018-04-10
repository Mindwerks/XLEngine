#include "CellLoader_OutlawsMap.h"
#include "../world/WorldCell.h"
#include "../world/Sector_2_5D.h"
#include "../world/ObjectManager.h"
#include "../world/Object.h"
#include "../world/OrientedSprite.h"
#include "../world/Sprite_ZAxis.h"
#include "../render/TextureCache.h"
#include "../fileformats/TextureTypes.h"
#include "../fileformats/ArchiveTypes.h"
#include "../fileformats/ArchiveManager.h"
#include "../memory/ScratchPad.h"
#include "../math/Math.h"
#include "../ui/XL_Console.h"
#include "Parser.h"

#include <string>
#include <cstdio>
#include <cassert>

const float m_fWorldToTexel_X = 8.0f;
const float m_fWorldToTexel_Z = 8.0f;

#define MAX_WALLS 32768

//Outlaws Sector Flags.
enum
{
    OSF_SKY                 = (1<<0),
    OSF_PIT                 = (1<<1),
    OSF_SKY_ADJOIN          = (1<<2),
    OSF_PIT_ADJOIN          = (1<<3),
    OSF_NO_WALLS            = (1<<4),
    OSF_NO_SLIDING_SLOPE    = (1<<5),
    OSF_VEL_FLOOR_ONLY      = (1<<6),
    OSF_UNDERWATER          = (1<<7),
    OSF_DOOR                = (1<<8),
    OSF_DOOR_REVERSE        = (1<<9),
    OSF_FLAG_10             = (1<<10),
    OSF_FLAG_11             = (1<<11),
    OSF_SECRET_AREA         = (1<<12),
    OSF_FLAG_13             = (1<<13),
    OSF_FLAG_14             = (1<<14),
    OSF_FLAG_15             = (1<<15),
    OSF_FLAG_16             = (1<<16),
    OSF_FLAG_17             = (1<<17),
    OSF_DAMAGE_SM           = (1<<18),
    OSF_DAMAGE_LG           = (1<<19),
    OSF_DAMAGE_DY           = (1<<20),
    OSF_DEAN_WERMER         = (1<<21),
    OSF_SECRET_TAG          = (1<<22),
    OSF_DONT_SHADE_FLOOR    = (1<<23),
    OSF_RAIL_TRACK_PULL     = (1<<24),
    OSF_RAIL_LINE           = (1<<25),
    OSF_HIDE_ON_MAP         = (1<<26),
    OSF_FLOOR_SLOPED        = (1<<30),
    OSF_CEIL_SLOPED         = (1<<31)
};

//WAX data, how similar is NWX?
struct WAX_Header
{
    char id[4];
    int32_t Version;    // constant = 0x00100100
    int32_t Nseqs;      // number of SEQUENCES
    int32_t Nframes;    // number of FRAMES
    int32_t Ncells; // number of CELLS
    int32_t Xscale; // unused
    int32_t Yscale; // unused
    int32_t XtraLight;  // unused
    int32_t pad4;       // unused
    int32_t WAXES[32];  // pointers to WAXES
                    // = different actions
};

struct WAX
{
    int32_t Wwidth;     // World Width
    int32_t Wheight;    // World Height
    int32_t FrameRate;  // Frames per second
    int32_t Nframes;    // unused = 0
    int32_t pad2;       // unused = 0
    int32_t pad3;       // unused = 0
    int32_t pad4;       // unused = 0
    int32_t SEQS[32];   // pointers to SEQUENCES
                    // = views from different angles
};

struct SEQUENCE
{
    int32_t pad1;           // unused = 0
    int32_t pad2;           // unused = 0
    int32_t pad3;           // unused = 0
    int32_t pad4;           // unused = 0
    int32_t FRAMES[32]; // pointers to FRAMES
                        // = the animation frames
};

struct FRAME
{
    int32_t InsertX;        // Insertion point, X coordinate
                        // Negative values shift the cell left; Positive values shift the cell right
    int32_t InsertY;        // Insertion point, Y coordinate
                        // Negative values shift the cell up; Positive values shift the cell down
    int32_t Flip;           // 0 = not flipped
                        // 1 = flipped horizontally
    int32_t Cell;           // pointer to CELL = single picture
    int32_t UnitWidth;      // Unused
    int32_t UnitHeight; // Unused
    int32_t pad3;           // Unused
    int32_t pad4;           // Unused
};

struct CELL
{
    int32_t SizeX;          // Size of the CELL, X value
    int32_t SizeY;          // Size of the CELL, Y value
    int32_t Compressed; // 0 = not compressed
                        // 1 = compressed
    int32_t DataSize;       // Datasize for compressed CELL,// equals length of the CELL// If not compressed, DataSize = 0
    int32_t ColOffs;        // Always 0, because columns table
                        // follows just after
    int32_t pad1;           // Unused
};

struct CELL_NWX
{
    uint32_t size;
    uint32_t width;
    uint32_t height;
    uint32_t flags;
};

TextureHandle LoadNWX(uint8_t *pData, uint32_t uLen, const char *pszName, uint32_t& uWidth, uint32_t& uHeight)
{
    CELL_NWX *pCell0 = (CELL_NWX *)&pData[48];
    uint8_t *pImageData  = (uint8_t *)&pData[48+sizeof(CELL_NWX)];

    //1. read the column offsets.
    uint32_t columnOffsets[1024];
    uint32_t *pOffsetTable = (uint32_t *)pImageData;
    for (uint32_t x=0; x<pCell0->width; x++)
    {
        columnOffsets[x] = pOffsetTable[x];
    }
    uint8_t *pImage = (uint8_t *)ScratchPad::AllocMem( pCell0->width*pCell0->height );
    int32_t h = (int32_t)pCell0->height;
    //2. now fill in each column.
    for (uint32_t x=0; x<pCell0->width; x++)
    {
        uint8_t *pColumn = &pImageData[ columnOffsets[x] ];
        int32_t y=0;
        uint32_t cidx=0;
        while (y<(int32_t)pCell0->height)
        {
            //read the count.
            uint8_t count = pColumn[cidx]; cidx++;
            if ( count&1 )
            {
                uint8_t skip  = (count+1)>>1;
                uint8_t color = pColumn[cidx]; cidx++;
                if ( color < 16 ) color = 0;
                //fill in color * skip
                for (int32_t y1=y; y1<y+skip; y1++)
                {
                    pImage[x+(h-y1-1)*pCell0->width] = color;
                }
                y += skip;
            }
            else
            {
                uint8_t read = (count+2)>>1;
                for (int32_t y1=y; y1<y+read; y1++)
                {
                    uint8_t color = pColumn[cidx]; cidx++;
                    if ( color < 16 ) color = 0;
                    //fill in color
                    pImage[x+(h-y1-1)*pCell0->width] = color;
                }
                y += read;
            }
        };
    }

    uWidth  = pCell0->width;
    uHeight = pCell0->height;

    return TextureCache::LoadTextureFromMem_Pal(pImage, 0, pCell0->width, pCell0->height, pszName, false);

    //CELL_NWX *pCell1 = (CELL_NWX *)&pData[48+sizeof(CELL_NWX)+pCell0->size+4];
    //CELL_NWX *pCell2 = (CELL_NWX *)&pData[48+sizeof(CELL_NWX)+pCell0->size+4 + sizeof(CELL_NWX)+pCell1->size+4];
}

WorldCell *CellLoader_OutlawsMap::Load( IDriver3D *pDriver, World *pWorld, uint8_t *pData, uint32_t uLen, const std::string& sFile, int32_t worldX, int32_t worldY )
{
    WorldCell *pCell = nullptr;
    ObjectManager::FreeAllObjects();
    if ( uLen )
    {
        pCell = xlNew WorldCell();
        Parser::SetData((char *)pData, uLen);

        int32_t nTex;
        char szTexName[64];
        Parser::SearchKeyword_int32_t("TEXTURES", nTex);

        TextureHandle ahLevelTex[1024];
        float afTexScaleX[1024];
        float afTexScaleY[1024];
        uint32_t aTexWidth[1024];
        uint32_t aTexHeight[1024];

        //load the palette.
        TextureCache::GameFile_LoadTexture(TEXTURETYPE_PCX, 0, ARCHIVETYPE_LAB, "OUTLAWS.LAB", "HIDEOUT.PCX", false, true);
        
        //now load all the textures.
        for (int32_t t=0; t<nTex; t++)
        {
            Parser::SearchKeyword_Str("TEXTURE:", szTexName);

            size_t l = strlen(szTexName);
            if ( szTexName[l-3] == 'A' && szTexName[l-2] == 'T' && szTexName[l-1] == 'X' )
            {
                //we have to load and parse the animated file...
                ahLevelTex[t] = ahLevelTex[0];
                afTexScaleX[t] = afTexScaleX[0];
                afTexScaleY[t] = afTexScaleY[0];

                continue;
            }

            ahLevelTex[t] = TextureCache::GameFile_LoadTexture(TEXTURETYPE_PCX, 0, ARCHIVETYPE_LAB, "OLTEX.LAB", szTexName, true);
            uint32_t uTexWidth, uTexHeight;
            float fRelSizeX, fRelSizeY;
            assert( ahLevelTex[t] != XL_INVALID_TEXTURE );
            if ( ahLevelTex[t] != XL_INVALID_TEXTURE )
            {
                int32_t nOffsX, nOffsY;
                TextureCache::GetTextureSize(nOffsX, nOffsY, uTexWidth, uTexHeight, fRelSizeX, fRelSizeY);

                afTexScaleX[t] = m_fWorldToTexel_X/(float)uTexWidth;
                afTexScaleY[t] = m_fWorldToTexel_Z/(float)uTexHeight;

                aTexWidth[t]  = uTexWidth;
                aTexHeight[t] = uTexHeight;
            }
        }

        int32_t nSectorCnt = 0;
        Parser::SearchKeyword_int32_t("NUMSECTORS", nSectorCnt);

        //now search for sectors
        char szSecName[64];
        char szSecID[64];
        int32_t ambient, vtxCnt;
        float f_fTex, f_cTex;
        int32_t flags0, flags1, layer;
        uint32_t uFlags0;
        float secAlt=0.0f;
        int32_t nMinLayer = 10000, nMaxLayer = -10000;
        for (int32_t s=0; s<nSectorCnt; s++)
        {
            Sector_2_5D *pSector = xlNew Sector_2_5D();

            memset(szSecName, 0, 64);
            Parser::SearchKeyword_Str("SECTOR", szSecID);
            Parser::SearchKeyword_Str("NAME", szSecName);
            Parser::SearchKeyword_int32_t("AMBIENT", ambient);
            Parser::SearchKeyword_F1("FLOOR Y", pSector->m_ZRangeBase.x);
            Parser::SearchKeyword_F_Next(f_fTex);
            Parser::SearchKeyword_F_Next(pSector->m_texOffset[0].x);
            Parser::SearchKeyword_F_Next(pSector->m_texOffset[0].y);
            Parser::SearchKeyword_F1("CEILING Y", pSector->m_ZRangeBase.y);
            Parser::SearchKeyword_F_Next(f_cTex);
            Parser::SearchKeyword_F_Next(pSector->m_texOffset[1].x);
            Parser::SearchKeyword_F_Next(pSector->m_texOffset[1].y);
            Parser::SearchKeyword_int32_t("FLAGS", flags0);
            Parser::SearchKeyword_Next(flags1);

            strcpy(pSector->m_szName, szSecID);

            uFlags0 = (uint32_t)flags0;

            pSector->m_ZRangeCur = pSector->m_ZRangeBase;

            assert(f_fTex >= 0 && f_fTex < nTex);
            assert(f_cTex >= 0 && f_cTex < nTex);
            pSector->m_hFloorTex = ahLevelTex[ (int32_t)f_fTex ];
            pSector->m_hCeilTex  = ahLevelTex[ (int32_t)f_cTex ];

            pSector->m_FloorTexScale.Set( 0.125f * 64.0f / (float)aTexWidth[ (int32_t)f_fTex], 0.125f * 64.0f / (float)aTexHeight[(int32_t)f_fTex] );
            pSector->m_CeilTexScale.Set(  0.125f * 64.0f / (float)aTexWidth[ (int32_t)f_cTex], 0.125f * 64.0f / (float)aTexHeight[(int32_t)f_cTex] );

            pSector->m_texOffset[0].x *= pSector->m_FloorTexScale.x;
            pSector->m_texOffset[0].y *= pSector->m_FloorTexScale.y;
            pSector->m_texOffset[1].x *= pSector->m_CeilTexScale.x;
            pSector->m_texOffset[1].y *= pSector->m_CeilTexScale.y;

            //is this an exterior sector? If so then setup the sky...
            pSector->m_uFlags = 0;
            if ( uFlags0&OSF_SKY )
            {
                pCell->SetSkyTex( 0, pSector->m_hCeilTex, aTexWidth[ (int32_t)f_cTex], aTexHeight[(int32_t)f_cTex] );
                pCell->SetSkyTex( 1, pSector->m_hCeilTex, aTexWidth[ (int32_t)f_cTex], aTexHeight[(int32_t)f_cTex] );
                pCell->SetSkyTexCount(2);

                pSector->m_uFlags |= Sector_2_5D::SEC_FLAGS_EXTERIOR;
            }

            pSector->m_uAmbientFloor = ambient*8;
            pSector->m_uAmbientCeil  = ambient*8;
                        
            pSector->m_auSlopeAnchor[0] = 0;
            pSector->m_auSlopeAnchor[1] = 0;
            int32_t slope[3];
            Parser::SaveFilePtr();  //We're being extra cautious since these are optional values.
            if ( uFlags0&OSF_FLOOR_SLOPED )
            {
                Parser::SearchKeyword_int32_t("SLOPEDFLOOR", slope[0]);
                Parser::SearchKeyword_Next(slope[1]);
                Parser::SearchKeyword_Next(slope[2]);

                pSector->m_uFlags |= Sector_2_5D::SEC_FLAGS_FLOOR_SLOPE;
                pSector->m_auSlopeSector[0] = slope[0];
                pSector->m_auSlopeAnchor[0] = slope[1];
                pSector->m_fFloorSlope = 1.68f*(float)slope[2] / 4096.0f;
            }
            if ( uFlags0&OSF_CEIL_SLOPED )
            {
                Parser::SearchKeyword_int32_t("SLOPEDCEILING", slope[0]);
                Parser::SearchKeyword_Next(slope[1]);
                Parser::SearchKeyword_Next(slope[2]);

                pSector->m_uFlags |= Sector_2_5D::SEC_FLAGS_CEIL_SLOPE;
                pSector->m_auSlopeSector[1] = slope[0];
                pSector->m_auSlopeAnchor[1] = slope[1];
                pSector->m_fCeilSlope = 1.68f*(float)slope[2] / 4096.0f;
            }
            Parser::RestoreFilePtr();

            Parser::SearchKeyword_int32_t("LAYER", layer);
            if ( layer > nMaxLayer ) { nMaxLayer = layer; }
            if ( layer < nMinLayer ) { nMinLayer = layer; }
            pSector->m_uLayer = (uint32_t)layer;

            Parser::SearchKeyword_int32_t("VERTICES", vtxCnt);
            pSector->m_uVertexCount = (uint32_t)vtxCnt;
            pSector->m_pVertexBase = xlNew Vector2[vtxCnt];
            pSector->m_pVertexCur  = xlNew Vector2[vtxCnt];
            for (int32_t v=0; v<vtxCnt; v++)
            {
                Parser::SearchKeyword_F1("X:", pSector->m_pVertexBase[v].x);
                Parser::SearchKeyword_F1("Z:", pSector->m_pVertexBase[v].y);
                pSector->m_pVertexCur[v] = pSector->m_pVertexBase[v];
            }

            int32_t wallCnt;
            Parser::SearchKeyword_int32_t("WALLS", wallCnt);
            assert(wallCnt <= MAX_WALLS);
            pSector->m_uWallCount = (uint32_t)wallCnt;
            pSector->m_Walls = xlNew Wall[wallCnt];

            for (int32_t w=0; w<wallCnt; w++)
            {
                Wall *pWall = &pSector->m_Walls[w];
                int32_t idx[2];
                Parser::SearchKeyword_int32_t("V1:", idx[0]);
                Parser::SearchKeyword_int32_t("V2:", idx[1]);

                assert(idx[0] >= 0 && idx[0] < vtxCnt);
                assert(idx[1] >= 0 && idx[1] < vtxCnt);

                pWall->m_idx[0] = (uint16_t)idx[0];
                pWall->m_idx[1] = (uint16_t)idx[1];

                int32_t texIdx;
                Parser::SearchKeyword_int32_t("MID:", texIdx);
                pWall->m_textures[Wall::WALL_TEX_MID] = ahLevelTex[texIdx];
                pWall->m_texScale[Wall::WALL_TEX_MID].Set( afTexScaleX[texIdx], afTexScaleY[texIdx] );
                Parser::SearchKeyword_F_Next(pWall->m_texOffset[Wall::WALL_TEX_MID].x); pWall->m_texOffset[Wall::WALL_TEX_MID].x *= afTexScaleX[texIdx];
                Parser::SearchKeyword_F_Next(pWall->m_texOffset[Wall::WALL_TEX_MID].y); pWall->m_texOffset[Wall::WALL_TEX_MID].y *= afTexScaleY[texIdx];
                Parser::SearchKeyword_int32_t("TOP:", texIdx);
                pWall->m_textures[Wall::WALL_TEX_TOP] = ahLevelTex[texIdx];
                pWall->m_texScale[Wall::WALL_TEX_TOP].Set( afTexScaleX[texIdx], afTexScaleY[texIdx] );
                Parser::SearchKeyword_F_Next(pWall->m_texOffset[Wall::WALL_TEX_TOP].x); pWall->m_texOffset[Wall::WALL_TEX_TOP].x *= afTexScaleX[texIdx];
                Parser::SearchKeyword_F_Next(pWall->m_texOffset[Wall::WALL_TEX_TOP].y); pWall->m_texOffset[Wall::WALL_TEX_TOP].y *= afTexScaleY[texIdx];
                Parser::SearchKeyword_int32_t("BOT:", texIdx);
                pWall->m_textures[Wall::WALL_TEX_BOT] = ahLevelTex[texIdx];
                pWall->m_texScale[Wall::WALL_TEX_BOT].Set( afTexScaleX[texIdx], afTexScaleY[texIdx] );
                Parser::SearchKeyword_F_Next(pWall->m_texOffset[Wall::WALL_TEX_BOT].x); pWall->m_texOffset[Wall::WALL_TEX_BOT].x *= afTexScaleX[texIdx];
                Parser::SearchKeyword_F_Next(pWall->m_texOffset[Wall::WALL_TEX_BOT].y); pWall->m_texOffset[Wall::WALL_TEX_BOT].y *= afTexScaleY[texIdx];
                Parser::SearchKeyword_int32_t("OVERLAY:", texIdx);
                pWall->m_textures[Wall::WALL_TEX_SIGN] = (texIdx>-1) ? XL_INVALID_TEXTURE : ahLevelTex[texIdx];
                if ( texIdx > -1 )
                {
                    pWall->m_texScale[Wall::WALL_TEX_SIGN].Set( afTexScaleX[texIdx], afTexScaleY[texIdx] );
                    Parser::SearchKeyword_F_Next(pWall->m_texOffset[Wall::WALL_TEX_SIGN].x);
                    Parser::SearchKeyword_F_Next(pWall->m_texOffset[Wall::WALL_TEX_SIGN].y);
                }
                int32_t adjoin, mirror;
                Parser::SearchKeyword_int32_t("ADJOIN:", adjoin);
                Parser::SearchKeyword_int32_t("MIRROR:", mirror);
                pWall->m_adjoin[0] = adjoin < 0 ? SOLID_WALL : (uint16_t)adjoin;
                pWall->m_mirror[0] = mirror < 0 ? SOLID_WALL : (uint16_t)mirror;
                Parser::SearchKeyword_int32_t("DADJOIN:", adjoin);
                Parser::SearchKeyword_int32_t("DMIRROR:", mirror);
                pWall->m_adjoin[1] = adjoin < 0 ? SOLID_WALL : (uint16_t)adjoin;
                pWall->m_mirror[1] = mirror < 0 ? SOLID_WALL : (uint16_t)mirror;
                assert( pWall->m_adjoin[1] == SOLID_WALL || pWall->m_adjoin[0] != SOLID_WALL );
                int32_t wflags1, wflags2;
                Parser::SearchKeyword_int32_t("FLAGS:", wflags1);
                Parser::SearchKeyword_Next(wflags2);
                int32_t light;
                Parser::SearchKeyword_int32_t("LIGHT:", light);
                int32_t wallLight = (int16_t)light;
                if ( wallLight > 32767 )
                {
                    wallLight = wallLight - 65535;
                }
                pWall->m_lightDelta = wallLight*8 + ambient*8;
                if ( pWall->m_lightDelta < 0  ) pWall->m_lightDelta = 0;
                if ( pWall->m_lightDelta > 255) pWall->m_lightDelta = 255;

                //compute wall length...
                const Vector2& v0 = pSector->m_pVertexBase[ pWall->m_idx[0] ];
                const Vector2& v1 = pSector->m_pVertexBase[ pWall->m_idx[1] ];
                Vector2 offs = v1 - v0;
                pWall->m_wallLen = offs.Length();

                pWall->m_flags = 0;
                if ( uFlags0&OSF_NO_WALLS )
                {
                    pWall->m_flags |= Wall::WALL_FLAGS_SKY;
                }
                if ( ((uint32_t)wflags1)&1 )
                {
                    pWall->m_flags |= Wall::WALL_FLAGS_MASKWALL;
                }
            }

            pCell->AddSector( pSector );
        }

        //now load objects...
        //the first step is to change the extension.
        char szObjectFile[32];
        size_t l = sFile.size();
        for (size_t c=0; c<l-3; c++)
        {
            szObjectFile[c] = sFile[c];
        }
        szObjectFile[l-3] = 'O';
        szObjectFile[l-2] = 'B';
        szObjectFile[l-1] = 'T';
        szObjectFile[l]   = 0;

        Vector3 vStartLoc(322.36f, 271.88f, -3.0f);
        uint32_t uStartSec = 410;
        if ( ArchiveManager::GameFile_Open(ARCHIVETYPE_LAB, "OLGEO_1.LAB", szObjectFile) )
        {
            uint32_t uObjLen  = ArchiveManager::GameFile_GetLength();
            uint8_t *pObjData = (uint8_t *)ScratchPad::AllocMem( uObjLen+1 );

            if ( pObjData )
            {
                ArchiveManager::GameFile_Read(pObjData, uObjLen);
                ArchiveManager::GameFile_Close();

                Parser::SetData((char *)pObjData, uObjLen);

                int32_t nObjCnt;
                Parser::SearchKeyword_int32_t("OBJECTS", nObjCnt);

                char szName[32];
                char szID[32];
                char szSecID[32];
                Vector3 vLoc;
                float yaw, pch, roll;
                int32_t flags0, flags1;

                //allocate some temporary memory for all the items...
                uint8_t *pItmData = (uint8_t *)ScratchPad::AllocMem( 2048 );    //Is 2K enough?

                for (int32_t i=0; i<nObjCnt; i++)
                {
                    Parser::SearchKeyword_Str("NAME:",  szName);
                    Parser::SearchKeyword_Str("ID:",    szID);
                    Parser::SearchKeyword_Str("SECTOR:",szSecID);
                    Parser::SearchKeyword_F1("X:",      vLoc.x);
                    Parser::SearchKeyword_F1("Y:",      vLoc.z);
                    Parser::SearchKeyword_F1("Z:",      vLoc.y);
                    Parser::SearchKeyword_F1("PCH:",    pch);
                    Parser::SearchKeyword_F1("YAW:",    yaw);
                    Parser::SearchKeyword_F1("ROL:",    roll);
                    Parser::SearchKeyword_int32_t("FLAGS:", flags0);
                    Parser::SearchKeyword_Next(flags1);

                    //find the sector...
                    int32_t nSector = pCell->FindSector(szSecID);
                    if ( nSector == -1 )
                        nSector = 0;
                    //handle the player             
                    if ( strnicmp(szName, "PLAYER", 6) == 0 )
                    {
                        if ( nSector > -1 )
                            uStartSec = (uint32_t)nSector;

                        vStartLoc = vLoc;
                    }
                    else if ( nSector > -1 )
                    {
                        //handle regular items.
                        char szItemFile[32];
                        sprintf(szItemFile, "%s.ITM", szName);
                        if ( ArchiveManager::GameFile_Open(ARCHIVETYPE_LAB, "OLOBJ.LAB", szItemFile) )
                        {
                            uint32_t uItmLen = ArchiveManager::GameFile_GetLength();
                            assert(uItmLen <= 2048);
                            if ( uItmLen <= 2048 )
                            {
                                ArchiveManager::GameFile_Read(pItmData, uItmLen);
                                ArchiveManager::GameFile_Close();
                                //for now just read the "ANIM"
                                for (uint32_t c=0; c<uItmLen; c++)
                                {
                                    if ( pItmData[c] == 'A' && pItmData[c+1] == 'N' && pItmData[c+2] == 'I' && pItmData[c+3] == 'M' )
                                    {
                                        //ok now find the first non-space...
                                        uint32_t cc=c+4;
                                        for (; cc<uItmLen; cc++)
                                        {
                                            if ( pItmData[cc] != ' ' )
                                            {
                                                break;
                                            }
                                        }
                                        //now read in the filename until a space, newline, etc. is found.
                                        char spriteName[32];
                                        uint32_t s=0;
                                        for (; cc<uItmLen; cc++)
                                        {
                                            if ( pItmData[cc] == ' ' || pItmData[cc] == 0 || pItmData[cc] == '\n' || pItmData[cc] == '\t' || pItmData[cc] == 13 )
                                            {
                                                spriteName[s] = 0;
                                                break;
                                            }
                                            spriteName[s++] = pItmData[cc];
                                        }
                                        //is this a NWX file? that is the only file type we support right now...
                                        size_t l = strlen(spriteName);
                                        if ( toupper(spriteName[l-3]) == 'N' && toupper(spriteName[l-2]) == 'W' && toupper(spriteName[l-1]) == 'X' )
                                        {
                                            //ok load the NWX file...
                                            ScratchPad::StartFrame();
                                            if ( ArchiveManager::GameFile_Open(ARCHIVETYPE_LAB, "OLOBJ.LAB", spriteName) )
                                            {
                                                uint32_t uNWX_Len = ArchiveManager::GameFile_GetLength();
                                                uint8_t *pNWX_Data = (uint8_t *)ScratchPad::AllocMem( uNWX_Len );
                                                ArchiveManager::GameFile_Read(pNWX_Data, uNWX_Len);
                                                ArchiveManager::GameFile_Close();

                                                uint32_t uWidth, uHeight;
                                                TextureHandle hTex = LoadNWX(pNWX_Data, uNWX_Len, spriteName, uWidth, uHeight);

                                                Object *pObj = ObjectManager::CreateObject("Sprite_ZAxis");
                                                if ( pObj )
                                                {
                                                    uint32_t uObjID = pObj->GetID();
                                                    Sector_2_5D *pSector = (Sector_2_5D *)pCell->GetSector( nSector );
                                                    pObj->SetSector( nSector );
                                                    
                                                    //
                                                    Vector3 vScale;
                                                    vScale.x = (float)uWidth * 0.125f * 0.5f * 0.525f;
                                                    vScale.y = (float)uWidth * 0.125f * 0.5f * 0.525f;
                                                    vScale.z = (float)uHeight* 0.125f * 0.5f * 0.525f;

                                                    vLoc.z += vScale.z;

                                                    pObj->SetScale(vScale);
                                                    pObj->SetLoc(vLoc);

                                                    float z0 = pSector->GetZ_Floor(vLoc.x, vLoc.y, pCell->GetSectors());
                                                    float z1 = pSector->GetZ_Ceil(vLoc.x, vLoc.y, pCell->GetSectors());

                                                    if ( vLoc.z-vScale.z < z0 )
                                                    {
                                                        vLoc.z = z0+vScale.z;
                                                    }
                                                    else if ( vLoc.z+vScale.z > z1 )
                                                    {
                                                        vLoc.z = z1-vScale.z;
                                                    }
                                                    else if ( vLoc.z < (z0+z1)*0.5f )
                                                    {
                                                        vLoc.z = z0+vScale.z;
                                                    }

                                                    Sprite_ZAxis *pSprite = xlNew Sprite_ZAxis();
                                                    pSprite->SetTextureHandle( hTex );

                                                    pSprite->SetUV_Flip(false, false);
                                                    pSprite->SetAlpha( 1.0f );
                                                    pObj->SetRenderComponent( pSprite );

                                                    pSector->AddObject( uObjID );
                                                }
                                            }
                                            ScratchPad::FreeFrame();
                                        }
                                        break;
                                    }
                                }
                            }
                        }
                        else
                        {
                            char szItemFile[32];
                            sprintf(szItemFile, "%s.NWX", szName);
                            if ( ArchiveManager::GameFile_Open(ARCHIVETYPE_LAB, "OLOBJ.LAB", szItemFile) )
                            {
                                //ok load the NWX file...
                                ScratchPad::StartFrame();

                                uint32_t uNWX_Len = ArchiveManager::GameFile_GetLength();
                                uint8_t *pNWX_Data = (uint8_t *)ScratchPad::AllocMem( uNWX_Len );
                                ArchiveManager::GameFile_Read(pNWX_Data, uNWX_Len);
                                ArchiveManager::GameFile_Close();

                                uint32_t uWidth, uHeight;
                                TextureHandle hTex = LoadNWX(pNWX_Data, uNWX_Len, szItemFile, uWidth, uHeight);

                                Object *pObj = ObjectManager::CreateObject("Sprite_ZAxis");
                                if ( pObj )
                                {
                                    uint32_t uObjID = pObj->GetID();
                                    Sector_2_5D *pSector = (Sector_2_5D *)pCell->GetSector( nSector );
                                    pObj->SetSector( nSector );
                                    
                                    //
                                    Vector3 vScale;
                                    vScale.x = (float)uWidth * 0.125f * 0.5f * 0.525f;
                                    vScale.y = (float)uWidth * 0.125f * 0.5f * 0.525f;
                                    vScale.z = (float)uHeight* 0.125f * 0.5f * 0.525f;

                                    vLoc.z += vScale.z;

                                    pObj->SetScale(vScale);
                                    pObj->SetLoc(vLoc);

                                    float z0 = pSector->GetZ_Floor(vLoc.x, vLoc.y, pCell->GetSectors());
                                    float z1 = pSector->GetZ_Ceil(vLoc.x, vLoc.y, pCell->GetSectors());

                                    if ( vLoc.z-vScale.z < z0 )
                                    {
                                        vLoc.z = z0+vScale.z;
                                    }
                                    else if ( vLoc.z+vScale.z > z1 )
                                    {
                                        vLoc.z = z1-vScale.z;
                                    }
                                    else if ( vLoc.z < (z0+z1)*0.5f )
                                    {
                                        vLoc.z = z0+vScale.z;
                                    }

                                    Sprite_ZAxis *pSprite = xlNew Sprite_ZAxis();
                                    pSprite->SetTextureHandle( hTex );

                                    pSprite->SetUV_Flip(false, false);
                                    pSprite->SetAlpha( 1.0f );
                                    pObj->SetRenderComponent( pSprite );

                                    pSector->AddObject( uObjID );
                                }
                            }
                            ScratchPad::FreeFrame();
                        }
                    }
                }
            }
        }

        //start the player in the correct place.
        Object *player = ObjectManager::FindObject("PLAYER");
        if ( player )
        {
            player->SetLoc(vStartLoc);
            player->SetSector(uStartSec);
        }
    }

    return pCell;
}
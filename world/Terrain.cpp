#include "Terrain.h"
#include "World.h"
#include "WorldCell.h"
#include "Sector_GeoBlock.h"
#include "ObjectManager.h"
#include "Sprite_ZAxis.h"
#include "../render/TextureCache.h"
#include "../render/Camera.h"
#include "../fileformats/TextureTypes.h"
#include "../fileformats/ArchiveTypes.h"
#include "../fileformats/ArchiveManager.h"
#include "../procedural/ProceduralFunc.h"
#include "../fileformats/Location_Daggerfall.h"
#include "../memory/ScratchPad.h"
#include "../math/Math.h"
#include "../os/Input.h"
//just load the Daggerfall skyloader for now..., fix later.
#include "../fileformats/SkyLoader_Daggerfall.h"
#include <algorithm>
#include <cstring>

struct TerrainVtx
{
	float x, y, z;
	float nx, ny, nz;
	float u, v;
};
TextureHandle m_ahTex[256];

//Constants
const int Terrain::c_numVertexEdge = CHUNK_TILE_WIDTH * TILE_QUAD_WIDTH + 1;
const float Terrain::c_fTotalWidth  = (float)CHUNK_TILE_WIDTH * 1024.0f;
const float Terrain::c_fTotalHeight = (float)CHUNK_TILE_WIDTH * 1024.0f;
const Vector3 Terrain::c_startPos( -c_fTotalWidth*0.5f, -c_fTotalHeight*0.5f, 0.0f );
Vector3 Terrain::m_vCamDir;
Vector3 Terrain::m_vCamLoc;

TextureHandle m_hDebugHM;
TextureHandle m_hDebugClimate;
TextureHandle m_hDebugGroundTex;
TextureHandle m_hPosMarker;

bool m_bShowTerrainDebug = false;
int32_t  m_nTerrainMapX = 0;
int32_t  m_nTerrainMapY = 0;
int32_t  m_nTerrainMapScale = 1;

const float m_fSkyTop =  136.0f;
const float m_fSkyBot =   -4.0f;

//Texture Tile Mapping Data
#define E(a,b,c,d) a+(b<<2)+(c<<4)+(d<<6)

static const int32_t _baseTileCnt=4;
static const int32_t _comboCount = _baseTileCnt*_baseTileCnt*_baseTileCnt*_baseTileCnt;
static int32_t _anTileMapping[_comboCount];
static const int32_t _workSetSize = 56;
static int32_t _anWorkingSet[]=
{
	E(0,0,0,0), E(1,1,1,1), E(2,2,2,2), E(3,3,3,3),         -1, E(1,0,0,0), 
	E(1,0,1,0), E(1,1,1,0),         -1,         -1, E(2,1,1,1), E(2,1,2,1),
	E(2,2,2,1),	        -1,         -1, E(3,2,2,2), E(3,2,3,2), E(3,3,3,2),
			-1,         -1, E(2,0,0,0), E(2,0,2,0), E(2,2,2,0),         -1,
		    -1, E(3,1,1,1), E(3,1,3,1), E(3,3,3,1),         -1,         -1,
	E(3,0,0,0), E(3,0,3,0), E(3,3,3,0),         -1, E(1,0,2,0), E(2,0,3,0),
	E(1,0,3,0), E(1,2,1,0), E(1,3,1,0), E(1,3,1,2), E(2,1,2,0), E(2,3,2,0),
	E(2,3,2,1), E(3,1,3,0), E(3,2,3,0), E(3,2,3,1),         -1,         -1,
	E(1,0,0,1), E(2,0,0,2), E(3,0,0,3), E(2,1,1,2), E(3,1,1,3), E(3,2,2,3),
		    -1,         -1
};
static int32_t m_anMapSky[]=
{
	29,
	 9,
	 9,
	 1,
	29,
	29,
	25,
	17,
	17,
	17
};

int32_t Terrain::s_anMapClimate[]=
{
	CLIMATE_SWAMP,
	CLIMATE_DESERT,
	CLIMATE_DESERT,
	CLIMATE_MOUNTAIN,
	CLIMATE_SWAMP,
	CLIMATE_SWAMP,
	CLIMATE_DESERT,
	CLIMATE_MOUNTAIN,
	CLIMATE_TEMPERATE,
	CLIMATE_TEMPERATE,
};

int32_t Terrain::s_anMapFlat[]=
{
	502,
	503,
	503,
	510,
	500,
	502,
	503,
	504,
	508,
	508
};

void TerrainDebug_KeyDownCB(int32_t key)
{
	if ( key == XL_F3 )
	{
		m_bShowTerrainDebug = !m_bShowTerrainDebug;
	}
	if ( key == XL_ADD )
	{
		m_nTerrainMapScale = Math::Min( m_nTerrainMapScale<<1, 16 );
	}
	else if ( key == XL_SUBTRACT )
	{
		m_nTerrainMapScale = Math::Max( m_nTerrainMapScale>>1, 1 );
	}
}

Terrain::Terrain(IDriver3D *pDriver, World *pWorld)
{
	m_pDriver = pDriver;
	m_pWorld  = pWorld;
	m_bActive = false;
	m_bMeshBuilt = false;

	m_nWidth  = 0;
	m_nHeight = 0;
	m_x = -1;
	m_y = -1;
	m_afHeightmap = NULL;
	m_pClimate    = NULL;
	m_afCoastalDist = NULL;
	m_pSector = NULL;
	m_pWorldCell = NULL;

	for (uint32_t l=0; l<LOD_COUNT; l++)
	{
		m_LOD[l].m_pVB       = NULL;
		for (int i=0; i<CHUNK_COUNT; i++) m_LOD[l].m_aChunks[i].m_pChunkIB  = NULL;
		m_LOD[l].m_pGlobalIB = NULL;

		m_LOD[l].m_pLocalHM = xlNew float[(CHUNK_TILE_WIDTH*TILE_QUAD_WIDTH+1) * (CHUNK_TILE_WIDTH*TILE_QUAD_WIDTH+1)];
		m_LOD[l].m_pLocalNM = xlNew Vector3[(CHUNK_TILE_WIDTH*TILE_QUAD_WIDTH+1) * (CHUNK_TILE_WIDTH*TILE_QUAD_WIDTH+1)];
		m_LOD[l].m_pHM_Flags = xlNew float[(CHUNK_TILE_WIDTH*TILE_QUAD_WIDTH+1) * (CHUNK_TILE_WIDTH*TILE_QUAD_WIDTH+1)];
	}

	//debug
	Input::AddKeyDownCallback( TerrainDebug_KeyDownCB, Input::KDCb_FLAGS_NOREPEAT );
	m_ChunkRenderList.reserve( CHUNK_COUNT );

	m_pSkyLoader = xlNew SkyLoader_Daggerfall();
	//for now just load a region.
	m_pSkyLoader->LoadSky(0);
}

Terrain::~Terrain()
{
	if ( m_afHeightmap )
	{
		xlDelete [] m_afHeightmap;
	}
	if ( m_afCoastalDist )
	{
		xlDelete [] m_afCoastalDist;
	}
	if ( m_pClimate )
	{
		xlDelete [] m_pClimate;
	}
	for (uint32_t l=0; l<LOD_COUNT; l++)
	{
		if ( m_LOD[l].m_pVB )
		{
			xlDelete m_LOD[l].m_pVB;
		}
		if ( m_LOD[l].m_aChunks[0].m_pChunkIB )
		{
			for (int i=0; i<CHUNK_COUNT; i++) { xlDelete m_LOD[l].m_aChunks[i].m_pChunkIB; }
		}
		if ( m_LOD[l].m_pGlobalIB )
		{
			xlDelete m_LOD[l].m_pGlobalIB;
		}
		xlDelete [] m_LOD[l].m_pLocalHM;
		xlDelete [] m_LOD[l].m_pLocalNM;
		xlDelete [] m_LOD[l].m_pHM_Flags;
	}
	xlDelete m_pSkyLoader;
	xlDelete m_pSector;
	xlDelete m_pWorldCell;
}

void Terrain::BinLocations()
{
	//clear out the map, if it is already loaded.
	m_LocationMap.clear();
	
	//now place locations on the map...
	for (uint32_t r=0; r<WorldMap::m_uRegionCount; r++)
	{
		for (uint32_t l=0; l<WorldMap::m_pRegions[r].m_uLocationCount; l++)
		{
			uint32_t x = (uint32_t)(WorldMap::m_pRegions[r].m_pLocations[l].m_x/8.0f);
			uint32_t y = (uint32_t)(WorldMap::m_pRegions[r].m_pLocations[l].m_y/8.0f);
			assert(x >= 0 && x < 1000);
			assert(y >= 0 && y < 500);

			uint32_t key = LOC_KEY(x,y);
			m_LocationMap[key] = &WorldMap::m_pRegions[r].m_pLocations[l];
		}
	}
}

//////////////////////////////////////////////////////
//Load here for now, however this should be moved later
//so other games can use terrain.
//////////////////////////////////////////////////////
struct MapPixelData
{
	uint16_t FileIndex;
	uint8_t Climate;
	uint8_t ClimateNoise;
	uint8_t ElevNoise[25];	//5x5 noise?
};

void Terrain::LoadHeightmap()
{
	static uint32_t uHM_Data[1000*500];

	const char *szFile = "WOODS.WLD";
	if ( ArchiveManager::File_Open(szFile) )
	{
		ScratchPad::StartFrame();

		//Read the file.
		uint32_t uLen  = ArchiveManager::File_GetLength();
		uint8_t *pData = (uint8_t *)ScratchPad::AllocMem( uLen+1 );
		ArchiveManager::File_Read(pData, 0, uLen);
		ArchiveManager::File_Close();

		//Process the contents.
		int32_t index = 0;
		uint32_t OffsetSize = *((uint32_t *)&pData[index]); index += 4;
		uint32_t Width = *((uint32_t *)&pData[index]); index += 4;
		uint32_t Height = *((uint32_t *)&pData[index]); index += 4;
		index += 4;	//NullValue
		uint32_t DataSec1Offs = *((uint32_t *)&pData[index]); index += 4;
		index += 8; //unknown * 2
		uint32_t AltMapOffs = *((uint32_t *)&pData[index]); index += 4;
		index += 28*4; //uint32 * 28

		assert(Width == 1000);
		assert(Height == 500);

		//500000 offsets...
#if 0
		uint32_t *pDataOffs = (uint32_t *)&pData[index];
		MapPixelData *pMapPixData = (MapPixelData *)ScratchPad::AllocMem(Width*Height*sizeof(MapPixelData));
		for (uint32_t i=0; i<Width*Height; i++)
		{
			index = pDataOffs[i];
			index += 2; index += 4;	//unknown, null
			pMapPixData[i].FileIndex = *((uint16_t *)&pData[index]); index += 2;
			pMapPixData[i].Climate = *((uint8_t *)&pData[index]); index++;
			pMapPixData[i].ClimateNoise = *((uint8_t *)&pData[index]); index++;
			index += 4*3;	//null
			memcpy(pMapPixData[i].ElevNoise, &pData[index], 25);
		}
#endif

		//Read altitude...
		index = AltMapOffs;
		uint8_t *pAltMap = (uint8_t *)&pData[index];

		m_nWidth  = Width;
		m_nHeight = Height;
		m_afHeightmap   = xlNew float[Width*Height];
		m_afCoastalDist = xlNew float[Width*Height];

		FilterHeightMap(pAltMap, m_afHeightmap, m_afCoastalDist);

		//DEBUG
		float fOO64 = (1.0f/64.0f);
		for (uint32_t i=0; i<1000*500; i++)
		{
			uint32_t I = (uint32_t)Math::clamp( m_afHeightmap[i]*fOO64, 0.0f, 255.0f );
			uHM_Data[i] = I | (I<<8) | (I<<16) | (0xff<<24);
		}
		m_hDebugHM = m_pDriver->CreateTexture(1000, 500, IDriver3D::TEX_FORMAT_FORCE_32bpp, (uint8_t *)&uHM_Data[0], false);
		//

		ScratchPad::FreeFrame();
	}

	szFile = "CLIMATE.PAK";
	if ( ArchiveManager::File_Open(szFile) )
	{
		ScratchPad::StartFrame();

		//Read the file.
		uint32_t uLen  = ArchiveManager::File_GetLength();
		uint8_t *pData = (uint8_t *)ScratchPad::AllocMem( uLen+1 );
		ArchiveManager::File_Read(pData, 0, uLen);
		ArchiveManager::File_Close();

		//read climate data.
		m_pClimate = xlNew uint8_t[1001*500];
		uint32_t index = 0;
		uint32_t *pOffsList = (uint32_t *)&pData[index];
		for (int y=0, yOffs=0; y<500; y++, yOffs+=1000)
		{
			index = pOffsList[y];
			int32_t x = 0;
			while (x < 1000)
			{
				uint16_t Count = *((uint16_t *)&pData[index]); index += 2;
				uint8_t  Value = *((uint8_t *)&pData[index]); index++;

				for (uint32_t i=0; i<Count; i++, x++)
				{
					m_pClimate[yOffs+x] = Value;
				}
			};
		}

		//DEBUG
		static uint32_t uClimate_Data[1000*500];
		static uint32_t _aClimateColors[]=
		{
			0xff004000,
			0xff004080,
			0xff005090,
			0xff808080,
			0xff005000,
			0xff006000,
			0xff0060a0,
			0xff909090,
			0xff009000,
			0xff00a000,
		};
		for (uint32_t i=0; i<1000*500; i++)
		{
			if ( (uHM_Data[i]&0xff) == 0 )
			{
				uClimate_Data[i] = 0xff200000;

				//int I = (int)( Math::Min( m_afCoastalDist[i], 1.0f ) * 255.0f );
				//uClimate_Data[i] = I | (I<<8) | (I<<16) | (0xff<<24);
			}
			else
			{
				uClimate_Data[i] = _aClimateColors[ m_pClimate[i]-223 ];
				uint32_t R = uClimate_Data[i]&0xff;
				uint32_t G = (uClimate_Data[i]>>8)&0xff;
				uint32_t B = (uClimate_Data[i]>>16)&0xff;

				uint32_t H = Math::clamp((uHM_Data[i]&0xff)*2, 0, 255);
				R = (R * H)>>8;
				G = (G * H)>>8;
				B = (B * H)>>8;

				uClimate_Data[i] = (0xff<<24) | R | (G<<8) | (B<<16);

				//scale heightmap if in mountains?
				if ( s_anMapClimate[m_pClimate[i]-223] == Terrain::CLIMATE_MOUNTAIN )
				{
					m_afHeightmap[i] *= 2.0f;
				}
			}
		}

		//now place locations on the map...
		for (uint32_t r=0; r<WorldMap::m_uRegionCount; r++)
		{
			for (uint32_t l=0; l<WorldMap::m_pRegions[r].m_uLocationCount; l++)
			{
				int32_t x = (int32_t)(WorldMap::m_pRegions[r].m_pLocations[l].m_x/8.0f);
				int32_t y = 499 - (int32_t)(WorldMap::m_pRegions[r].m_pLocations[l].m_y/8.0f);
				assert(x >= 0 && x < 1000);
				assert(y >= 0 && y < 500);

				if ( WorldMap::m_pRegions[r].m_pLocations[l].m_BlockWidth || WorldMap::m_pRegions[r].m_pLocations[l].m_BlockHeight )
				{
					int32_t lumR = Math::Min( (WorldMap::m_pRegions[r].m_pLocations[l].m_BlockWidth*WorldMap::m_pRegions[r].m_pLocations[l].m_BlockHeight)<<2, 255 );
					int32_t lumGB = Math::Min( lumR+64, 255 );
					uClimate_Data[y*1000+x] = 0xff000000 | (lumGB<<16) | (lumGB<<8) | lumR;
				}
				else
				{
					uClimate_Data[y*1000+x] = 0xff00ffff;
				}
			}
		}

		m_hDebugClimate = m_pDriver->CreateTexture(1000, 500, IDriver3D::TEX_FORMAT_FORCE_32bpp, (uint8_t *)&uClimate_Data[0], false);
		uint32_t uRed = 0xff000080, uDkRed = 0xff000040;
		uint32_t auMarkerArr[] = { uDkRed, uDkRed, uDkRed, uDkRed, uRed, uDkRed, uDkRed, uDkRed, uDkRed, uDkRed };
		m_hPosMarker = m_pDriver->CreateTexture(3, 3, IDriver3D::TEX_FORMAT_FORCE_32bpp, (uint8_t *)&auMarkerArr[0], false);

		//debug ground texture.
		uint32_t _groundTex[256*256];
		Vector3 p(0.2f, 0.2f, 0.675f);
		float delta = 8.0f/256.0f;
		static const uint32_t colors[]=
		{
			0xff3c5c7b, 0xff25402d, 0xff636363
		};
		for (int32_t y=0; y<256; y++)
		{
			p.x = 0.2f;
			for (int32_t x=0; x<256; x++)
			{
				float f = ProceduralFunc::fBm(p, 4);
				int32_t I = (int32_t)Math::clamp(255.0f*(f*0.5f+0.5f), 0.0f, 255.0f);

				int32_t index = Math::clamp( I/85, 0, 2 );
				_groundTex[ y*256 + x ] = colors[index];

				p.x += delta;
			}
			p.y += delta;
		}
		m_hDebugGroundTex = m_pDriver->CreateTexture(256, 256, IDriver3D::TEX_FORMAT_FORCE_32bpp, (uint8_t *)&_groundTex[0], false);
		//
	}
}

void Terrain::FilterHeightMap(uint8_t *pAltMap, float *pAltMapF, float *pCoastalDist)
{
	static float afHeights0[1000*500];
	static float afHeights1[1000*500];
	int x, xx, y, yy, yOffs;
	//Setup floats
	for (y=0, yOffs=0; y<500; y++, yOffs+=1000)
	{
		for (x=0; x<1000; x++)
		{
			afHeights0[yOffs+x] = (float)pAltMap[yOffs+x];
		}
	}
#if 1
	//Fixup water ahead of time to help the filter...
	//Handle water
	for (y=0, yOffs=0; y<500; y++, yOffs+=1000)
	{
		for (x=0; x<1000; x++)
		{
			if (pAltMap[yOffs+x] <= 2)
			{
				afHeights0[yOffs+x] = 0.0f;
			}
			else if ( x > 0 && pAltMap[yOffs+x-1] <= 2 )
			{
				afHeights0[yOffs+x] = 1.0f;
			}
			else if ( x < 999 && pAltMap[yOffs+x+1] <= 2 )
			{
				afHeights0[yOffs+x] = 1.0f;
			}
			else if ( y > 0 && pAltMap[yOffs-1000+x] <= 2 )
			{
				afHeights0[yOffs+x] = 1.0f;
			}
			else if ( y < 499 && pAltMap[yOffs+1000+x] <= 2 )
			{
				afHeights0[yOffs+x] = 1.0f;
			}
			else if ( x > 0 && y > 0 && pAltMap[yOffs-1000+x-1] <= 2 )
			{
				afHeights0[yOffs+x] = 1.0f;
			}
			else if ( x > 0 && y < 499 && pAltMap[yOffs+1000+x-1] <= 2 )
			{
				afHeights0[yOffs+x] = 1.0f;
			}
			else if ( x < 999 && y > 0 && pAltMap[yOffs-1000+x+1] <= 2 )
			{
				afHeights0[yOffs+x] = 1.0f;
			}
			else if ( x < 999 && y < 499 && pAltMap[yOffs+1000+x+1] <= 2 )
			{
				afHeights0[yOffs+x] = 1.0f;
			}
		}
	}

	//kernel
	float afKernel[] = { 1.0f, 1.0f, 2.0f, 4.0f, 8.0f, 4.0f, 2.0f, 1.0f, 1.0f };
	float weight = 0.0f;
	//Horizontal blur
	for (int p=0; p<8; p++)
	{
		for (y=0, yOffs=0; y<500; y++, yOffs+=1000)
		{
			for (x=0; x<1000; x++)
			{
				weight = 0.0f;
				if (pAltMap[yOffs+x] <= 2)
				{
					afHeights1[yOffs+x] = afHeights0[yOffs+x];
				}
				else
				{
					afHeights1[yOffs+x] = 0.0f;
					for (xx=x-4; xx<=x+4; xx++)
					{
						int u = xx;
						u = (u < 0 )  ? 0 : u;
						u = (u > 999) ? 999 : u;
					
						if (pAltMap[yOffs+u] > 2)
						{
							afHeights1[yOffs+x] += afHeights0[yOffs+u] * afKernel[xx-x+4];
							weight += afKernel[xx-x+4];
						}
					}
					afHeights1[yOffs+x] /= weight;
				}
			}
		}
		//Vertical blur
		for (y=0, yOffs=0; y<500; y++, yOffs+=1000)
		{
			for (x=0; x<1000; x++)
			{
				weight = 0.0f;
				if (pAltMap[yOffs+x] <= 2)
				{
					afHeights0[yOffs+x] = afHeights1[yOffs+x];
				}
				else
				{
					afHeights0[yOffs+x] = 0.0f;
					for (yy=y-4; yy<=y+4; yy++)
					{
						int v = yy;
						v = (v < 0 ) ? 0 : v;
						v = (v > 499) ? 499 : v;
					
						if (pAltMap[v*1000+x] > 2)
						{
							afHeights0[yOffs+x] += afHeights1[v*1000+x] * afKernel[yy-y+4];
							weight += afKernel[yy-y+4];
						}
					}
					afHeights0[yOffs+x] /= weight;
				}
			}
		}
	}
	//Handle water
	for (y=0, yOffs=0; y<500; y++, yOffs+=1000)
	{
		for (x=0; x<1000; x++)
		{
			if (pAltMap[yOffs+x] <= 2)
			{
				afHeights0[yOffs+x] = 0.0f;
			}
			else if ( x > 0 && pAltMap[yOffs+x-1] <= 2 )
			{
				afHeights0[yOffs+x] = 1.0f;
			}
			else if ( x < 999 && pAltMap[yOffs+x+1] <= 2 )
			{
				afHeights0[yOffs+x] = 1.0f;
			}
			else if ( y > 0 && pAltMap[yOffs-1000+x] <= 2 )
			{
				afHeights0[yOffs+x] = 1.0f;
			}
			else if ( y < 499 && pAltMap[yOffs+1000+x] <= 2 )
			{
				afHeights0[yOffs+x] = 1.0f;
			}
			else if ( x > 0 && y > 0 && pAltMap[yOffs-1000+x-1] <= 2 )
			{
				afHeights0[yOffs+x] = 1.0f;
			}
			else if ( x > 0 && y < 499 && pAltMap[yOffs+1000+x-1] <= 2 )
			{
				afHeights0[yOffs+x] = 1.0f;
			}
			else if ( x < 999 && y > 0 && pAltMap[yOffs-1000+x+1] <= 2 )
			{
				afHeights0[yOffs+x] = 1.0f;
			}
			else if ( x < 999 && y < 499 && pAltMap[yOffs+1000+x+1] <= 2 )
			{
				afHeights0[yOffs+x] = 1.0f;
			}
		}
	}
	//Now figure out how close we are to water and adjust the coastlines.
	for (y=0, yOffs=0; y<500; y++, yOffs+=1000)
	{
		for (x=0; x<1000; x++)
		{
			if (pAltMap[yy*1000+xx] <= 2)
			{
				continue;
			}

			for (int yy=y-3; yy<=y+3; yy++)
			{
				for (int xx=x-3; xx<=x+3; xx++)
				{
					if ( xx == x && yy == y ) continue;
					if ( yy < 0 || xx < 0 || yy > 499 || xx > 999 ) continue;
					if ( abs(xx-x) <= 1 && abs(yy-y)<=1 ) continue;

					if (pAltMap[yy*1000+xx] <= 2)
					{
						float dx = (float)xx - (float)x;
						float dy = (float)yy - (float)y;
						float d = sqrtf(dx*dx + dy*dy);
						float scale = (d-1.0f)*0.33f;
						afHeights0[yOffs+x] = scale*afHeights0[yOffs+x] + (1.0f-scale)*1.0f;
					}
				}
			}
		}
	}
	//Figure out how close we are to the coastline.
	//1. prime distance metric - either 0 for right next to land or 1.
	for (y=0, yOffs=0; y<500; y++, yOffs+=1000)
	{
		for (x=0; x<1000; x++)
		{
			//if land, just mark with 0.
			if (pAltMap[y*1000+x] > 2)
			{
				pCoastalDist[yOffs+x] = 0.0f;
			}
			else
			{
				pCoastalDist[yOffs+x] = -1.0f;
			}
		}
	}

	//2. iteratively compute the approximate minimum distance.
	static float afTemp[1000*500];
	float *pTemp = &afTemp[0];
	int passCnt = 32;
	float deltaDist = 1.0f / (float)passCnt;
	float *pRead  = pCoastalDist;
	float *pWrite = pTemp;
	for (int p=0; p<passCnt; p++)
	{
		for (y=0, yOffs=0; y<500; y++, yOffs+=1000)
		{
			for (x=0; x<1000; x++)
			{
				//if already filled in, continue
				if (pRead[yOffs+x] > -1)
				{
					//just copy over the value.
					pWrite[yOffs+x] = pRead[yOffs+x];
					continue;
				}

				//otherwise determine the iterative distance.
				float minDist = 1000.0f;
				int fx, fy;
				for (int yy=Math::Max(y-1,0); yy<=Math::Min(y+1,499); yy++)
				{
					for (int xx=Math::Max(x-1,0); xx<=Math::Min(x+1,999); xx++)
					{
						float d = pRead[yy*1000+xx];
						if ( d > -1 && d < minDist )
						{
							fx = xx;
							fy = yy;
							minDist = d;
						}
					}
				}
				if ( minDist < 1000.0f )
				{
					float fDistScale = 1.0f;
					if ( fx && fy && x < 500 )
					{
						//scale distance by diagonal...
						fDistScale = 1.41421356237f;
					}
					pWrite[yOffs+x] = Math::Min( minDist + deltaDist*fDistScale, 1.0f );
				}
				else
				{
					pWrite[yOffs+x] = -1.0f;
				}
			}
		}
		//swap buffers.
		float *pSwap = pRead;
		pRead  = pWrite;
		pWrite = pSwap;
	}

	//3. finally fill the rest with the maximum distance.
	for (y=0, yOffs=0; y<500; y++, yOffs+=1000)
	{
		for (x=0; x<1000; x++)
		{
			if (pCoastalDist[yOffs+x] < 0)
			{
				pCoastalDist[yOffs+x] = 1.0f;
			}
		}
	}
#endif
	//Write the data back... (or keep as float?)
	float max_height = 0.0f;
	for (y=0, yOffs=0; y<500; y++, yOffs+=1000)
	{
		for (x=0; x<1000; x++)
		{
			pAltMapF[yOffs+x] = afHeights0[yOffs+x]*64.0f;
			if ( pAltMapF[yOffs+x] > max_height ) { max_height = pAltMapF[yOffs+x]; }
		}
	}
#if 0
	//Generate the normalmap.
	for (y=0, yOffs=0; y<500; y++, yOffs+=1000)
	{
		for (x=0; x<1000; x++)
		{
			int xm1 = x-1; if ( xm1 < 0 ) xm1 = 0;
			int xp1 = x+1; if ( xp1 > 999) xp1 = 999;
			int ym1 = y-1; if ( ym1 < 0 ) ym1 = 0;
			int yp1 = y+1; if ( yp1 > 499) yp1 = 499;

			float s00 = pAltMapF[(ym1)*1000+xm1];
			float s01 = pAltMapF[(ym1)*1000+x];
			float s02 = pAltMapF[(ym1)*1000+xp1];

			float s10 = pAltMapF[(y)*1000+xm1];
			float s12 = pAltMapF[(y)*1000+xp1];

			float s20 = pAltMapF[(yp1)*1000+xm1];
			float s21 = pAltMapF[(yp1)*1000+x];
			float s22 = pAltMapF[(yp1)*1000+xp1];

			float sobelX = s00 + 2.0f*s10 + s20 - s02 - 2.0f*s12 - s22;
			float sobelY = s00 + 2.0f*s01 + s02 - s20 - 2.0f*s21 - s22;

			Vector3 nrml(sobelX, sobelY, 128.0f);
			nrml.Normalize();
			pvNrmlMap[y*1000+x] = nrml;
		}
	}
#endif
}

void Terrain::AddDynamicObject(uint32_t uID)
{
	m_pSector->AddObject( uID );
}

void Terrain::SetHeightmap(int32_t width, int32_t height, uint8_t *pData, float fScale, float fBias)
{
	m_nWidth  = width;
	m_nHeight = height;
	m_afHeightmap   = xlNew float[width*height];
	m_afCoastalDist = xlNew float[width*height];

	fScale *= (1.0f/255.0f);	//convert from 8 bit to float.
	for (int i=0; i<m_nWidth*m_nHeight; i++)
	{
		m_afHeightmap[i] = (float)pData[i] * fScale + fBias;
	}
}

//Whether the terrain is currently active or not.
//If its not active, then no rendering or collision takes place.
void Terrain::Activate(bool bActive)
{
	m_bActive = bActive;
}

//Update the terrain mesh and texturing based on the world position.
bool Terrain::Update(int32_t x, int32_t y, int32_t nRectCnt, LocationRect *pRects)
{
	bool bUpdateNeeded = false;
	if ( !m_bActive )
	{
		return false;
	}

	if ( !m_bMeshBuilt )
	{
		BinLocations();
		LoadHeightmap();
		BuildTerrainMeshes();
		GenTextureTileMapping();
		m_bMeshBuilt = true;
	}
	
	if ( (m_x != x || m_y != y) && x > -1 && y > -1 )
	{
		BuildHeightmap(x, y, m_x, m_y, nRectCnt, pRects);
		bUpdateNeeded = true;
	}
	Animate();
	m_pSector->m_x = m_x;
	m_pSector->m_y = m_y;

	if ( m_bShowTerrainDebug && !Input::IsKeyDown(XL_SHIFT) )
	{
		if ( Input::IsKeyDown(XL_LEFT) )
		{
			m_nTerrainMapX++;
		}
		else if ( Input::IsKeyDown(XL_RIGHT) )
		{
			m_nTerrainMapX--;
		}

		if ( Input::IsKeyDown(XL_UP) )
		{
			m_nTerrainMapY++;
		}
		else if ( Input::IsKeyDown(XL_DOWN) )
		{
			m_nTerrainMapY--;
		}
	}

	return bUpdateNeeded;
}

void Terrain::RenderSky(int32_t skyIndex, int32_t timeIndex, Camera *pCamera)
{
	m_pSkyLoader->LoadSky(skyIndex);
	SkyData *pSkyData = (SkyData *)m_pSkyLoader->GetSkyData(skyIndex);
	if ( !pSkyData )
		  return;

	pCamera->Compute(0.4f, true);
	pCamera->Set(m_pDriver, false);

	TextureLoader::SetPalette(8, pSkyData->aPalettes[timeIndex].colors, 768, 0);
	TextureLoader::SetColormap(4, pSkyData->aColormaps[timeIndex].data, pSkyData->aColormaps[timeIndex].lightLevels);

	m_pDriver->SetClearColorFromTex(pSkyData->ahTexEast[timeIndex]);
	m_pDriver->SetCurrentPalette(8, true);
	m_pDriver->SetCurrentColormap(4);
	m_pDriver->SetAmbient(1.0f);
	TextureHandle ahTex[] = { pSkyData->ahTexEast[timeIndex], pSkyData->ahTexWest[timeIndex], pSkyData->ahTexWest[timeIndex], pSkyData->ahTexWest[timeIndex] };

	//Render Sky Cylinder...
	const int32_t cylinderCnt = 32;

	float dA = MATH_TWO_PI / (float)cylinderCnt;
	float A = 0.0f;
	float x0 =  cosf(A);
	float y0 = -sinf(A);
	float x1, y1;
	A += dA;

	Vector3 posList[4];
	Vector2 uvList[4];

	uint32_t uSkyTexCnt = 4;

	uvList[0].Set( 0.01f, 0.99f );
	uvList[1].Set( 0.99f, 0.99f );
	uvList[2].Set( 0.99f, 0.01f );
	uvList[3].Set( 0.01f, 0.01f );

	float topScale = 0.5f;
	float cylTaper = 100.0f * topScale;

	const Vector3& vLoc = m_vCamLoc;
	float zTop = vLoc.z + m_fSkyTop*topScale;
	float zBot = vLoc.z + m_fSkyBot;

	uint32_t uDiv = cylinderCnt/uSkyTexCnt;
	float fWidth = 1.0f/(float)uDiv;
	for (uint32_t s=0; s<cylinderCnt; s++)
	{
		uint32_t idx = s%uDiv;
		float fStart = (float)idx * fWidth;

		uvList[0].x = fStart;
		uvList[1].x = fStart + fWidth;
		uvList[2].x = fStart + fWidth;
		uvList[3].x = fStart;

		x1 =  cosf(A);
		y1 = -sinf(A);

		posList[0].Set( vLoc.x+x0*100.0f, vLoc.y+y0*100.0f, zBot );
		posList[1].Set( vLoc.x+x1*100.0f, vLoc.y+y1*100.0f, zBot );
		posList[2].Set( vLoc.x+x1*cylTaper, vLoc.y+y1*cylTaper, zTop );
		posList[3].Set( vLoc.x+x0*cylTaper, vLoc.y+y0*cylTaper, zTop );

		TextureHandle hTex = ahTex[s/uDiv];
		m_pDriver->SetTexture(0, hTex);
		A += dA;

		x0 = x1;
		y0 = y1;

		m_pDriver->RenderWorldQuad(posList, uvList, Vector4::One);
	}

	m_pDriver->SetTexture(0, 0);
	pCamera->Compute(1.0f, true);
	pCamera->Set(m_pDriver);
	m_pDriver->SetAmbient(0.75f*195.0f/255.0f);
}

//Render the terrain.
void Terrain::Render(Camera *pCamera)
{
	if ( !m_bActive || !m_bMeshBuilt || m_x < 0 || m_y < 0 )
	{
		return;
	}

	m_vCamLoc = pCamera->GetLoc();
	m_vCamDir = pCamera->GetDir();
	
	if ( m_bShowTerrainDebug )
	{
		int32_t x = (m_nTerrainMapX-500)*m_nTerrainMapScale + 500;
		int32_t y = (m_nTerrainMapY-250)*m_nTerrainMapScale + 250;

		m_pDriver->AddOverlay(x, y, m_nTerrainMapScale, m_hDebugClimate);

		if ( m_x >= 0 && m_y >= 0 )
		{
			int32_t px = m_nTerrainMapX + ( (m_x+8)>>3 );
			int32_t py = m_nTerrainMapY + ( 499 - ((m_y-8)>>3) );

			px = (px-500)*m_nTerrainMapScale + 500 - 1;
			py = (py-250)*m_nTerrainMapScale + 250 - 1;

			m_pDriver->AddOverlay(px, py, 1, m_hPosMarker);
		}
	}
	else
	{
		m_pDriver->ForceMipmapping(false);

		int32_t skyIndex = GetSkyIndex(7, 7);
		m_pDriver->EnableFog(false);
		RenderSky(skyIndex, 20, pCamera);
		m_pDriver->Clear(false);
		m_pDriver->EnableFog(true, 8000.0f);

		//test.
		extern float m_fZRange;
		m_pDriver->ForceMipmapping(true);

		int32_t bEnable = 1;
		m_pDriver->SetExtension_Data( IDriver3D::EXT_GOURAUD, &bEnable, NULL);

		//m_fZRange = 8000.0f * 4.0f;
		//m_pDriver->EnableFog(true, 8000.0f * 4.0f);
		//RenderLOD(pCamera, 1);

		//m_pDriver->EnableFog(false);
		//m_pDriver->Clear(false);

		m_fZRange = 8000.0f;
		pCamera->SetMaxRenderDistance( 8000.0f );
		RenderLOD(pCamera, 0);

		//pCamera->SetMaxRenderDistance( 400.0f );
		m_pDriver->EnableFog(true, 400.0f);
		bEnable = 0;
		m_pDriver->SetExtension_Data( IDriver3D::EXT_GOURAUD, &bEnable, NULL);

		m_pDriver->ForceMipmapping(false);
	}
}

bool Terrain::IsPointInWater(float xPos, float yPos)
{
	xPos = (xPos - c_startPos.x) / 64.0f;
	yPos = (yPos - c_startPos.y) / 64.0f;

	if ( xPos < 0 ) xPos = 0;
	if ( yPos < 0 ) yPos = 0;
	if ( xPos > c_numVertexEdge-1) xPos = c_numVertexEdge-1;
	if ( yPos > c_numVertexEdge-1) yPos = c_numVertexEdge-1;

	//figure out the top-left vertex index
	int x = (int)floorf(xPos);
	int y = (int)floorf(yPos);

	bool bWater0 = m_LOD[0].m_pHM_Flags[y*c_numVertexEdge + x]!=0;
	bool bWater1 = m_LOD[0].m_pHM_Flags[y*c_numVertexEdge + x + 1]!=0;
	bool bWater2 = m_LOD[0].m_pHM_Flags[(y+1)*c_numVertexEdge + x]!=0;
	bool bWater3 = m_LOD[0].m_pHM_Flags[(y+1)*c_numVertexEdge + x + 1]!=0;

	return bWater0 && bWater1 && bWater2 && bWater3;
}

float Terrain::GetHeight_MapScale(int32_t x, int32_t y)
{
	if ( !m_bMeshBuilt )
	{
		BinLocations();
		LoadHeightmap();
		BuildTerrainMeshes();
		GenTextureTileMapping();
		m_bMeshBuilt = true;
	}

	x = Math::clamp(x, 0, 999);
	y = Math::clamp(499 - y, 0, 499);

	return m_afHeightmap[y*1000+x];
}

uint32_t Terrain::GetClimate_MapScale(int32_t x, int32_t y)
{
	if ( !m_bMeshBuilt )
	{
		BinLocations();
		LoadHeightmap();
		BuildTerrainMeshes();
		GenTextureTileMapping();
		m_bMeshBuilt = true;
	}

	x = Math::clamp(x, 0, 999);
	y = Math::clamp(499 - y, 0, 499);

	int32_t idx = (int)m_pClimate[y*1000+x]-223;
	assert( idx >= 0 && idx < 10 );
	return s_anMapClimate[idx];
}

float Terrain::GetHeight(float xPos, float yPos)
{
	xPos = (xPos - c_startPos.x) / 64.0f;
	yPos = (yPos - c_startPos.y) / 64.0f;

	if ( xPos < 0 ) xPos = 0;
	if ( yPos < 0 ) yPos = 0;
	if ( xPos > c_numVertexEdge-1) xPos = c_numVertexEdge-1;
	if ( yPos > c_numVertexEdge-1) yPos = c_numVertexEdge-1;

	//figure out the top-left vertex index
	int x = (int)floorf(xPos);
	int y = (int)floorf(yPos);
	//figure out the fractional component.
	float u = xPos - (float)x;
	float v = yPos - (float)y;

	//compute the height value from the 4 quad vertices.
	float h0 = m_LOD[0].m_pLocalHM[y*c_numVertexEdge + x];
	float h1 = m_LOD[0].m_pLocalHM[y*c_numVertexEdge + x + 1];
	float h2 = m_LOD[0].m_pLocalHM[(y+1)*c_numVertexEdge + x];
	float h3 = m_LOD[0].m_pLocalHM[(y+1)*c_numVertexEdge + x + 1];

	float hx0 = (1.0f - u)*h0 + u*h1;
	float hx1 = (1.0f - u)*h2 + u*h3;

	return (1.0f - v)*hx0 + v*hx1;
}

/*****************************************************
 ******************* INTERNAL ************************
 *****************************************************/
bool Terrain::SortCB_Chunks(Chunk*& d1, Chunk*& d2)
{
	if ( d1->dist < d2->dist )
		return true;

	return false;
}

void Terrain::RenderLOD(Camera *pCamera, int32_t lod)
{
	//Set Identity for now, later we'll have a real world matrix.
	m_pDriver->SetWorldMatrix( &Matrix::s_Identity, m_x, m_y );
	m_pDriver->EnableAlphaTest(false);

	if ( m_LOD[lod].m_pVB )
	{
		//The entire LOD uses 
		m_LOD[lod].m_pVB->Set();

		//for now just render all the chunks... later add proper culling.
		m_ChunkRenderList.clear();
		for (int i=0; i<CHUNK_COUNT; i++)
		{
			RenderChunk(pCamera, lod, i);
		}
		std::sort( m_ChunkRenderList.begin(), m_ChunkRenderList.end(), SortCB_Chunks );
		if ( m_pDriver->HasExtension( IDriver3D::EXT_TEXTURE_INDEX) )
		{
			for (int i=0; i<(int)m_ChunkRenderList.size(); i++)
			{
				m_pDriver->SetExtension_Data(IDriver3D::EXT_TEXTURE_INDEX, m_ahTex, m_ChunkRenderList[i]->m_TileTexArray);
				m_pDriver->RenderIndexedTriangles( m_ChunkRenderList[i]->m_pChunkIB, TILE_QUAD_COUNT*2 );
			}
		}
		else
		{
			for (int i=0; i<(int)m_ChunkRenderList.size(); i++)
			{
				m_pDriver->RenderIndexedTriangles( m_ChunkRenderList[i]->m_pChunkIB, TILE_QUAD_COUNT*2 );
			}
		}
	}

	m_pDriver->SetExtension_Data(IDriver3D::EXT_TEXTURE_INDEX, NULL, NULL);
}

void Terrain::RenderChunk(Camera *pCamera, int32_t lod, int32_t chunkNum)
{
	if ( lod > 0 || pCamera->AABBInsideFrustum(m_LOD[lod].m_aChunks[chunkNum].m_aChunkBounds.vMin, m_LOD[lod].m_aChunks[chunkNum].m_aChunkBounds.vMax, m_x, m_y)!=Camera::FRUSTUM_OUT )
	{
		m_LOD[lod].m_aChunks[chunkNum].dist = m_vCamDir.Dot(m_LOD[lod].m_aChunks[chunkNum].m_aChunkBounds.vCen - m_vCamLoc);
		m_ChunkRenderList.push_back( &m_LOD[lod].m_aChunks[chunkNum] );
	}
}

struct FoliageData
{
	TextureHandle hTex;
	Vector3 vScale;
};

FoliageData m_aFoliageData[32];
const uint32_t uPlantsOnAxis = 56;
uint32_t m_auObjID[uPlantsOnAxis*uPlantsOnAxis];
uint8_t  m_auIndex[uPlantsOnAxis*uPlantsOnAxis];

void Terrain::BuildTerrainMeshes()
{
	if ( !m_pSector )
	{
		m_pWorldCell = xlNew WorldCell();
		m_pWorldCell->SetWorldPos(m_x, m_y);
		m_pWorld->AddWorldCell( m_pWorldCell );

		m_pSector = xlNew Sector_GeoBlock();
		m_pSector->m_x = m_x;
		m_pSector->m_y = m_y;
		m_pSector->m_Bounds[0].Set( -7.0f*1024.0f, -7.0f*1024.0f, -32768.0f );
		m_pSector->m_Bounds[1].Set(  7.0f*1024.0f,  7.0f*1024.0f,  32768.0f );
		m_pSector->m_uTypeFlags = SECTOR_TYPE_EXTERIOR;
		m_pWorldCell->AddSector( m_pSector );

		//preload foliage textures.
		const char *szTexName = "TEXTURE.508";
		for (int32_t i=0; i<32; i++)
		{
			TextureHandle hTex = TextureCache::GameFile_LoadTexture_TexList( TEXTURETYPE_IMG, 7, ARCHIVETYPE_NONE, "", szTexName, i );
			float fw, fh;
			int32_t ox, oy;
			uint32_t w, h;
			TextureCache::GetTextureSize(ox, oy, w, h, fw, fh);
			int16_t *pSpriteScale = (int16_t *)TextureCache::GetTexExtraData();

			//sprite scale...
			int32_t newWidth  = w*(256+pSpriteScale[0])>>8;
			int32_t newHeight = h*(256+pSpriteScale[1])>>8;

			Vector3 vScale;
			vScale.x = (float)newWidth  / 8.0f;
			vScale.y = vScale.x;
			vScale.z = (float)newHeight / 8.0f;

			m_aFoliageData[i].hTex   = hTex;
			m_aFoliageData[i].vScale = vScale;
		}

		//now create the foliage objects...
		float yPos = -512.0f;
		for (int32_t y=0; y<uPlantsOnAxis; y++)
		{
			float xPos = -512.0f;
			for (int32_t x=0; x<uPlantsOnAxis; x++)
			{
				int32_t f = rand()&31;

				Object *pObj = ObjectManager::CreateObject("Sprite_ZAxis");
				pObj->SetWorldPos(m_x, m_y);
				
				uint32_t uObjID = pObj->GetID();
				m_pSector->AddObject( uObjID );
				m_auObjID[y*uPlantsOnAxis+x] = uObjID;
				m_auIndex[y*uPlantsOnAxis+x] = f;
				pObj->SetSector( m_pSector->m_uID );
				pObj->SetScale( m_aFoliageData[f].vScale );
				
				Vector3 vLoc;
				vLoc.x = xPos;
				vLoc.z = 0.0f;
				vLoc.y = yPos;
				pObj->SetLoc(vLoc);
				pObj->SetWorldPos(m_x, m_y);

				Sprite_ZAxis *pSprite = xlNew Sprite_ZAxis();
				pSprite->SetTextureHandle( m_aFoliageData[f].hTex );
				pObj->SetRenderComponent( pSprite );
				
				pObj->SetWorldBounds( vLoc - m_aFoliageData[f].vScale, vLoc + m_aFoliageData[f].vScale );
				pObj->SetBoundingSphere(vLoc, m_aFoliageData[f].vScale.Length());

				xPos += 128.0f;
				if ( xPos >= 512.0f )
					xPos -= 1024.0f;
			}
			yPos += 128.0f;
			if ( yPos >= 512.0f )
				 yPos -= 1024.0f;
		}
	}

	for (uint32_t l=0; l<LOD_COUNT; l++)
	{
		m_LOD[l].m_pVB = xlNew VertexBuffer(m_pDriver);
		m_LOD[l].m_pVB->Create(sizeof(TerrainVtx), c_numVertexEdge*c_numVertexEdge, true, IDriver3D::VBO_HAS_NORMALS | IDriver3D::VBO_HAS_TEXCOORDS | IDriver3D::VBO_WORLDSPACE);

		int32_t numQuads = TILE_QUAD_COUNT * CHUNK_COUNT;

		m_LOD[l].m_pGlobalIB = xlNew IndexBuffer(m_pDriver);
		m_LOD[l].m_pGlobalIB->Create( numQuads*6, sizeof(uint16_t), false );

		Vector3 pos;

		TerrainVtx *pVtx = (TerrainVtx *)m_LOD[l].m_pVB->Lock();
		assert(pVtx);
		if ( pVtx )
		{
			//for now build a single LOD.
			int vIndex = 0;
			pos.y = c_startPos.y;
			pos.z = c_startPos.z;

			//first generate vertices.
			for (int y=0; y<c_numVertexEdge; y++)
			{
				pos.x = c_startPos.x;
				for (int x=0; x<c_numVertexEdge; x++, vIndex++)
				{
					if ( l == 0 )
					{
						pVtx[vIndex].x = pos.x;
						pVtx[vIndex].y = pos.y;
						pVtx[vIndex].z = pos.z;
					}
					else
					{
						pVtx[vIndex].x = pos.x * 4.0f;
						pVtx[vIndex].y = pos.y * 4.0f;
						pVtx[vIndex].z = pos.z;
					}
					pVtx[vIndex].nx = 0;
					pVtx[vIndex].ny = 0;
					pVtx[vIndex].nz = 1;
					if ( l == 0 )
					{
						pVtx[vIndex].u = pos.x/64.0f;
						pVtx[vIndex].v = pos.y/64.0f;
					}
					else
					{
						pVtx[vIndex].u = pos.x/16.0f;
						pVtx[vIndex].v = pos.y/16.0f;
					}

					pos.x += 64.0f;
				}
				pos.y += 64.0f;
			}
		}
		m_LOD[l].m_pVB->Unlock();

		int chunkVtxIdx = 0;
		for (int y=0; y<CHUNK_TILE_WIDTH; y++)
		{
			for (int x=0; x<CHUNK_TILE_WIDTH; x++)
			{
				int chunkIndex = y*CHUNK_TILE_WIDTH+x;
				m_LOD[l].m_aChunks[chunkIndex].m_pChunkIB = xlNew IndexBuffer(m_pDriver);
				m_LOD[l].m_aChunks[chunkIndex].m_pChunkIB->Create( TILE_QUAD_WIDTH*TILE_QUAD_WIDTH*6, sizeof(uint16_t), false );

				uint16_t *pChunkIndices = (uint16_t *)m_LOD[l].m_aChunks[chunkIndex].m_pChunkIB->Lock();
				assert( pChunkIndices );
				if ( pChunkIndices )
				{
					int idxCnt = 0;
					for (int vy=0; vy<TILE_QUAD_WIDTH; vy++)
					{
						for (int vx=0; vx<TILE_QUAD_WIDTH; vx++)
						{
							pChunkIndices[idxCnt++] = (vy+y*TILE_QUAD_WIDTH  )*c_numVertexEdge + (vx+x*TILE_QUAD_WIDTH  );
							pChunkIndices[idxCnt++] = (vy+y*TILE_QUAD_WIDTH  )*c_numVertexEdge + (vx+x*TILE_QUAD_WIDTH+1);
							pChunkIndices[idxCnt++] = (vy+y*TILE_QUAD_WIDTH+1)*c_numVertexEdge + (vx+x*TILE_QUAD_WIDTH+1);

							pChunkIndices[idxCnt++] = (vy+y*TILE_QUAD_WIDTH  )*c_numVertexEdge + (vx+x*TILE_QUAD_WIDTH  );
							pChunkIndices[idxCnt++] = (vy+y*TILE_QUAD_WIDTH+1)*c_numVertexEdge + (vx+x*TILE_QUAD_WIDTH+1);
							pChunkIndices[idxCnt++] = (vy+y*TILE_QUAD_WIDTH+1)*c_numVertexEdge + (vx+x*TILE_QUAD_WIDTH  );
						}
					}
				}
				m_LOD[l].m_aChunks[chunkIndex].m_pChunkIB->Unlock();
			}
		}
	}

	//load textures.
	for (int rec=0; rec<56; rec++)
	{
		m_ahTex[rec    ] = TextureCache::GameFile_LoadTexture_TexList( TEXTURETYPE_IMG, 7, ARCHIVETYPE_NONE, "", "TEXTURE.002", rec );
		m_ahTex[rec+ 56] = TextureCache::GameFile_LoadTexture_TexList( TEXTURETYPE_IMG, 7, ARCHIVETYPE_NONE, "", "TEXTURE.102", rec );
		m_ahTex[rec+112] = TextureCache::GameFile_LoadTexture_TexList( TEXTURETYPE_IMG, 7, ARCHIVETYPE_NONE, "", "TEXTURE.302", rec );
		m_ahTex[rec+168] = TextureCache::GameFile_LoadTexture_TexList( TEXTURETYPE_IMG, 7, ARCHIVETYPE_NONE, "", "TEXTURE.402", rec );
	}
}

int32_t _climateTexOffs[]=
{
	112, 56, 0, 168
};

int32_t Terrain::GetSkyIndex(int x, int y)
{
	int xp = x + ((m_x-7)<<4);
	int yp = y + ((m_y-7)<<4);

	int px0 = Math::clamp(xp>>7, 0, 999);
	int py0 = Math::clamp(499 - (yp>>7), 0, 499);

	int idx = (int)m_pClimate[py0*1000+px0]-223;
	assert( idx >= 0 && idx < 10 );
	return m_anMapSky[idx];
}

int32_t Terrain::GetClimate(int x, int y, int *pnFlat/*=NULL*/)
{
	int xp = x + ((m_x-7)<<4);
	int yp = y + ((m_y-7)<<4);

	int px0 = Math::clamp(xp>>7, 0, 999);
	int py0 = Math::clamp(499 - (yp>>7), 0, 499);

	int idx = (int)m_pClimate[py0*1000+px0]-223;
	assert( idx >= 0 && idx < 10 );

	if ( pnFlat )
	{
		*pnFlat = s_anMapFlat[idx];
	}

	return s_anMapClimate[idx];
}

float Terrain::SampleCoastalDistance(int x, int y)
{
	int xp, yp;
	xp = x + ((m_x-7)<<4);
	yp = y + ((m_y-7)<<4);

	int px0 = xp>>7;
	int py0 = yp>>7;

	const float fOO128 = (1.0f/128.0f);
	float u = (float)(xp - (px0<<7)) * fOO128;
	float v = (float)(yp - (py0<<7)) * fOO128;
	assert(u >= 0.0f && u <= 1.0f);
	assert(v >= 0.0f && v <= 1.0f);

	py0 = 499 - py0;

	float d11 = m_afCoastalDist[Math::clamp(py0*m_nWidth,0,499) + Math::clamp(px0,0,999)];
	float d21 = m_afCoastalDist[Math::clamp(py0*m_nWidth,0,499) + Math::clamp(px0+1,0,999)];
	float d12 = m_afCoastalDist[Math::clamp(py0+1,0,499)*m_nWidth + Math::clamp(px0,0,999)];
	float d22 = m_afCoastalDist[Math::clamp(py0+1,0,499)*m_nWidth + Math::clamp(px0+1,0,999)];

	float dx0 = (1.0f-u)*d11 + u*d21;
	float dx1 = (1.0f-u)*d12 + u*d22;

	return (1.0f-v)*dx0 + v*dx1;
}

float Terrain::SampleBaseHeightmap(int lod, int x, int y)
{
#if 1
	int lodFactor = lod*2;

	int xp, yp;
	if ( lod == 0 )
	{
		xp = x + ((m_x-7)<<4);
		yp = y + ((m_y-7)<<4);
	}
	else
	{
		xp = (x<<2) + (m_x<<4) - 448;
		yp = (y<<2) + (m_y<<4) - 448;
	}

	int px0 = xp>>7;
	int py0 = yp>>7;

	const float fOO128 = (1.0f/128.0f);
	float u = (float)(xp - (px0<<7)) * fOO128;
	float v = 1.0f - (float)(yp - (py0<<7)) * fOO128;
	assert(u >= 0.0f && u <= 1.0f);
	assert(v >= 0.0f && v <= 1.0f);

	py0 = 499 - py0;

	float h[4][4];
	//Height
	h[0][0] = m_afHeightmap[Math::clamp(py0-1,0,499)*m_nWidth + Math::clamp(px0-1,0,999)];
	h[1][0] = m_afHeightmap[Math::clamp(py0-1,0,499)*m_nWidth + Math::clamp(px0,  0,999)];
	h[2][0] = m_afHeightmap[Math::clamp(py0-1,0,499)*m_nWidth + Math::clamp(px0+1,0,999)];
	h[3][0] = m_afHeightmap[Math::clamp(py0-1,0,499)*m_nWidth + Math::clamp(px0+2,0,999)];

	h[0][1] = m_afHeightmap[Math::clamp(py0,  0,499)*m_nWidth + Math::clamp(px0-1,0,999)];
	h[1][1] = m_afHeightmap[Math::clamp(py0,  0,499)*m_nWidth + Math::clamp(px0,  0,999)];
	h[2][1] = m_afHeightmap[Math::clamp(py0,  0,499)*m_nWidth + Math::clamp(px0+1,0,999)];
	h[3][1] = m_afHeightmap[Math::clamp(py0,  0,499)*m_nWidth + Math::clamp(px0+2,0,999)];

	h[0][2] = m_afHeightmap[Math::clamp(py0+1,0,499)*m_nWidth + Math::clamp(px0-1,0,999)];
	h[1][2] = m_afHeightmap[Math::clamp(py0+1,0,499)*m_nWidth + Math::clamp(px0,  0,999)];
	h[2][2] = m_afHeightmap[Math::clamp(py0+1,0,499)*m_nWidth + Math::clamp(px0+1,0,999)];
	h[3][2] = m_afHeightmap[Math::clamp(py0+1,0,499)*m_nWidth + Math::clamp(px0+2,0,999)];

	h[0][3] = m_afHeightmap[Math::clamp(py0+2,0,499)*m_nWidth + Math::clamp(px0-1,0,999)];
	h[1][3] = m_afHeightmap[Math::clamp(py0+2,0,499)*m_nWidth + Math::clamp(px0,  0,999)];
	h[2][3] = m_afHeightmap[Math::clamp(py0+2,0,499)*m_nWidth + Math::clamp(px0+1,0,999)];
	h[3][3] = m_afHeightmap[Math::clamp(py0+2,0,499)*m_nWidth + Math::clamp(px0+2,0,999)];

	return Math::BiCubic_Heightfield(h, u, v);
#else
	int px0 = (x<<2)%1000;
	int py0 = (y<<2)%500;

	return m_afHeightmap[py0*m_nWidth+px0];
#endif
}

void Terrain::ComputeNormal(int32_t x, int32_t y)
{
	Vector3 N(0,0,1);

	//compute the height value from the 4 quad vertices.
	int x0 = Math::Max( x-2, 0 );
	int x1 = Math::Min( x+2, c_numVertexEdge-1 );
	int y0 = Math::Max( y-2, 0 );
	int y1 = Math::Min( y+2, c_numVertexEdge-1 );

	float h0 = m_LOD[0].m_pLocalHM[y *c_numVertexEdge + x0];
	float h1 = m_LOD[0].m_pLocalHM[y *c_numVertexEdge + x1];
	float h2 = m_LOD[0].m_pLocalHM[y0*c_numVertexEdge + x ];
	float h3 = m_LOD[0].m_pLocalHM[y1*c_numVertexEdge + x ];

	N.Set( h0-h1, h2-h3, 6.0f );
	N = -N;
	N.Normalize();

	m_LOD[0].m_pLocalNM[y*c_numVertexEdge+x] = N;
}

int32_t ApplyFlip(int32_t value, int32_t flip)
{
	int32_t flip_value = value;
	if ( flip&1 )
	{
		int32_t A =  flip_value&3;
		int32_t B = (flip_value>>2)&3;
		int32_t C = (flip_value>>4)&3;
		int32_t D = (flip_value>>6)&3;

		flip_value  = B;
		flip_value |= A<<2;
		flip_value |= D<<4;
		flip_value |= C<<6;
	}
	if ( flip&2 )
	{
		int32_t A =  flip_value&3;
		int32_t B = (flip_value>>2)&3;
		int32_t C = (flip_value>>4)&3;
		int32_t D = (flip_value>>6)&3;

		flip_value  = C;
		flip_value |= D<<2;
		flip_value |= A<<4;
		flip_value |= B<<6;
	}
	if ( flip&4 )
	{
		int32_t A =  flip_value&3;
		int32_t B = (flip_value>>2)&3;
		int32_t C = (flip_value>>4)&3;
		int32_t D = (flip_value>>6)&3;

		flip_value  = A;
		flip_value |= C<<2;
		flip_value |= B<<4;
		flip_value |= D<<6;
	}

	return flip_value;
}

void Terrain::GenTextureTileMapping()
{
	//build the ground texture index array.
	//now go through all the possible combinations and match them to the correct texture + flip/rotation.
	for (int c=0; c<_comboCount; c++)
	{
		bool bFound = false;
		for (int w=0; w<_workSetSize; w++)
		{
			//first try a direct match.
			if ( c == _anWorkingSet[w] )
			{
				_anTileMapping[c] = w;
				bFound = true;
				break;
			}
			//next try the various flip/rotation combinations.
			static const int32_t _flipCombos[7] = { 1, 2, 4, 1|2, 1|4, 2|4, 1|2|4 };
			for (int f=0; f<7; f++)
			{
				if ( c == ApplyFlip(_anWorkingSet[w], _flipCombos[f]) )
				{
					//hacky... have to swap u and v flips when rotating.
					if ( (_flipCombos[f]&4) && (_flipCombos[f]&1) && !(_flipCombos[f]&2) )
						_anTileMapping[c] = w | ((4|2)<<8);
					else if ( (_flipCombos[f]&4) && (_flipCombos[f]&2) && !(_flipCombos[f]&1) )
						_anTileMapping[c] = w | ((4|1)<<8);
					else	//otherwise the proper value works fine.
						_anTileMapping[c] = w | (_flipCombos[f]<<8);
					bFound = true;
					break;
				}
			}
		}
		//if no mapping is found, just pick a default based on the top left corner.
		if ( bFound == false )
		{
			int32_t c0 = c&3, c1 = (c>>2)&3, c2 = (c>>4)&3, c3 = (c>>6)&3;
			int32_t ct0 = 1 + (c1 == c0 ? 1 : 0) + (c2 == c0 ? 1 : 0) + (c3 == c0 ? 1 : 0);
			int32_t ct1 = 1 + (c0 == c1 ? 1 : 0) + (c2 == c1 ? 1 : 0) + (c3 == c1 ? 1 : 0);
			int32_t ct2 = 1 + (c0 == c2 ? 1 : 0) + (c1 == c2 ? 1 : 0) + (c3 == c2 ? 1 : 0);
			int32_t ct3 = 1 + (c0 == c3 ? 1 : 0) + (c1 == c3 ? 1 : 0) + (c2 == c3 ? 1 : 0);

			if ( ct0 >= ct1 && ct0 >= ct2 && ct0 >= ct3 )
				_anTileMapping[c] = c0;
			else if ( ct1 >= ct0 && ct1 >= ct2 && ct1 >= ct3 )
				_anTileMapping[c] = c1;
			else if ( ct2 >= ct0 && ct2 >= ct1 && ct2 >= ct3 )
				_anTileMapping[c] = c2;
			else
				_anTileMapping[c] = c3;
		}
	}
}

void Terrain::Animate()
{
	Vector3 procStart( (float)m_x, (float)m_y, 0.0f );
	Vector3 pos = c_startPos;

	const float fOO1024  = (1.0f/1024.0f);
	const float fOO200   = (1.0f/200.0f);
	static float time = 0.0f;
	time += (1.0f/60.0f)*0.2f;

	int32_t vIndexY = 0;
	pos.y = c_startPos.y;
	float largeWaveScaleBlend = 1.0f / 200.0f;
	float smallWaveScale = 16.0f;
	float fDeadZone = (2.0f/32.0f);
	for (int y=0; y<c_numVertexEdge; y++)
	{
		int32_t vIndex = vIndexY;
		pos.x = c_startPos.x;
		for (int x=0; x<c_numVertexEdge; x++, vIndex++)
		{
			if ( m_LOD[0].m_pHM_Flags[vIndex] )
			{
				Vector3 proc_pos(pos.x*fOO1024 + procStart.x, pos.y*fOO1024 + procStart.y, time);
				float posScale = 8.0f;
				float ampScale = smallWaveScale;
				/*if ( m_LOD[0].m_pHM_Flags[vIndex] > fDeadZone )
				{
					float largeWaveBlend = (m_LOD[0].m_pHM_Flags[vIndex]-fDeadZone);
					posScale = (1.0f-largeWaveBlend)*posScale + largeWaveBlend*0.00001f;
					ampScale = (1.0f-largeWaveBlend)*ampScale + largeWaveBlend*128.0f;
				}*/
				proc_pos.x = proc_pos.x*posScale + time;
				proc_pos.y = proc_pos.y*posScale - time;

				m_LOD[0].m_pLocalHM[vIndex] = ProceduralFunc::fBm(proc_pos, 1)*ampScale;
			}

			pos.x += 64.0f;
		}
		pos.y += 64.0f;
		vIndexY += c_numVertexEdge;
	}

	//now update the vertex buffer.
	TerrainVtx *pVtx  = (TerrainVtx *)m_LOD[0].m_pVB->Lock();
	assert(pVtx);
	if ( pVtx )
	{
		for (int v=0; v<c_numVertexEdge*c_numVertexEdge; v++)
		{
			pVtx[v].z  = m_LOD[0].m_pLocalHM[v];
		}
	}
	m_LOD[0].m_pVB->Unlock();
}

void Terrain::BuildHeightmap(int32_t newX, int32_t newY, int32_t prevX, int32_t prevY, int32_t nRectCnt, LocationRect *pRects)
{
	//for now build a single LOD.
	int vIndex = 0;

	m_pSector->m_x = newX;
	m_pSector->m_y = newY;
	m_pWorldCell->SetWorldPos(newX, newY);
	
	//build the heightmap for the area.
	const float fOO1024  = (1.0f/1024.0f);
	const float fOO8k    = (1024.0f/8192.0f);
	const float fOO64k   = (1024.0f/65536.0f);
	const float angle = 0.753f;
	const float ca = cosf(angle);
	const float sa = sinf(angle);
	static uint8_t _auGroundMap[c_numVertexEdge*c_numVertexEdge];
	static uint8_t _auGroundMapTmp[c_numVertexEdge*c_numVertexEdge];
	static uint16_t _auGroundMapOverride[c_numVertexEdge*c_numVertexEdge];

	float fZ = 0.0f;
	Vector3 procStart( (float)newX, (float)newY, 0.0f );
	Vector3 pos = c_startPos;

	//first copy the part of the heightmap that stays the same but moves.
	static float *pTmpBuffer = NULL;
	static Vector3 *pTmpV3Buffer = NULL;
	static float *pTmpFltBuffer = NULL;
	int32_t bufferSize = (CHUNK_TILE_WIDTH*TILE_QUAD_WIDTH+1) * (CHUNK_TILE_WIDTH*TILE_QUAD_WIDTH+1);
	if ( !pTmpBuffer )
	{
		pTmpBuffer   = xlNew float[bufferSize];
		pTmpV3Buffer = xlNew Vector3[bufferSize];
		pTmpFltBuffer = xlNew float[bufferSize];
	}
	memcpy(pTmpBuffer, m_LOD[0].m_pLocalHM, bufferSize*sizeof(float));
	memcpy(pTmpV3Buffer, m_LOD[0].m_pLocalNM, bufferSize*sizeof(Vector3));
	memcpy(pTmpFltBuffer, m_LOD[0].m_pHM_Flags, bufferSize*sizeof(float));
	memcpy(_auGroundMapTmp, _auGroundMap, c_numVertexEdge*c_numVertexEdge);

	int32_t dx = newX-prevX;
	int32_t dy = newY-prevY;

	int32_t updateRegionCnt;
	int32_t updateRegionX0[4];
	int32_t updateRegionY0[4];
	int32_t updateRegionW[4];
	int32_t updateRegionH[4];

	if ( (dx || dy) && (abs(dx) <= 1 && abs(dy) <=1) )
	{
		int32_t startX = dx < 0 ? -dx*TILE_QUAD_WIDTH : 0;
		int32_t startY = dy < 0 ? -dy*TILE_QUAD_WIDTH : 0;

		int32_t srcY = (dy > 0 ? dy*TILE_QUAD_WIDTH : 0);
		for (int32_t y=startY; y<startY+c_numVertexEdge-abs(dy)*TILE_QUAD_WIDTH; y++, srcY++)
		{
			int32_t srcX = (dx > 0 ? dx*TILE_QUAD_WIDTH : 0);
			for (int32_t x=startX; x<startX+c_numVertexEdge-abs(dx)*TILE_QUAD_WIDTH; x++, srcX++)
			{
				m_LOD[0].m_pLocalHM[  y*c_numVertexEdge+x]  = pTmpBuffer[srcY*c_numVertexEdge+srcX];
				m_LOD[0].m_pLocalNM[  y*c_numVertexEdge+x]  = pTmpV3Buffer[srcY*c_numVertexEdge+srcX];
				m_LOD[0].m_pHM_Flags[  y*c_numVertexEdge+x] = pTmpFltBuffer[srcY*c_numVertexEdge+srcX];
				_auGroundMap[y*c_numVertexEdge+x] = _auGroundMapTmp[srcY*c_numVertexEdge+srcX];
			}
		}

		//update the "L" region that is changing.
		//generate the "x" update region.
		updateRegionCnt = 0;
		if ( dx )
		{
			updateRegionX0[updateRegionCnt] = dx > 0 ? TILE_QUAD_WIDTH*(CHUNK_TILE_WIDTH-1) : 0;
			updateRegionW [updateRegionCnt] = TILE_QUAD_WIDTH+1;
			updateRegionY0[updateRegionCnt] = 0;
			updateRegionH [updateRegionCnt] = c_numVertexEdge;
			updateRegionCnt++;
		}
		//generate the "y" update region.
		if ( dy )
		{
			updateRegionX0[updateRegionCnt] = 0;
			updateRegionW [updateRegionCnt] = c_numVertexEdge;
			updateRegionY0[updateRegionCnt] = dy > 0 ? TILE_QUAD_WIDTH*(CHUNK_TILE_WIDTH-1) : 0;
			updateRegionH [updateRegionCnt] = TILE_QUAD_WIDTH+1;
			updateRegionCnt++;
		}
	}
	else
	{
		updateRegionCnt = 1;
		updateRegionX0[0] = 0;
		updateRegionY0[0] = 0;

		updateRegionW[0]  = c_numVertexEdge;
		updateRegionH[0]  = c_numVertexEdge;
	}

	const float coastalDeadZone = 1.0f/32.0f;
	m_x = newX;
	m_y = newY;

	//before updating check to see if there are any overlapping locations.
	//if so, generate a mask - height + strength
	//where height is the desired height for the location and strength is how much the computed height is
	//blended towards the desired height.
	static float locHeight[c_numVertexEdge*c_numVertexEdge];
	static float locBlend[ c_numVertexEdge*c_numVertexEdge];
	memset(locHeight, 0, sizeof(float)*c_numVertexEdge*c_numVertexEdge);
	memset(locBlend,  0, sizeof(float)*c_numVertexEdge*c_numVertexEdge);
	memset(_auGroundMapOverride, 0xffffffff, sizeof(uint16_t)*c_numVertexEdge*c_numVertexEdge);

	//handle incoming location "rects" - initially just the inner.
	for (int32_t r=0; r<nRectCnt; r++)
	{
		LocationRect *cRect = &pRects[r];
		//first clip the rect to the terrain area.
		int32_t innerRect[4];
		int32_t outerRect[4];
		innerRect[0] = Math::Max( (int32_t)cRect->vRectInner.x, m_x-7 );
		innerRect[1] = Math::Max( (int32_t)cRect->vRectInner.y, m_y-7 );
		innerRect[2] = Math::Min( (int32_t)cRect->vRectInner.z, m_x+8 );
		innerRect[3] = Math::Min( (int32_t)cRect->vRectInner.w, m_y+8 );

		outerRect[0] = Math::Max( (int32_t)cRect->vRectOuter.x, m_x-7 );
		outerRect[1] = Math::Max( (int32_t)cRect->vRectOuter.y, m_y-7 );
		outerRect[2] = Math::Min( (int32_t)cRect->vRectOuter.z, m_x+8 );
		outerRect[3] = Math::Min( (int32_t)cRect->vRectOuter.w, m_y+8 );

		//next modify all tiles in the inner rect so that the height value = location height.
		const float fBlendScale = 1.0f/64.0f;
		for (int32_t ty=outerRect[1]; ty<outerRect[3]; ty++)
		{
			int32_t y0 = (ty-(m_y-7))*TILE_QUAD_WIDTH;
			for (int32_t tx=outerRect[0]; tx<outerRect[2]; tx++)
			{
				int32_t x0 = (tx-(m_x-7))*TILE_QUAD_WIDTH;
				//Inner rect, just set blend to 1.0
				if ( ty >= innerRect[1] && ty < innerRect[3] && tx >= innerRect[0] && tx < innerRect[2] )
				{
					for (int32_t y=y0; y<=y0+TILE_QUAD_WIDTH; y++)
					{
						for (int32_t x=x0; x<=x0+TILE_QUAD_WIDTH; x++)
						{
							int32_t vy = y*c_numVertexEdge;
							locHeight[vy + x] = cRect->fHeight;
							locBlend[ vy + x] = 1.0f;
						}
					}
				}
				else
				{
					//outer rect, blend.
					//First do X blend.
					float _xBlend[17*17];
					if ( tx < innerRect[0] )
					{
						for (int32_t y=y0; y<=y0+TILE_QUAD_WIDTH; y++)
						{
							float xOuterEdge = (float)( ((int32_t)cRect->vRectOuter.x - (m_x-7))*16 );
							for (int32_t x=x0; x<=x0+TILE_QUAD_WIDTH; x++)
							{
								float xBlend = Math::clamp( ( (float)x-xOuterEdge ) * fBlendScale, 0.0f, 1.0f );
								int32_t vy = y*c_numVertexEdge;
								if ( locBlend[vy+x] < 1.0f || locHeight[vy+x] == cRect->fHeight )
								{
									locHeight[vy + x] = cRect->fHeight;
									if ( ty >= innerRect[1] && ty < innerRect[3] )
									{
										locBlend[ vy + x] = xBlend;
									}
									else
									{
										_xBlend[(y-y0)*17+(x-x0)] = xBlend;
									}
								}
							}
						}
					}
					else if ( tx >= innerRect[2] )
					{
						for (int32_t y=y0; y<=y0+TILE_QUAD_WIDTH; y++)
						{
							float xOuterEdge = (float)( ((int32_t)cRect->vRectOuter.z - (m_x-7))*16 );
							for (int32_t x=x0; x<=x0+TILE_QUAD_WIDTH; x++)
							{
								float xBlend = Math::clamp( ( xOuterEdge - (float)x ) * fBlendScale, 0.0f, 1.0f );
								int32_t vy = y*c_numVertexEdge;
								if ( locBlend[vy+x] < 1.0f || locHeight[vy+x] == cRect->fHeight )
								{
									locHeight[vy + x] = cRect->fHeight;
									if ( ty >= innerRect[1] && ty < innerRect[3] )
									{
										locBlend[ vy + x] = xBlend;
									}
									else
									{
										_xBlend[(y-y0)*17+(x-x0)] = xBlend;
									}
								}
							}
						}
					}

					//Then do Y blend.
					if ( ty < innerRect[1] )
					{
						for (int32_t y=y0; y<=y0+TILE_QUAD_WIDTH; y++)
						{
							float yOuterEdge = (float)( ((int32_t)cRect->vRectOuter.y - (m_y-7))*16 );
							float yBlend = Math::clamp( ( (float)y-yOuterEdge ) * fBlendScale, 0.0f, 1.0f );
							for (int32_t x=x0; x<=x0+TILE_QUAD_WIDTH; x++)
							{
								int32_t vy = y*c_numVertexEdge;
								if ( locBlend[vy+x] < 1.0f || locHeight[vy+x] == cRect->fHeight )
								{
									locHeight[vy + x] = cRect->fHeight;
									if ( tx >= innerRect[0] && tx < innerRect[2] )
										locBlend[ vy + x] = yBlend;//0.0f;//locBlend[ vy + x] ? locBlend[ vy + x]*yBlend : yBlend;
									else
										locBlend[ vy + x] = yBlend * _xBlend[(y-y0)*17+(x-x0)];
								}
							}
						}
					}
					else if ( ty >= innerRect[3] )
					{
						for (int32_t y=y0; y<=y0+TILE_QUAD_WIDTH; y++)
						{
							float yOuterEdge = (float)( ((int32_t)cRect->vRectOuter.w - (m_y-7))*16 );
							float yBlend = Math::clamp( ( yOuterEdge - (float)y ) * fBlendScale, 0.0f, 1.0f );
							for (int32_t x=x0; x<=x0+TILE_QUAD_WIDTH; x++)
							{
								int32_t vy = y*c_numVertexEdge;
								if ( locBlend[vy+x] < 1.0f || locHeight[vy+x] == cRect->fHeight )
								{
									locHeight[vy + x]  = cRect->fHeight;
									if ( tx >= innerRect[0] && tx < innerRect[2] )
										locBlend[ vy + x] = yBlend;//locBlend[ vy + x] ? locBlend[ vy + x]*yBlend : yBlend;
									else
										locBlend[ vy + x] = yBlend * _xBlend[(y-y0)*17+(x-x0)];
								}
							}
						}
					}
				}
			}
		}
	}

	//then generate the parts that are new.
	for (int32_t r=0; r<updateRegionCnt; r++)
	{
		int32_t vIndexY = updateRegionY0[r]*c_numVertexEdge + updateRegionX0[r];
		pos.y = c_startPos.y + 64.0f*(float)updateRegionY0[r];
		for (int y=updateRegionY0[r]; y<updateRegionY0[r]+updateRegionH[r]; y++)
		{
			vIndex = vIndexY;
			pos.x = c_startPos.x + 64.0f*(float)updateRegionX0[r];
			for (int x=updateRegionX0[r]; x<updateRegionX0[r]+updateRegionW[r]; x++, vIndex++)
			{
				{
					//large scale - from the Daggerfall heightmap itself.
					float baseHM = SampleBaseHeightmap(0,x,y);
					m_LOD[0].m_pLocalHM[vIndex] = baseHM;

					float fScaleHeight = baseHM;
					fScaleHeight = fScaleHeight*(1.0f-locBlend[vIndex]) + locHeight[vIndex]*locBlend[vIndex];
					float scale0 = Math::clamp( fScaleHeight / 128.0f,  0.0f, 1.0f );
					float scale1 = 0.5f*Math::clamp( fScaleHeight / 512.0f,  0.0f, 1.0f );
					float scale2 = 0.5f*Math::clamp( fScaleHeight / 2048.0f, 0.0f, 1.0f );

					if ( GetClimate(x, y) == Terrain::CLIMATE_MOUNTAIN )
					{
						scale1 *= 2.0f;
						scale2 *= 1.5f;
					}

					//Procedural pos...
					Vector3 proc_pos(pos.x*fOO1024 + procStart.x, pos.y*fOO1024 + procStart.y, pos.z);

					//small scale - "rolling hills" (fBm)
					Vector3 p(proc_pos.y, proc_pos.x, 0.754f);
					m_LOD[0].m_pLocalHM[vIndex] += ProceduralFunc::fBm(p, 3, 0.49f, 1.98f+scale2*0.1f)*64.0f*scale0;

					//medium scale - "ridges and plateus" (Ridged Multi-Fractal)
					p.Set( proc_pos.x*fOO8k+0.777f, proc_pos.y*fOO8k+0.335f, 0.33f );
					Vector3 p2( ca*p.x + sa*p.y, sa*p.x - ca*p.y, p.z);
					m_LOD[0].m_pLocalHM[vIndex] += ProceduralFunc::fBm(p2, 5, 0.535f, 2.2f)*512.0f*scale1;

					//large scale - "ridges and plateus" (Ridged Multi-Fractal)
					p.Set( proc_pos.y*fOO64k*2.0f+0.657f, proc_pos.x*fOO64k*2.0f+0.625f, 0.5f );
					p2.Set( ca*p.x + sa*p.y, sa*p.x - ca*p.y, p.z);
					m_LOD[0].m_pLocalHM[vIndex] += ProceduralFunc::fBm(p2, 5, 0.523f, 2.1f)*1024.0f*scale2;

					//generate the ground type per-vertex.
					const float fWaterHeight = 0.0001f;
					if ( baseHM+locBlend[vIndex]*locHeight[vIndex] < fWaterHeight )
					{
						_auGroundMap[vIndex] = 0;
						m_LOD[0].m_pLocalHM[vIndex] = 0.0f;

						float cdist = SampleCoastalDistance(x,y);
						m_LOD[0].m_pHM_Flags[vIndex] = coastalDeadZone + cdist;
					}
					else
					{
						m_LOD[0].m_pLocalHM[vIndex] = (1.0f-locBlend[vIndex])*m_LOD[0].m_pLocalHM[vIndex] + (locBlend[vIndex])*locHeight[vIndex];

						p.Set(proc_pos.x*2.0f, proc_pos.y*2.0f, 0.5f);
						float noise = ProceduralFunc::fBm(p, 4)*0.5f+0.5f;
						_auGroundMap[vIndex] = Math::clamp( (int32_t)(noise*3.0f+1.0f), 1, 3 );
						m_LOD[0].m_pHM_Flags[vIndex] = 0;
					}
				}

				pos.x += 64.0f;
				fZ += 0.001f;
			}
			pos.y += 64.0f;
			vIndexY += c_numVertexEdge;
		}
	}

	//update foliage...
	//now create the foliage objects...
	int32_t vIndexY = 4*16;
	int32_t vIndexX = 4*16;
	for (int32_t y=0; y<uPlantsOnAxis; y++)
	{
		int32_t vOffsetY = vIndexY*c_numVertexEdge;
		for (int32_t x=0; x<uPlantsOnAxis; x++)
		{
			Object *pObj = ObjectManager::GetObjectFromID( m_auObjID[y*uPlantsOnAxis+x] );
			
			int32_t vIndex = vOffsetY + vIndexX + x*2;

			float h = m_LOD[0].m_pLocalHM[ vIndex ];
			uint8_t f = (int32_t)(h * 32.0f) & 31;
			uint8_t prob = (int32_t)(h * 100.0f)%100;

			if ( locBlend[vIndex] < 1.0f && _auGroundMap[vIndex] != 0 && prob > 50 )
			{
				Sprite_ZAxis *pSpriteData = (Sprite_ZAxis *)pObj->GetRenderComponent();
				pSpriteData->SetTextureHandle( m_aFoliageData[f].hTex );

				Vector3 vLoc;
				pObj->GetLoc(vLoc);
				vLoc.z = h + m_aFoliageData[f].vScale.z;
				pObj->SetLoc(vLoc);
				pObj->SetWorldPos(newX-3+(x>>3), newY-3+(y>>3));
		
				pObj->SetScale( m_aFoliageData[f].vScale );
				pObj->SetWorldBounds( vLoc - m_aFoliageData[f].vScale, vLoc + m_aFoliageData[f].vScale );
				pObj->SetBoundingSphere(vLoc, m_aFoliageData[f].vScale.Length());

				pObj->SetFlag( Object::OBJFLAGS_ACTIVE );
			}
			else
			{
				pObj->ClearFlag( Object::OBJFLAGS_ACTIVE );
			}
		}
		vIndexY+=2;
	}

	//generate LOD 1...
	vIndexY = 0;
	pos.y = c_startPos.y*4.0f;
	for (int y=0; y<c_numVertexEdge; y++)
	{
		vIndex = vIndexY;
		pos.x = c_startPos.x*4.0f;
		for (int x=0; x<c_numVertexEdge; x++, vIndex++)
		{
			//large scale - from the Daggerfall heightmap itself.
			float baseHM = SampleBaseHeightmap(1,x,y);
			m_LOD[1].m_pLocalHM[vIndex] = baseHM;

			pos.x += 64.0f;
		}

		pos.y += 64.0f;
		vIndexY += c_numVertexEdge;
	}

	//finally generate the vertex normals for regions that need updating.
	for (int32_t r=0; r<updateRegionCnt; r++)
	{
		for (int y=updateRegionY0[r]; y<updateRegionY0[r]+updateRegionH[r]; y++)
		{
			for (int x=updateRegionX0[r]; x<updateRegionX0[r]+updateRegionW[r]; x++, vIndex++)
			{
				ComputeNormal(x, y);
			}
		}
	}

	//Build the texture index array for each chunk.
	for (int y=0; y<CHUNK_TILE_WIDTH; y++)
	{
		for (int x=0; x<CHUNK_TILE_WIDTH; x++)
		{
			m_LOD[0].m_aChunks[y*CHUNK_TILE_WIDTH+x].m_aChunkBounds.vMin.Set( c_startPos.x + (float)x*1024.0f, c_startPos.y + (float)y*1024.0f, 1000000.0f );
			m_LOD[0].m_aChunks[y*CHUNK_TILE_WIDTH+x].m_aChunkBounds.vMax.Set( c_startPos.x + (float)(x+1)*1024.0f, c_startPos.y + (float)(y+1)*1024.0f, -1000000.0f );
			//m_pDriver->ResetIBFlags( m_LOD[0].m_aChunks[y*CHUNK_TILE_WIDTH+x].m_pChunkIB->GetID() );
		}
	}

	for (int y=0, yOffs=0; y<c_numVertexEdge-1; y++, yOffs+=c_numVertexEdge)
	{
		int32_t chunkY = (y>>4)*CHUNK_TILE_WIDTH;
		int32_t quadY  = (y&15)*TILE_QUAD_WIDTH;
		for (int x=0; x<c_numVertexEdge-1; x++)
		{
			int32_t v0 = _auGroundMap[ yOffs + x ];
			int32_t v1 = _auGroundMap[ yOffs + x + 1 ];
			int32_t v2 = _auGroundMap[ yOffs + c_numVertexEdge + x ];
			int32_t v3 = _auGroundMap[ yOffs + c_numVertexEdge + x + 1 ];

			int32_t lOcc = E(v0, v1, v2, v3);
			int32_t climateTexOffs = _climateTexOffs[GetClimate(x,y)];
			int32_t chunk = chunkY + (x>>4);
			int32_t quad  = quadY  + (x&15);

			m_LOD[0].m_aChunks[chunk].m_TileTexArray[quad] = _anTileMapping[lOcc] + climateTexOffs;
			assert( ( (m_LOD[0].m_aChunks[chunk].m_TileTexArray[quad])&0xff ) < (56*4) );

			if ( _anTileMapping[lOcc] != 0 )
			{
				m_LOD[0].m_pHM_Flags[yOffs + x] = 0;
				m_LOD[0].m_pHM_Flags[yOffs + x + 1] = 0;
				m_LOD[0].m_pHM_Flags[yOffs + c_numVertexEdge + x ] = 0;
				m_LOD[0].m_pHM_Flags[yOffs + c_numVertexEdge + x + 1 ] = 0;
			}

			m_LOD[1].m_aChunks[chunk].m_TileTexArray[quad] = _anTileMapping[lOcc] + climateTexOffs;

			//check vertex heights...
			float z = m_LOD[0].m_pLocalHM[yOffs + x];
			if ( z < m_LOD[0].m_aChunks[chunk].m_aChunkBounds.vMin.z )
			{
				m_LOD[0].m_aChunks[chunk].m_aChunkBounds.vMin.z = z;
			}
			if ( z > m_LOD[0].m_aChunks[chunk].m_aChunkBounds.vMax.z )
			{
				m_LOD[0].m_aChunks[chunk].m_aChunkBounds.vMax.z = z;
			}
		}
	}

	//extra padding for animation.
	for (int i=0; i<CHUNK_COUNT; i++)
	{
		m_LOD[0].m_aChunks[i].m_aChunkBounds.vMin.z -= 16.0f;
		m_LOD[0].m_aChunks[i].m_aChunkBounds.vMax.z += 16.0f;

		m_LOD[0].m_aChunks[i].m_aChunkBounds.vCen = (m_LOD[0].m_aChunks[i].m_aChunkBounds.vMin + m_LOD[0].m_aChunks[i].m_aChunkBounds.vMax) * 0.5f;
		m_pDriver->ResetIBFlags( m_LOD[0].m_aChunks[i].m_pChunkIB->GetID() );
	}
	

	//now update the vertex buffer.
	TerrainVtx *pVtx  = (TerrainVtx *)m_LOD[0].m_pVB->Lock();
	TerrainVtx *pVtx1 = (TerrainVtx *)m_LOD[1].m_pVB->Lock();
	assert(pVtx);
	if ( pVtx )
	{
		for (int v=0; v<c_numVertexEdge*c_numVertexEdge; v++)
		{
			pVtx[v].z  = m_LOD[0].m_pLocalHM[v];
			pVtx[v].nx = m_LOD[0].m_pLocalNM[v].x;
			pVtx[v].ny = m_LOD[0].m_pLocalNM[v].y;
			pVtx[v].nz = m_LOD[0].m_pLocalNM[v].z;

			pVtx1[v].z = m_LOD[1].m_pLocalHM[v];
		}
	}
	m_LOD[0].m_pVB->Unlock();
	m_LOD[1].m_pVB->Unlock();
}

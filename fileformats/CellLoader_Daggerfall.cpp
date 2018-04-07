#include "CellLoader_Daggerfall.h"
#include "../world/World.h"
#include "../world/WorldCell.h"
#include "../world/ObjectManager.h"
#include "../world/Object.h"
#include "../world/OrientedSprite.h"
#include "../world/Sprite_ZAxis.h"
#include "../world/MeshCollision.h"
#include "../world/LogicManager.h"
#include "../world/Terrain.h"
#include "../render/MeshCache.h"
#include "../render/Mesh.h"
#include "../render/TextureCache.h"

#include "../fileformats/TextureTypes.h"
#include "../fileformats/ArchiveTypes.h"
#include "../fileformats/ArchiveManager.h"
#include "../fileformats/Location_Daggerfall.h"
#include "../world/LevelFuncMgr.h"
#include "../world/Sector_GeoBlock.h"

#include "../memory/ScratchPad.h"

#include "../math/Math.h"
#include "../ui/XL_Console.h"
#include "Parser.h"

#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <float.h>
#include "../os/Clock.h"

#include <vector>

using namespace std;

struct MeshAction
{
	int nParentID;	//-1 = no parent.
	int nTargetID;	//-1 = no target.
	int nType;
	int nAxis;
	float Duration;
	float Delta;

	float origPos[3];
	float origAng[3];

	//animation
	float fDelta;	//which direction to move in and how much per step.
	float fAnim;	//current animation state.
	int   bAnim;	//is currently animating?
};

#pragma pack(push)
#pragma pack(1)

//Exterior
struct BlockPos
{
	int32_t XPos1;
	int32_t YPos1;
	int32_t XPos2;
	int32_t YPos2;
	int32_t Angle;
};

struct BlockHeader
{
	uint8_t  Num3DObjRec;
	uint8_t  NumFlatObjRec;
	uint8_t  NumSection3Rec;
	uint8_t  NumPeopleRec;
	uint8_t  NumDoorRec;
	int16_t Unknown[6];
};

struct Block3DObj
{
	int16_t ObjID1;
	int8_t  ObjID2;
	int8_t  Unknown1;
	int32_t Unknown2[5];
	int32_t XPos1;
	int32_t YPos1;
	int32_t ZPos1;
	int32_t XPos2;
	int32_t YPos2;
	int32_t ZPos2;
	int32_t NullValue;
	int16_t Angle;
	int16_t Unknown3;
	int32_t NullValue2;
	int32_t Unknown4;
	int16_t NullValue3;
};

struct BuildingData
{
	uint16_t NameSeed;
	uint64_t NullValue[2];
	uint16_t FactionID;
	int16_t Sector;
	uint16_t LocationID;
	uint8_t  BuildingType;
	uint8_t  Quality;
};

struct DoorData
{
	int32_t XPos;
	int32_t YPos;
	int32_t ZPos;
	uint16_t unknown;
	int16_t angle;
	int16_t unknown2;
	uint8_t  null;
};

//Dungeon
struct DT_Header
{
	int16_t unknown;
	int16_t unknown2;
	int32_t nGridWidth;
	int32_t nGridHeight;
	uint32_t uObjOffset;
	int32_t unknown3;
};

struct DT_Model
{
	char modelID[5];
	char objectType[3];
};

struct DT_ObjHeader
{
	int32_t unknown1;
	int32_t unknown2;
	int32_t unknown3;
	int32_t unknown4;
	int32_t FileSize;
	char unknown5[32];
	char TagDAGR[4];
	char unknown6[456];
};

enum ObjectType_e
{
	Model3D = 0x01,
	Light   = 0x02,
	Flat    = 0x03
};

struct DT_Object
{
	int32_t offsNext;
	int32_t offsPrev;
	int32_t xLoc;
	int32_t yLoc;
	int32_t zLoc;
	char objectType;
	int32_t postRecordOffset;
};

struct DT_PostObj3D
{
	int32_t xAngle;
	int32_t yAngle;
	int32_t zAngle;
	int16_t ModelIndex;	//index into m_apModels
	int32_t unknown1;
	char unknown2;
	int32_t ActionOffset;
};

struct DT_PostObjLight
{
	int32_t unknown1;
	int32_t unknown2;
	int16_t unknown3;
};

struct DT_PostObjFlat
{
	uint16_t Texture;
	int16_t character;
	int16_t factionID;
	char unknown[9];
};

struct DT_ActionRecord
{
	char DataEntry[5];
	int32_t TargetOffset;
	char Type;
};

#pragma pack(pop)

CellLoader_Daggerfall::CellLoader_Daggerfall() : CellLoader()
{
}

CellLoader_Daggerfall::~CellLoader_Daggerfall()
{
}

WorldCell *CellLoader_Daggerfall::LoadFromLocation( IDriver3D *pDriver, World *pWorld, void *pLocPtr )
{
	Location_Daggerfall *pLocation = (Location_Daggerfall *)pLocPtr;

	WorldCell *pCell = NULL;
	int32_t worldX = (int32_t)pLocation->m_x;
	int32_t worldY = (int32_t)pLocation->m_y;

	if ( pLocation )
	{
		pCell = xlNew WorldCell();
		LevelFuncMgr::SetWorldCell( pCell );
		pCell->SetWorldPos(worldX>>3, worldY>>3);
	}
	else
	{
		return NULL;
	}

	WorldMap::SetWorldCell(worldX>>3, worldY>>3, pCell);

	/***************************************************************
	 There are multiple parts to a World Cell. A Cell should have
	 at least one by may not have them all.
	 For example, most towns do not have a DUNGEON and most DUNGEONS
	 do not have INTERIORS. [Daggerfall/Sentinel/Wayrest are exceptions
	 to both of the above - they have all 3 parts].
	 ***************************************************************/

	int32_t worldX_Orig = worldX;
	int32_t worldY_Orig = worldY;

	int32_t nClimate = pWorld->GetTerrain()->GetClimate_MapScale(worldX>>3, worldY>>3);
	int32_t nRegion  = TERRAIN_TEMPERATE;
	if ( nClimate == Terrain::CLIMATE_DESERT )
		nRegion  = TERRAIN_DESERT;
	else if ( nClimate == Terrain::CLIMATE_MOUNTAIN )
		nRegion  = TERRAIN_MOUNTAIN;
	else if ( nClimate == Terrain::CLIMATE_SWAMP )
		nRegion  = TERRAIN_SWAMP;

	/***************************************************************
	 Load the EXTERIOR part of this cell.
	 ***************************************************************/
	if ( pLocation->m_BlockWidth > 0 || pLocation->m_BlockHeight > 0 )
	{
		Terrain *pTerrain = pWorld->GetTerrain();
		int32_t wx_map = worldX_Orig>>3;
		int32_t wy_map = worldY_Orig>>3;
		float fTileHeight = Math::Max( pTerrain->GetHeight_MapScale(wx_map, wy_map), 16.0f );
		int32_t worldY = worldY_Orig;
		if ( pLocation->m_pTexData == NULL )
		{
			 pLocation->m_pTexData = new uint8_t[256*pLocation->m_BlockWidth*pLocation->m_BlockHeight];
		}
		for (int y=0; y<pLocation->m_BlockHeight; y++, worldY++)
		{
			int32_t worldX = worldX_Orig;
			for (int x=0; x<pLocation->m_BlockWidth; x++, worldX++)
			{
				if ( ArchiveManager::GameFile_Open(ARCHIVETYPE_BSA, "BLOCKS.BSA", pLocation->m_pBlockNames[y*pLocation->m_BlockWidth+x].szName) )
				{
					ScratchPad::StartFrame();

					uint32_t uLength = ArchiveManager::GameFile_GetLength();
					char *pData = (char *)ScratchPad::AllocMem(uLength);
					ArchiveManager::GameFile_Read(pData, uLength);
					ArchiveManager::GameFile_Close();

					Sector *pSector = LoadBlock_Ext(pDriver, uLength, pData, nRegion, fTileHeight, worldX, worldY, &pLocation->m_pTexData[(y*pLocation->m_BlockWidth+x)*256]);
					pSector->m_uTypeFlags = SECTOR_TYPE_EXTERIOR;
					pCell->AddSector( pSector );

					ScratchPad::FreeFrame();
				}
			}
		}
	}

	/***************************************************************
	 Load the DUNGEON part of this cell.
	 ***************************************************************/
	if ( pLocation->m_dungeonBlockCnt > 0 )
	{
		int dungeonType = 1;

		Vector3 *pvStartLoc = NULL;
		int startBlock = pLocation->m_startDungeonBlock;
		if ( pLocation->m_dungeonBlockCnt == 1 )
		{
			startBlock = 0;
		}

		for (int b=0; b<pLocation->m_dungeonBlockCnt; b++)
		{
			if ( ArchiveManager::GameFile_Open(ARCHIVETYPE_BSA, "BLOCKS.BSA", pLocation->m_pDungeonBlocks[b].szName) )
			{
				ScratchPad::StartFrame();

				uint32_t uLength = ArchiveManager::GameFile_GetLength();
				char *pData = (char *)ScratchPad::AllocMem(uLength);
				ArchiveManager::GameFile_Read(pData, uLength);
				ArchiveManager::GameFile_Close();

				Vector3 vLoc(0.0f, 0.0f, 0.0f);
				vLoc.x = (f32)pLocation->m_pDungeonBlocks[b].x * 512.0f;
				vLoc.y = (f32)pLocation->m_pDungeonBlocks[b].y * 512.0f;

				int blockType = 1;
				if ( pLocation->m_pDungeonBlocks[b].szName[0] == 'B' || pLocation->m_pDungeonBlocks[b].szName[0] == 'S' )
				{
					blockType = 1;
				}
				else if ( pLocation->m_pDungeonBlocks[b].szName[0] == 'M' )
				{
					blockType = 2;
					size_t l = strlen( pLocation->m_pDungeonBlocks[b].szName );
					int idx = pLocation->m_pDungeonBlocks[b].szName[ l-5 ] - '0';
					blockType |= idx * 16;

					if ( idx == 1 )
						blockType++;
				}

				int index = 0;
				Vector3 vStartTag(0,0,0);
				Sector *pSector = LoadBlock(pDriver, uLength, index, pData, vLoc, vStartTag, (startBlock == b), worldX, worldY, blockType);
				pSector->m_uTypeFlags = SECTOR_TYPE_DUNGEON;
				pCell->AddSector( pSector );
				
				if ( pLocation->m_startDungeonBlock == b )
				{
					//set the dungeon start position for future reference (i.e. when entering the dungeon from outside).
					pCell->SetStartLoc(vStartTag);
				}

				ScratchPad::FreeFrame();
			}
		}
	}
	pLocation->m_bLoaded = true;

	return pCell;
}

WorldCell *CellLoader_Daggerfall::Load( IDriver3D *pDriver, World *pWorld, uint8_t *pData, uint32_t uLen, const string& sFile, int32_t worldX, int32_t worldY )
{
	Location_Daggerfall *pLocation = WorldMap::GetLocation(sFile.c_str());
	//Don't reload a location already in memory.
	if ( pLocation && pLocation->m_bLoaded )
		return NULL;

	return LoadFromLocation( pDriver, pWorld, pLocation );
}

DT_Header m_Header;
int m_nModelCnt;
DT_Model m_aModels[750];
Mesh *m_apModels[750];
MeshCollision *m_apModelsCol[750];
int m_anMeshID[750];

void _BuildWorldMtx(Object *pMesh, Vector3& vStartAngles, Vector3& vPos)
{
	Vector3 vAngles = vStartAngles * MATH_PI_OVER_2/512.0f;
	Matrix mWorld;
	mWorld.Identity();
	mWorld.EulerToMatrix(vAngles.x, vAngles.y, vAngles.z);
	
	mWorld.m[12] = vPos.x;
	mWorld.m[13] = vPos.y;
	mWorld.m[14] = vPos.z;

	pMesh->SetMatrix(mWorld);
	pMesh->ComputeTransformedBounds();
}

bool _IsDoor( int nModelID )
{
	if ( (nModelID >= 55000 && nModelID <= 55012) || nModelID == 55030 )
	{
		return true;
	}
	return false;
}

bool _IsEntrance( int nModelID )
{
	if ( nModelID == 70300 )
	{
		return true;
	}
	return false;
}

Sector *CellLoader_Daggerfall::LoadBlock_Ext(IDriver3D *pDriver, uint32_t uLength, char *pData, int32_t nClimate, float fTileHeight, int32_t worldX, int32_t worldY, uint8_t *pTexData)
{
	Sector_GeoBlock *pSector = xlNew Sector_GeoBlock();
	pSector->m_x = worldX;
	pSector->m_y = worldY;
	pSector->m_pValidNodes = xlNew uint8_t[16*16];

	int32_t index      = 0;
	int8_t  NumSubRec1 = pData[index]; index++;
	int8_t  NumSubRec2 = pData[index]; index++;
	int8_t  NumSubRec3 = pData[index]; index++;
	BlockPos *pBlockPos = (BlockPos *)&pData[index]; index += 640;
    //Building data.
	BuildingData *pBuildingData = (BuildingData *)&pData[index]; index += 832;
	//Section2 = 128 bytes.
	index += 128;
	int32_t *pBlockSizes = (int32_t *)&pData[index]; index += 128;
	//Small map data...
	char *pSmallMapHeader = &pData[index]; index +=   8;
	uint8_t *pTexInfo = (uint8_t *)&pData[index];	   index += 256;
	uint8_t *pObjInfo = (uint8_t *)&pData[index];	   index += 256;
	memcpy(pTexData, pTexInfo, 256);
	//Automap data (skip for now)...
	index += 4096;

	//FLD filenames.
	char *pszBlockFileName = &pData[index]; index += 13;
	char *pszFileNames[32];
	for (int32_t i=0; i<32; i++)
	{
		pszFileNames[i] = &pData[index]; index += 13;
	}

	float fFP_Scale = 1.0f / 4.0f;
	pSector->m_Bounds[0].Set( FLT_MAX, FLT_MAX, FLT_MAX );
	pSector->m_Bounds[1] = -pSector->m_Bounds[0];

	int32_t outside_index = index;
	for (int32_t j=0; j<NumSubRec1; j++)
	{
		index = outside_index;
		int32_t extIndex = (int32_t)pSector->m_Objects.size()-1;
		Object *pExtMesh=NULL;
		//exterior
		BlockHeader *pBlockHeader = (BlockHeader *)&pData[index]; index += 17;
		for (uint32_t i=0; i<pBlockHeader->Num3DObjRec; i++)
		{
			Block3DObj *pObj = (Block3DObj *)&pData[index]; index += 66;
			int32_t objID = pObj->ObjID1*100 + pObj->ObjID2;

			Object *pMeshObj = ObjectManager::CreateObject("exterior");
			pMeshObj->SetWorldPos(worldX, worldY);
			pMeshObj->SetScale( Vector3(1,1,1) );
			pSector->AddObject( pMeshObj->GetID() );
			
			Vector3 vPos;
			vPos.x = (float)( pObj->XPos2) * fFP_Scale;
			vPos.z = (float)(-pObj->YPos2) * fFP_Scale + fTileHeight + 0.1f;
			vPos.y = (float)(-pObj->ZPos2) * fFP_Scale;
			
			Vector3 vAngles( 0.0f, 0.0f, (float)(pObj->Angle + pBlockPos[j].Angle) );

			Matrix mBlockRot;
			float zAngle = -(float)pBlockPos[j].Angle * MATH_PI_OVER_2/512.0f;
			mBlockRot.Identity();
			Vector3 vAngel = Vector3(0, 0, 1);
			mBlockRot.AxisAngle( vAngel , zAngle );
			Vector3 vBlockOffs = Vector3((float)pBlockPos[j].XPos2 * fFP_Scale, (float)pBlockPos[j].YPos2 * fFP_Scale, 0.0f);
			Vector3 vOutPos = mBlockRot.TransformVector(vPos);
			vPos = vOutPos + vBlockOffs;
			vPos.x =  vPos.x - 512.0f;
			vPos.y = -vPos.y + 512.0f;
			pMeshObj->SetLoc( vPos );
			assert(vPos.x >= -512.0f && vPos.x <= 512.0f);
			assert(vPos.y >= -512.0f && vPos.y <= 512.0f);

			char szModel[6];
			char szModelName[32];
			szModel[0] = '0' + ((objID/10000)%10);
			szModel[1] = '0' + ((objID/1000)%10);
			szModel[2] = '0' + ((objID/100)%10);
			szModel[3] = '0' + ((objID/10)%10);
			szModel[4] = '0' + (objID%10);
			szModel[5] = 0;

			sprintf(szModelName, "%d_c%d", objID, nClimate);
			Mesh *pMesh = MeshCache::GetMesh( string(szModelName) );
			MeshCollision *pCol = MeshCache::GetMeshCollision( string(szModelName) );
			if ( pMesh->IsLoaded() == false )
			{
				m_MeshLoader.Load(pDriver, pMesh, pCol, szModel, nClimate, 0);
				pMesh->SetLoaded();
			}

			pMeshObj->SetRenderComponent( pMesh );
			pMeshObj->SetCollisionComponent( pCol );

			_BuildWorldMtx(pMeshObj, vAngles, vPos);
					
			Vector3 vMeshMin, vMeshMax;
			pMeshObj->GetWorldBounds(vMeshMin, vMeshMax);

			if ( vMeshMin.x < pSector->m_Bounds[0].x ) pSector->m_Bounds[0].x = vMeshMin.x;
			if ( vMeshMin.y < pSector->m_Bounds[0].y ) pSector->m_Bounds[0].y = vMeshMin.y;
			if ( vMeshMin.z < pSector->m_Bounds[0].z ) pSector->m_Bounds[0].z = vMeshMin.z;

			if ( vMeshMax.x > pSector->m_Bounds[1].x ) pSector->m_Bounds[1].x = vMeshMax.x;
			if ( vMeshMax.y > pSector->m_Bounds[1].y ) pSector->m_Bounds[1].y = vMeshMax.y;
			if ( vMeshMax.z > pSector->m_Bounds[1].z ) pSector->m_Bounds[1].z = vMeshMax.z;
		}
		for (uint32_t i=0; i<pBlockHeader->NumFlatObjRec; i++)
		{
			int32_t xPos = *((int32_t *)&pData[index]); index += 4;
			int32_t yPos = *((int32_t *)&pData[index]); index += 4;
			int32_t zPos = *((int32_t *)&pData[index]); index += 4;
			uint16_t uTex = *((uint16_t *)&pData[index]); index += 2;
			index += 2;	//unknown 1
			index++;	//unknown 2

			int32_t ImageIndex = uTex & 0x7f;
			int32_t FileIndex  = uTex / 0x80;

			if ( FileIndex != 199 )	//Tags, used for placing certain things.
			{
				char szTexName[128];
				MeshLoader_Daggerfall::BuildTextureName(szTexName, FileIndex);
				TextureHandle hTex = TextureCache::GameFile_LoadTexture_TexList( TEXTURETYPE_IMG, 7, ARCHIVETYPE_NONE, "", szTexName, ImageIndex );
				f32 fw, fh;
				int32_t ox, oy;
				uint32_t w, h;
				TextureCache::GetTextureSize(ox, oy, w, h, fw, fh);
				int16_t *pSpriteScale = (int16_t *)TextureCache::GetTexExtraData();

				assert( pSpriteScale[0] >= -256 && pSpriteScale[0] <= 256 );
				assert( pSpriteScale[1] >= -256 && pSpriteScale[1] <= 256 );

				//sprite scale...
				int newWidth  = w*(256+pSpriteScale[0])>>8;
				int newHeight = h*(256+pSpriteScale[1])>>8;

				Object *pObj = ObjectManager::CreateObject("Sprite_ZAxis");
				pObj->SetWorldPos(worldX, worldY);
				
				uint32_t uObjID = pObj->GetID();
				pSector->AddObject( uObjID );
				pObj->SetSector( pSector->m_uID );

				Vector3 vScale;
				vScale.x = (f32)newWidth  / 8.0f;
				vScale.y = vScale.x;
				vScale.z = (f32)newHeight / 8.0f;
				pObj->SetScale(vScale);
				
				Vector3 vLoc;
				vLoc.x =  (f32)xPos * fFP_Scale;
				vLoc.z = -(f32)yPos * fFP_Scale + fTileHeight;
				vLoc.y =  (f32)zPos * fFP_Scale;
				pObj->SetLoc(vLoc);

				Sprite_ZAxis *pSprite = xlNew Sprite_ZAxis();
				pSprite->SetTextureHandle( hTex );
				pObj->SetRenderComponent( pSprite );
				
				pObj->SetWorldBounds( vLoc - vScale, vLoc + vScale );
				pObj->SetBoundingSphere(vLoc, sqrtf(vScale.x*vScale.x + vScale.z*vScale.z));
			}
		}
		for (uint32_t i=0; i<pBlockHeader->NumSection3Rec; i++)
		{
			index += 16;
		}
		for (uint32_t i=0; i<pBlockHeader->NumPeopleRec; i++)
		{
			index += 17;
		}
		for (uint32_t i=0; i<pBlockHeader->NumDoorRec; i++)
		{
			index += 19;
		}
		
		//interior...
		pBlockHeader = (BlockHeader *)&pData[index]; index += 17;
		for (uint32_t i=0; i<pBlockHeader->Num3DObjRec; i++)
		{
			//assert( pExtMesh );

			Block3DObj *pObj = (Block3DObj *)&pData[index]; index += 66;
			if ( pExtMesh == NULL ) continue;
			int objID = pObj->ObjID1*100 + pObj->ObjID2;

			//finish the rest here....
		}

		for (uint32_t i=0; i<pBlockHeader->NumFlatObjRec; i++)
		{
			int32_t xPos = *((int32_t *)&pData[index]); index += 4;
			int32_t yPos = *((int32_t *)&pData[index]); index += 4;
			int32_t zPos = *((int32_t *)&pData[index]); index += 4;
			uint16_t uTex = *((uint16_t *)&pData[index]); index += 2;
			index += 2;	//unknown 1
			index++;	//unknown 2
			if ( pExtMesh == NULL ) continue;

			//finish the rest here...
		}
		for (uint32_t i=0; i<pBlockHeader->NumSection3Rec; i++)
		{
			index += 16;
		}
		for (uint32_t i=0; i<pBlockHeader->NumPeopleRec; i++)
		{
			int32_t xPos = *((int32_t *)&pData[index]); index += 4;
			int32_t yPos = *((int32_t *)&pData[index]); index += 4;
			int32_t zPos = *((int32_t *)&pData[index]); index += 4;
			uint16_t uTex = *((uint16_t *)&pData[index]); index += 2;
			uint16_t factionID = *((uint16_t *)&pData[index]); index += 2;
			index++;	//unknown

			if ( pExtMesh == NULL ) continue;

			//finish the rest here...
		}
		for (uint32_t i=0; i<pBlockHeader->NumDoorRec; i++)
		{
			DoorData *pDoor = (DoorData *)&pData[index];
			index += 19;

			if ( pExtMesh == NULL ) continue;

			//finish the rest here...
		}

		outside_index += pBlockSizes[j];
	}

	//additional 3D objects...
	index = outside_index;
	for (int32_t i=0; i<NumSubRec2; i++)
	{
		Block3DObj *pObj = (Block3DObj *)&pData[index]; index += 66;
		int32_t objID = pObj->ObjID1*100 + pObj->ObjID2;

		Object *pMeshObj = ObjectManager::CreateObject("exterior");
		pMeshObj->SetWorldPos(worldX, worldY);
		pMeshObj->SetScale( Vector3(1,1,1) );
		pSector->AddObject( pMeshObj->GetID() );

		Vector3 vPos;
		vPos.x =  (float)( pObj->XPos2) * fFP_Scale - 512.0f;
		vPos.z = -(float)pObj->YPos2 * fFP_Scale + fTileHeight + 0.1f;
		vPos.y =  (float)( pObj->ZPos2) * fFP_Scale + 512.0f;
		pMeshObj->SetLoc( vPos );
		assert(vPos.x >= -512.0f && vPos.x <= 512.0f);
		assert(vPos.y >= -512.0f && vPos.y <= 512.0f);

		Vector3 vAngles;
		vAngles.x = 0;
		vAngles.y = 0;
		vAngles.z = (float)(pObj->Angle);

		char szModel[32];
		char szModelName[32];
		sprintf(szModel, "%d", objID);
		sprintf(szModelName, "%d_c%d", objID, nClimate);
		Mesh *pMesh = MeshCache::GetMesh( string(szModelName) );
		MeshCollision *pCol = MeshCache::GetMeshCollision( string(szModelName) );
		if ( pMesh->IsLoaded() == false )
		{
			m_MeshLoader.Load(pDriver, pMesh, pCol, szModel, nClimate, 0);
			pMesh->SetLoaded();
		}

		pMeshObj->SetRenderComponent( pMesh );
		pMeshObj->SetCollisionComponent( pCol );

		_BuildWorldMtx(pMeshObj, vAngles, vPos);
				
		Vector3 vMeshMin, vMeshMax;
		pMeshObj->GetWorldBounds(vMeshMin, vMeshMax);

		if ( vMeshMin.x < pSector->m_Bounds[0].x ) pSector->m_Bounds[0].x = vMeshMin.x;
		if ( vMeshMin.y < pSector->m_Bounds[0].y ) pSector->m_Bounds[0].y = vMeshMin.y;
		if ( vMeshMin.z < pSector->m_Bounds[0].z ) pSector->m_Bounds[0].z = vMeshMin.z;

		if ( vMeshMax.x > pSector->m_Bounds[1].x ) pSector->m_Bounds[1].x = vMeshMax.x;
		if ( vMeshMax.y > pSector->m_Bounds[1].y ) pSector->m_Bounds[1].y = vMeshMax.y;
		if ( vMeshMax.z > pSector->m_Bounds[1].z ) pSector->m_Bounds[1].z = vMeshMax.z;
	}

	//Since this is an exterior, the whole terrain tile should be filled so that flats work.
	pSector->m_Bounds[0].x = Math::Min( -512.0f, pSector->m_Bounds[0].x );
	pSector->m_Bounds[0].y = Math::Min( -512.0f, pSector->m_Bounds[0].y );
	pSector->m_Bounds[1].x = Math::Max(  512.0f, pSector->m_Bounds[1].x );
	pSector->m_Bounds[1].y = Math::Max(  512.0f, pSector->m_Bounds[1].y );

	//Now go through and build valid nodes for NPC path finding.
	//This is done before flats are loaded to avoid confusion...
	float xPos, yPos;
	yPos = -512.0f + 32.0f;
	for (int32_t fy=0; fy<16; fy++)
	{
		xPos = -512.0f + 32.0f;
		for (int32_t fx=0; fx<16; fx++)
		{
			bool bValidNode = true;
			//is this point inside a mesh?
			vector<uint32_t>::iterator iObj = pSector->m_Objects.begin();
			vector<uint32_t>::iterator eObj = pSector->m_Objects.end();
			for (; iObj != eObj; ++iObj)
			{
				Object *pObj = ObjectManager::GetObjectFromID( *iObj );
				Vector3 vMin, vMax;
				pObj->GetWorldBounds(vMin, vMax);
				//is this node inside an object? If so, it is invalid...
				if ( xPos+0.5f >= vMin.x && xPos-0.5f <= vMax.x && yPos+0.5f >= vMin.y && yPos-0.5f <= vMax.y )
				{
					bValidNode = false;
					break;
				}
			}
			pSector->m_pValidNodes[ fy*16 + fx ] = bValidNode ? 1 : 0;

			xPos += 64;
		}
		yPos += 64;
	}

	//foliage and other stuff comes here.
	//foilage
	int32_t nFoilageIndex = 504;
	if ( nClimate == TERRAIN_DESERT )
		nFoilageIndex = 501;
	else if ( nClimate == TERRAIN_MOUNTAIN )
		nFoilageIndex = 510;
	else if ( nClimate == TERRAIN_SWAMP )
		nFoilageIndex = 502;

	yPos = -512.0f;
	for (int32_t fy=0; fy<16; fy++)
	{
		xPos = -512.0f;
		for (int32_t fx=0; fx<16; fx++)
		{
			uint8_t foilage = pObjInfo[fy*16+fx];
			if ( foilage < 0xff )
			{
				int32_t nIndex = (foilage>>2)&31;
				if (nIndex > 0) nIndex--;
				
				//skip the rest for now.
				char szTexName[128];
				MeshLoader_Daggerfall::BuildTextureName(szTexName, nFoilageIndex);
				TextureHandle hTex = TextureCache::GameFile_LoadTexture_TexList( TEXTURETYPE_IMG, 7, ARCHIVETYPE_NONE, "", szTexName, nIndex );
				f32 fw, fh;
				int32_t ox, oy;
				uint32_t w, h;
				TextureCache::GetTextureSize(ox, oy, w, h, fw, fh);
				int16_t *pSpriteScale = (int16_t *)TextureCache::GetTexExtraData();

				//sprite scale...
				int32_t newWidth  = w*(256+pSpriteScale[0])>>8;
				int32_t newHeight = h*(256+pSpriteScale[1])>>8;

				Object *pObj = ObjectManager::CreateObject("Sprite_ZAxis");
				pObj->SetWorldPos(worldX, worldY);
				
				uint32_t uObjID = pObj->GetID();
				pSector->AddObject( uObjID );
				pObj->SetSector( pSector->m_uID );
											
				Vector3 vScale;
				vScale.x = (f32)newWidth  / 8.0f;
				vScale.y = vScale.x;
				vScale.z = (f32)newHeight / 8.0f;
				pObj->SetScale(vScale);
				
				Vector3 vLoc;
				vLoc.x = xPos;
				vLoc.z = fTileHeight + vScale.z;
				vLoc.y = -yPos;
				pObj->SetLoc(vLoc);
				pObj->SetWorldPos(worldX, worldY);

				Sprite_ZAxis *pSprite = xlNew Sprite_ZAxis();
				pSprite->SetTextureHandle( hTex );
				pObj->SetRenderComponent( pSprite );
				
				pObj->SetWorldBounds( vLoc - vScale, vLoc + vScale );
				pObj->SetBoundingSphere(vLoc, sqrtf(vScale.x*vScale.x + vScale.y*vScale.y + vScale.z*vScale.z));
			}
			xPos += 64;
		}
		yPos += 64;
	}

	for (int32_t i=0; i<NumSubRec3; i++)
	{
		int32_t xPos = *((int32_t *)&pData[index]); index += 4;
		int32_t yPos = *((int32_t *)&pData[index]); index += 4;
		int32_t zPos = *((int32_t *)&pData[index]); index += 4;
		unsigned short uTex = *((uint16_t *)&pData[index]); index += 2;
		index += 2;	//unknown 1
		index++;	//unknown 2

		int ImageIndex = uTex & 0x7f;
		int FileIndex  = uTex / 0x80;

		//flat convert
		float fSpriteScale = 1.5f;
		bool bVisible = true;
		bool bHasGravity = false;
		bool bInteractive = false;
		int nMonsterID = -1;
		if ( FileIndex == 199 )	//Tags, used for placing certain things.
		{
		#if 0
			if ( ImageIndex == 8 )	//Enter tag.
			{
				bVisible = false;
			}
			else if ( ImageIndex == 10 ) //Start tag.
			{
				bVisible = false;
			}
			else if ( ImageIndex == 11 ) //Quest tag.
			{
				bVisible = false;
			}
			else if ( ImageIndex == 15 ) //Random monster.
			{
				FileIndex = 255;
				ImageIndex = 0;
				bHasGravity = true;

				nMonsterID = DynamicNPC::TYPE_BEAR;
			}
			else if ( ImageIndex == 16 ) //Specific monster.
			{
				FileIndex = 255;
				ImageIndex = 0;
				bHasGravity = true;

				nMonsterID = 0;//_ConvertFactionToMonsterID( sprite.factionID );
			}
			else if ( ImageIndex == 18 ) //Quest item.
			{
				bVisible = false;
			}
			else if ( ImageIndex == 19 ) //Random treasure.
			{
				FileIndex = 216;
				ImageIndex = rand()%48;
				while (ImageIndex == 4 || ImageIndex == 5 )
				{
					ImageIndex = rand()%48;
				};
				bHasGravity = true;
				bInteractive = true;
			}
			else if ( ImageIndex == 20 ) //Random flat.
			{
				bVisible = false;
			}
			else
			{
				bVisible = false;
			}
		#endif
			bVisible = false;
		}

		if ( bVisible )
		{
			char szTexName[128];
			MeshLoader_Daggerfall::BuildTextureName(szTexName, FileIndex);
			TextureHandle hTex = TextureCache::GameFile_LoadTexture_TexList( TEXTURETYPE_IMG, 7, ARCHIVETYPE_NONE, "", szTexName, ImageIndex );
			f32 fw, fh;
			int32_t ox, oy;
			uint32_t w, h;
			TextureCache::GetTextureSize(ox, oy, w, h, fw, fh);
			int16_t *pSpriteScale = (int16_t *)TextureCache::GetTexExtraData();

			//sprite scale...
			int newWidth  = w*(256+pSpriteScale[0])>>8;
			int newHeight = h*(256+pSpriteScale[1])>>8;

			Object *pObj = ObjectManager::CreateObject("Sprite_ZAxis");
			pObj->SetWorldPos(worldX, worldY);
			
			uint32_t uObjID = pObj->GetID();
			pSector->AddObject( uObjID );
			pObj->SetSector( pSector->m_uID );
										
			Vector3 vScale;
			vScale.x = (f32)newWidth  / 8.0f;
			vScale.y = vScale.x;
			vScale.z = (f32)newHeight / 8.0f;
			pObj->SetScale(vScale);
			
			Vector3 vLoc;
			vLoc.x =  (f32)xPos * fFP_Scale - 512.0f;
			vLoc.z = -(f32)yPos * fFP_Scale + fTileHeight + vScale.z;
			vLoc.y =  (f32)zPos * fFP_Scale + 512.0f;
			assert( vLoc.x >= -512.0f && vLoc.x <= 512.0f);
			assert( vLoc.y >= -512.0f && vLoc.y <= 512.0f);
			pObj->SetLoc(vLoc);
			pObj->SetWorldPos(worldX, worldY);

			Sprite_ZAxis *pSprite = xlNew Sprite_ZAxis();
			pSprite->SetTextureHandle( hTex );
			pObj->SetRenderComponent( pSprite );
			
			pObj->SetWorldBounds( vLoc - vScale, vLoc + vScale );
			pObj->SetBoundingSphere(vLoc, sqrtf(vScale.x*vScale.x + vScale.y*vScale.y + vScale.z*vScale.z));
		}
	}

	return pSector;
}

Sector *CellLoader_Daggerfall::LoadBlock(IDriver3D *pDriver, uint32_t uLength, int& index, char *pData, const Vector3& vBlockLoc, Vector3& vStartTagLoc, bool bStartBlock, int32_t worldX, int32_t worldY, int blockType)
{
	//for now... fix...
	int region = 300;
	int dungeonType = 1;

	ScratchPad::StartFrame();

	Sector_GeoBlock *pSector = xlNew Sector_GeoBlock();
	pSector->m_x = worldX;
	pSector->m_y = worldY;

	DT_Header m_Header = *((DT_Header *)&pData[index]); index += sizeof(DT_Header);
	dungeonType = blockType&15;
	m_nModelCnt = 0;
	int anIndex[750];
	for (int i=0; i<750; i++)
	{
		m_aModels[i] = *((DT_Model *)&pData[index]); index += sizeof(DT_Model);
		if ( m_aModels[i].modelID[0] > 0 )
		{
			anIndex[m_nModelCnt] = i;
			m_nModelCnt++;
		}
	}
	vector<LightObject *> pLights;
	unsigned int *pModelDataList = (unsigned int *)&pData[index];
	int mIndex = 0;
	unsigned int anModelData[750];
	for (int i=0; i<m_nModelCnt; i++)
	{
		if ( m_aModels[i].modelID[0] > 0 )
		{
			anModelData[mIndex++] = pModelDataList[i];
		}
	}
	index += 4*750;
	for (int i=0; i<m_nModelCnt; i++)
	{
		char pszModel[64];
		char szModelName[64];
		int nOverride = 0;
		pszModel[0] = m_aModels[ anIndex[i] ].modelID[0];
		pszModel[1] = m_aModels[ anIndex[i] ].modelID[1];
		pszModel[2] = m_aModels[ anIndex[i] ].modelID[2];
		pszModel[3] = m_aModels[ anIndex[i] ].modelID[3];
		pszModel[4] = m_aModels[ anIndex[i] ].modelID[4];
		pszModel[5] = 0;
		sprintf(szModelName, "%s_%d", pszModel, dungeonType);
		m_apModels[i] = MeshCache::GetMesh( string(szModelName) );
		m_apModelsCol[i] = MeshCache::GetMeshCollision( string(szModelName) );

		if ( dungeonType == 2 || dungeonType == 3 )
		{
			int index = blockType>>4;
			switch (index)
			{
				case 0:
						nOverride = 65536;
					break;
				case 1:
						if ( strnicmp(szModelName, "67011", 5) == 0 || strnicmp(szModelName, "67015", 5) == 0 || strnicmp(szModelName, "67019", 5) == 0 ||
							 strnicmp(szModelName, "67022", 5) == 0 || strnicmp(szModelName, "67025", 5) == 0 || strnicmp(szModelName, "55006", 5) == 0 )
							nOverride = 65536;
					break;
				case 2:
						nOverride = 65536;
					break;
				case 3:
						nOverride = 65536;
					break;
				case 4:
						nOverride = 65536;
					break;
				case 5:
						if ( strnicmp(szModelName, "61018", 5) == 0 || strnicmp(szModelName, "61020", 5) == 0 || strnicmp(szModelName, "72010", 5) == 0 )
							nOverride = 65536;
					break;
				case 6:
						nOverride = 65536;
					break;
				case 7:
						nOverride = 65536;
					break;
				case 8:
						nOverride = 65536;
					break;
			};
		}

		if ( m_apModels[i]->IsLoaded() == false )
		{
			m_MeshLoader.Load(pDriver, m_apModels[i], m_apModelsCol[i], m_aModels[ anIndex[i] ].modelID, region, dungeonType | nOverride);
			m_apModels[i]->SetLoaded();
		}

		m_anMeshID[ anIndex[i] ] = (pszModel[0]-'0')*10000 + (pszModel[1]-'0')*1000 + (pszModel[2]-'0')*100 + (pszModel[3]-'0')*10 + (pszModel[4]-'0');
	}

	DT_ObjHeader objHeader = *((DT_ObjHeader *)&pData[index]); index += sizeof(DT_ObjHeader);
	int  obj_roots_size = m_Header.nGridWidth*m_Header.nGridHeight;
	int *obj_root_offs  = (int *)ScratchPad::AllocMem(sizeof(int)*obj_roots_size);
	index = m_Header.uObjOffset;
	memcpy(obj_root_offs, &pData[index], obj_roots_size*4);
	index += obj_roots_size*4;

	f32 fFP_Scale = 1.0f / 4.0f;
	pSector->m_Bounds[0].Set( FLT_MAX, FLT_MAX, FLT_MAX );
	pSector->m_Bounds[1] = -pSector->m_Bounds[0];

	int objTarget[4096];
	int objRecordID[4096];
	int objCount = 0;

	bool bStartFound = false;
	for (int o=0; o<obj_roots_size; o++)
	{
		if ( obj_root_offs[o] > 0 )
		{
			int cur_obj = obj_root_offs[o];
			int next_obj;
			do
			{
				index = cur_obj;
				int objRecord = index;
				DT_Object object = *((DT_Object *)&pData[index]); index += sizeof(DT_Object);

				objTarget[objCount] = -1;
				objRecordID[objCount] = objRecord;
				
				if ( object.objectType == Model3D )
				{
					DT_PostObj3D obj3D = *((DT_PostObj3D *)&pData[object.postRecordOffset]); index += sizeof(DT_PostObj3D);
					if ( !bStartBlock && _IsEntrance( m_anMeshID[obj3D.ModelIndex] ) )
					{
						next_obj = object.offsNext;
						if ( next_obj > 0 )
						{
							cur_obj = next_obj;
						}

						continue;
					}

					char szName[64];
					strcpy(szName, "dungeonMesh");
					if ( _IsDoor( m_anMeshID[obj3D.ModelIndex] ) )
					{
						strcpy(szName, "door");
					}

					Object *pMeshObj = ObjectManager::CreateObject(szName);
					pMeshObj->SetWorldPos(worldX, worldY);
					pSector->AddObject( pMeshObj->GetID() );

					//pMeshObj->nLightCnt = 0;
					//pMeshObj->nFactionID = 0;

					pMeshObj->SetScale( Vector3(1,1,1) );

					Vector3 vPos;
					vPos.x =  (f32)object.xLoc * fFP_Scale + vBlockLoc.x;
					vPos.z = -(f32)object.yLoc * fFP_Scale + vBlockLoc.z;
					vPos.y =  (f32)object.zLoc * fFP_Scale + vBlockLoc.y;
					pMeshObj->SetLoc( vPos );

					Vector3 vAngles;
					vAngles.x = (f32)obj3D.zAngle;
					vAngles.y = (f32)obj3D.xAngle;
					vAngles.z = (f32)obj3D.yAngle;

					assert( obj3D.ModelIndex < m_nModelCnt && obj3D.ModelIndex >= 0 );
					pMeshObj->SetRenderComponent( m_apModels[obj3D.ModelIndex] );
					pMeshObj->SetCollisionComponent( m_apModelsCol[obj3D.ModelIndex] );

					_BuildWorldMtx(pMeshObj, vAngles, vPos);
					
					Vector3 vMeshMin, vMeshMax;
					pMeshObj->GetWorldBounds(vMeshMin, vMeshMax);					

					if ( vMeshMin.x < pSector->m_Bounds[0].x ) pSector->m_Bounds[0].x = vMeshMin.x;
					if ( vMeshMin.y < pSector->m_Bounds[0].y ) pSector->m_Bounds[0].y = vMeshMin.y;
					if ( vMeshMin.z < pSector->m_Bounds[0].z ) pSector->m_Bounds[0].z = vMeshMin.z;

					if ( vMeshMax.x > pSector->m_Bounds[1].x ) pSector->m_Bounds[1].x = vMeshMax.x;
					if ( vMeshMax.y > pSector->m_Bounds[1].y ) pSector->m_Bounds[1].y = vMeshMax.y;
					if ( vMeshMax.z > pSector->m_Bounds[1].z ) pSector->m_Bounds[1].z = vMeshMax.z;

					pMeshObj->SetGameID( m_anMeshID[obj3D.ModelIndex] );

					const f32 piOver2 = 1.5707963267948966192313216916398f;
					if ( objRecord == 22663 )
					{
						bStartFound = true;
						f32 angle = vAngles.z * piOver2/512.0f;

						//this is the dungeon entrace?
						pMeshObj->GetLoc(vStartTagLoc);
						vStartTagLoc.x = vStartTagLoc.x - 16.0f*cosf(angle);
						vStartTagLoc.y = vStartTagLoc.y - 16.0f*sinf(angle);
					}

					if ( _IsDoor( m_anMeshID[obj3D.ModelIndex] ) )
					{
						Logic *pLogic = LogicManager::GetLogic("LOGIC_DOOR");
						if ( pLogic )
						{
							pMeshObj->AddLogic( pLogic );
							//startAngle, endAngle, anim, open
							pMeshObj->AllocGameData(16);
							float *pGameData = (float *)pMeshObj->GetGameData();
							pGameData[0] =  vAngles.z * piOver2/512.0f;
							pGameData[1] = (vAngles.z + 450.0f) * piOver2/512.0f;
							pGameData[2] = 0.0f;
							pGameData[3] = 0;
						}
					}
					else if ( obj3D.ActionOffset > 0 )
					{
						DT_ActionRecord *pActionRec = (DT_ActionRecord *)&pData[ obj3D.ActionOffset ];

						Logic *pLogic = LogicManager::GetLogic("LOGIC_OBJ_ACTION");
						if ( pLogic )
						{
							pMeshObj->AddLogic( pLogic );

							pMeshObj->AllocGameData( sizeof(MeshAction) );
							MeshAction *pGameData = (MeshAction *)pMeshObj->GetGameData();

							pGameData->nParentID = -1;
							pGameData->nTargetID = -1;
							pGameData->nType	 = pActionRec->Type&0x0f;
							pGameData->nAxis	 = pActionRec->DataEntry[0];
							pGameData->Duration  = (float)( *((uint16_t *)&pActionRec->DataEntry[1]) ) / 16.0f;
							pGameData->Delta     = (float)( *((uint16_t *)&pActionRec->DataEntry[3]) );

							pGameData->origPos[0] = vPos.x;
							pGameData->origPos[1] = vPos.y;
							pGameData->origPos[2] = vPos.z;

							pGameData->origAng[0] = vAngles.x;
							pGameData->origAng[1] = vAngles.y;
							pGameData->origAng[2] = vAngles.z;

							pGameData->fDelta = 0.0f;		//which direction to move in and how much per step.
							pGameData->fAnim  = 0.0f;		//current animation state.
							pGameData->bAnim  = XL_FALSE;	//is currently animating?

							objTarget[objCount] = pActionRec->TargetOffset;
						}
					}

					if ( _IsEntrance( m_anMeshID[obj3D.ModelIndex] ) )
					{
						Vector3 vEntMin, vEntMax;
						pMeshObj->GetWorldBounds(vEntMin, vEntMax);
						Vector3 vEntrance( (vEntMin.x+vEntMax.x)*0.5f, (vEntMin.y+vEntMax.y)*0.5f, vEntMin.z );

						Vector3 vDE = vEntrance;
						Vector3 vDN(1, 0, 0), wDN;

						Matrix *pWorld = pMeshObj->GetMatrixPtr();
						wDN = pWorld->TransformNormal(vDN);

						Vector3 vLoc;
						pMeshObj->GetLoc(vLoc);
						vStartTagLoc.x = vLoc.x - 16.0f*wDN.x;
						vStartTagLoc.y = vLoc.y - 16.0f*wDN.y;
						vStartTagLoc.z = (vEntMin.z+vEntMax.z)*0.5f;
					}

					/*
					if ( obj3D.ActionOffset > 0 )
					{
						DT_ActionRecord *pActionRec = (DT_ActionRecord *)&pData[ obj3D.ActionOffset ];
						
						pMeshObj->action.nTargetOffs = pActionRec->TargetOffset;
						pMeshObj->action.pParent = NULL;
						pMeshObj->action.Type = pActionRec->Type&0x0f;
						pMeshObj->action.Axis = pActionRec->DataEntry[0];
						pMeshObj->action.Duration = (f32)( *((unsigned short *)&pActionRec->DataEntry[1]) ) / 16.0f;
						pMeshObj->action.Delta    = (f32)( *((unsigned short *)&pActionRec->DataEntry[3]) );
					}
					else
					{
						pMeshObj->action.Type = 0;
						pMeshObj->action.nTargetOffs = -1;
						pMeshObj->action.pParent = NULL;
						pMeshObj->action.Duration = 0;
					}
					*/

					/*
					if ( pMeshObj->pModel->m_bHasDungeonEntrance )
					{
						Vector3 vDE = pMeshObj->pModel->m_vDungeonEntrance;
						Vector3 vDN = pMeshObj->pModel->m_vDENrml, wDN;
						D3DXVec3TransformCoord( (D3DXVECTOR3 *)&m_vStartTagLoc, (D3DXVECTOR3 *)&vDE, &pMeshObj->mWorld );
						D3DXVec3TransformNormal( (D3DXVECTOR3 *)&wDN, (D3DXVECTOR3 *)&vDN, &pMeshObj->mWorld );

						m_vStartTagLoc.x = pMeshObj->vPos.x - 16.0f*wDN.x;
						m_vStartTagLoc.y = pMeshObj->vPos.y - 16.0f*wDN.y;
					}

					if ( pMeshObj->vWorldMin.x < m_vBoundsMin.x ) m_vBoundsMin.x = pMeshObj->vWorldMin.x;
					if ( pMeshObj->vWorldMin.y < m_vBoundsMin.y ) m_vBoundsMin.y = pMeshObj->vWorldMin.y;
					if ( pMeshObj->vWorldMin.z < m_vBoundsMin.z ) m_vBoundsMin.z = pMeshObj->vWorldMin.z;

					if ( pMeshObj->vWorldMax.x > m_vBoundsMax.x ) m_vBoundsMax.x = pMeshObj->vWorldMax.x;
					if ( pMeshObj->vWorldMax.y > m_vBoundsMax.y ) m_vBoundsMax.y = pMeshObj->vWorldMax.y;
					if ( pMeshObj->vWorldMax.z > m_vBoundsMax.z ) m_vBoundsMax.z = pMeshObj->vWorldMax.z;
					*/
					objCount++;
				}
				else if ( object.objectType == Flat )
				{
					DT_PostObjFlat sprite = *((DT_PostObjFlat *)&pData[object.postRecordOffset]); index += sizeof(DT_PostObjFlat);

					int ImageIndex = sprite.Texture & 0x7f;
					int FileIndex  = sprite.Texture / 0x80;
				#if 0 //temp.
					if ( FileIndex == 199 )	//Tags, used for placing certain things.
					{
						if ( ImageIndex == 10 ) //Start tag.
						{
						}
					}

				#else
					f32 fSpriteScale = 1.0f;

					bool bVisible = true;
					bool bHasGravity = false;
					bool bInteractive = false;
					int nMonsterID = -1;
					if ( FileIndex == 199 )	//Tags, used for placing certain things.
					{
						if ( ImageIndex == 8 )	//Enter tag.
						{
							bVisible = false;
						}
						else if ( ImageIndex == 10 ) //Start tag.
						{
							if ( bStartFound == false )
							{
								bStartFound = true;

								vStartTagLoc.x =  (f32)object.xLoc * fFP_Scale + vBlockLoc.x;
								vStartTagLoc.z = -(f32)object.yLoc * fFP_Scale + vBlockLoc.z;
								vStartTagLoc.y =  (f32)object.zLoc * fFP_Scale + vBlockLoc.y;
							}
							bVisible = false;
						}
						else if ( ImageIndex == 11 ) //Quest tag.
						{
							bVisible = false;
						}
						else if ( ImageIndex == 15 ) //Random monster.
						{
							FileIndex = 255;
							ImageIndex = 0;
							bHasGravity = true;

							//just pick from a random list for now...
							//const int randMonster[] = { DynamicNPC::TYPE_RAT, DynamicNPC::TYPE_IMP, DynamicNPC::TYPE_GIANT_BAT, 
							//	DynamicNPC::TYPE_BEAR, DynamicNPC::TYPE_ORC, DynamicNPC::TYPE_SKELETON };
							//nMonsterID = DynamicNPC::TYPE_BEAR;
							//nMonsterID = randMonster[ rand()%6 ];
						}
						else if ( ImageIndex == 16 ) //Specific monster.
						{
							FileIndex = 255;
							ImageIndex = 0;
							bHasGravity = true;

							//for testing.
							bVisible = false;
							//nMonsterID = _ConvertFactionToMonsterID( sprite.factionID );
						}
						else if ( ImageIndex == 18 ) //Quest item.
						{
							bVisible = false;
						}
						else if ( ImageIndex == 19 ) //Random treasure.
						{
							FileIndex = 216;
							ImageIndex = rand()%48;
							while (ImageIndex == 4 || ImageIndex == 5 )
							{
								ImageIndex = rand()%48;
							};
							bHasGravity = true;
							bInteractive = true;
						}
						else if ( ImageIndex == 20 ) //Random flat.
						{
							bVisible = false;
						}
						else
						{
							bVisible = false;
						}
					}

					if ( bVisible )
					{
						bool bEmissive = false;
						uint32_t w, h;
						int32_t ox, oy;
						if ( nMonsterID < 0 )
						{
							char szTexName[128];
							MeshLoader_Daggerfall::BuildTextureName(szTexName, FileIndex);
							TextureHandle hTex = TextureCache::GameFile_LoadTexture_TexList( TEXTURETYPE_IMG, 7, ARCHIVETYPE_NONE, "", szTexName, ImageIndex );
							f32 fw, fh;
							TextureCache::GetTextureSize(ox, oy, w, h, fw, fh);
							int16_t *pSpriteScale = (int16_t *)TextureCache::GetTexExtraData();

							assert( pSpriteScale[0] >= -256 && pSpriteScale[0] <= 256 );
							assert( pSpriteScale[1] >= -256 && pSpriteScale[1] <= 256 );

							//sprite scale...
							int newWidth  = w*(256+pSpriteScale[0])>>8;
							int newHeight = h*(256+pSpriteScale[1])>>8;

							Object *pObj = ObjectManager::CreateObject("Sprite_ZAxis");
							pObj->SetWorldPos(worldX, worldY);
							
							uint32_t uObjID = pObj->GetID();
							pSector->AddObject( uObjID );
							objCount++;
							pObj->SetSector( pSector->m_uID );
														
							Vector3 vScale;
							vScale.x = (f32)newWidth  / 8.0f;
							vScale.y = vScale.x;
							vScale.z = (f32)newHeight / 8.0f;
							pObj->SetScale(vScale);
							
							Vector3 vLoc;
							vLoc.x =  (f32)object.xLoc * fFP_Scale + vBlockLoc.x;
							vLoc.z = -(f32)object.yLoc * fFP_Scale + vBlockLoc.z;
							vLoc.y =  (f32)object.zLoc * fFP_Scale + vBlockLoc.y;
							pObj->SetLoc(vLoc);

							Sprite_ZAxis *pSprite = xlNew Sprite_ZAxis();
							pSprite->SetTextureHandle( hTex );
							pObj->SetRenderComponent( pSprite );
							
							pObj->SetWorldBounds( vLoc - vScale, vLoc + vScale );
							pObj->SetBoundingSphere(vLoc, sqrtf(vScale.x*vScale.x + vScale.z*vScale.z));

							//texture index: 190 or 210 = lighting (emissive)
							//DungeonLight_t *pLight = NULL;
							static int8_t _bLit190[] = { 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0 };
							static int8_t _bLit200[] = { 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
							static int8_t _bLit202[] = { 0, 0, 1, 0, 0, 0, 0 };
							static int8_t _bLit210[] = { 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
													 1, 1, 1, 1};
							if ( (FileIndex == 190 && _bLit190[ImageIndex]) || (FileIndex == 200 && _bLit200[ImageIndex]) || 
								 (FileIndex == 202 && _bLit202[ImageIndex]) || (FileIndex == 210 && _bLit210[ImageIndex]) )
							{
								//test...
								if (FileIndex == 210 && ImageIndex <= 6)
								{
									vLoc.z += 3.0f;
								}

								pSprite->SetFlag( Sprite_ZAxis::FLAG_EMISSIVE );
								pLights.push_back( xlNew LightObject(vLoc) );
							}

							/*DungeonSprite_t *pSprite = new DungeonSprite_t;
							pSprite->pSprite = new Sprite();
							pSprite->vPos.x =  (f32)object.xLoc * fFP_Scale + m_vPos.x;
							pSprite->vPos.z = -(f32)object.yLoc * fFP_Scale + m_vPos.z;
							pSprite->vPos.y =  (f32)object.zLoc * fFP_Scale + m_vPos.y;
							pSprite->bInterior = true;
							pSprite->bVisible = true;

							//temp
							pSprite->pSprite->character = sprite.character;
							pSprite->pSprite->factionID = sprite.factionID;
							for (int d=0; d<9; d++)
							{
								pSprite->pSprite->data[d] = sprite.unknown[d];
							}
							pSprite->pSprite->fileIndex = FileIndex;
							pSprite->bInteractive = bInteractive;
							if ( pSprite->bInteractive )
							{
								AssignTreasure( pSprite->pSprite );
							}
							//

							//texture index: 190 or 210 = lighting (emissive)
							DungeonLight_t *pLight = NULL;
							if ( FileIndex == 190 || FileIndex == 210 )
							{
								bEmissive = true;
								//allocate a dungeon light for this too...
								pLight = new DungeonLight_t;
								pLight->vPos = pSprite->vPos;
								pLight->bInterior = true;
								m_Lights.push_back( pLight );
							}

							DHANDLE hTex = TextureCache::LoadTextureDF(ImageIndex, FileIndex, w, h, ox, oy, 0, bEmissive, PAL_PAL, true);

							pSprite->pSprite->SetTexture(hTex, w, h, ox, oy, fSpriteScale);
							pSprite->pSprite->SetGravity( bHasGravity );

							if ( bEmissive )
							{
								pSprite->pSprite->SetEmissive(1.0f);
							}

							m_Sprites.push_back( pSprite );*/
						}
						/*else
						{
							DynamicNPC *pMonster = new DynamicNPC( pWorld );
														
							Vector3 vPos;
							vPos.x =  (f32)object.xLoc * fFP_Scale + m_vPos.x;
							vPos.z = -(f32)object.yLoc * fFP_Scale + m_vPos.z;
							vPos.y =  (f32)object.zLoc * fFP_Scale + m_vPos.y;
							pMonster->SetPos(vPos);

							pMonster->SetType( (unsigned int)nMonsterID, -1, bEnemiesActive );
							pMonster->SetNPC(true);
							pWorld->AddObjToSector(pMonster, 0);
						}*/
					}
				#endif
				}
				else
				{
					assert( object.objectType == Light || object.objectType == Flat );
				}

				next_obj = object.offsNext;
				if ( next_obj > 0 )
				{
					cur_obj = next_obj;
				}
			} while (next_obj > 0);
		}
	}

	if ( obj_root_offs )
	{
		ScratchPad::FreeFrame();
		obj_root_offs = NULL;
	}

	//now resolve action records.
	vector<uint32_t>::iterator iObj = pSector->m_Objects.begin();
	vector<uint32_t>::iterator eObj = pSector->m_Objects.end();
	int idx=0;
	for (; iObj != eObj; ++iObj)
	{
		Object *pObj = ObjectManager::GetObjectFromID( *iObj );
		assert(pObj);

		//resolve parent/child relationship.
		if ( objTarget[idx] > -1 )
		{
			MeshAction *pGameDataParent = (MeshAction *)pObj->GetGameData();

			int idxTarget = 0;
			vector<uint32_t>::iterator iObjTarget = pSector->m_Objects.begin();
			for (; iObjTarget != eObj; ++iObjTarget)
			{
				if ( objRecordID[idxTarget] == objTarget[idx] )
				{
					Object *pObjTarget = ObjectManager::GetObjectFromID( *iObjTarget );
					MeshAction *pGameDataTarget = (MeshAction *)pObjTarget->GetGameData();

					if ( pGameDataTarget )
					{
						pGameDataTarget->nParentID = *iObj;
						pGameDataParent->nTargetID = *iObjTarget;
					}

					break;
				}
				idxTarget++;
			}
		}
		idx++;
	}

	//Initialize the objects.
	iObj = pSector->m_Objects.begin();
	for (; iObj != eObj; ++iObj)
	{
		Object *pObj = ObjectManager::GetObjectFromID( *iObj );
		assert(pObj);

		//Finish the object initialization. Setup logics and such.
		pObj->Init();
	}

	const f32 fLightRadius = 64.0f;
	const f32 fTestRadius = (fLightRadius*0.95f)*(fLightRadius*0.95f);
	//combine lights.
	int lightCnt = (int)pLights.size();
	int8_t *lightAdded = xlNew int8_t[lightCnt];
	memset(lightAdded, 0, lightCnt);
	Vector3 aveLoc[16];
	for (int l=0; l<lightCnt; l++)
	{
		if ( lightAdded[l] )
			continue;

		Vector3 cenAve = pLights[l]->m_vLoc;
		aveLoc[0] = cenAve;
		int aveCnt = 1;
		lightAdded[l] = 2;
		//does this intersect any other lights?
		for (int ll=l+1; ll<lightCnt; ll++)
		{
			for (int a=0; a<aveCnt; a++)
			{
				Vector3 offs = aveLoc[a] - pLights[ll]->m_vLoc;
				if ( offs.Dot(offs) < fTestRadius && fabsf(offs.z) < 1.0f )
				{
					cenAve = cenAve + pLights[ll]->m_vLoc;
					aveLoc[aveCnt] = pLights[ll]->m_vLoc;
					aveCnt++;
					lightAdded[ll] = 1;

					break;
				}
			}
		}
		if ( aveCnt > 1 )
		{
			cenAve = cenAve * (1.0f/(f32)aveCnt);
			pLights[l]->m_vLoc = cenAve;
		}
	}
	
	//now add them to the sector...
	for (int l=0; l<lightCnt; l++)
	{
		if ( lightAdded[l] == 2 )
		{
			pSector->AddLight( pLights[l] );
		}
		else
		{
			xlDelete pLights[l];
		}
	}

	xlDelete [] lightAdded;

	//assign lights to objects.
	const vector<LightObject *>& lightList = pSector->GetLightList();
	iObj = pSector->m_Objects.begin();
	for (; iObj != eObj; ++iObj)
	{
		Object *pObj = ObjectManager::GetObjectFromID( *iObj );
		assert(pObj);

		Vector3 vCen;
		f32 fRadius = pObj->GetBoundingSphere(vCen);

		for (int l=0; l<(int)lightList.size(); l++)
		{
			const LightObject *pLight = lightList[l];

			Vector3 vOffset = vCen - pLight->m_vLoc;
			if ( vOffset.Length() <= fLightRadius + fRadius )
			{
				pObj->AddLight( pLight );
			}
		}
	}

#if 0
	//now assign geometry to each light.
	vector<DungeonMeshObj_t *>::iterator iMesh = m_MeshObjs.begin();
	vector<DungeonMeshObj_t *>::iterator iMEnd = m_MeshObjs.end();
	for (; iMesh != iMEnd; ++iMesh)
	{
		DungeonMeshObj_t *pMesh = *iMesh;
		
		vector<DungeonLight_t *>::iterator iLight = m_Lights.begin();
		vector<DungeonLight_t *>::iterator iLEnd  = m_Lights.end();
		Vector3 vOffs;
		
		for( ; iLight != iLEnd; ++iLight)
		{
			DungeonLight_t *pLight = *iLight;

			vOffs = pLight->vPos - pMesh->vCen;

			if ( vOffs.Length() < pMesh->fRadius + _fLightRadius && pMesh->nLightCnt < 32 )
			{
				pMesh->apLights[ pMesh->nLightCnt++ ] = pLight;
			}
		}

		//now resolve action records.
		if ( pMesh->action.nTargetOffs > 0 )
		{
			//search for the target...
			bool bMatchFound = false;
			vector<DungeonMeshObj_t *>::iterator iMesh2 = m_MeshObjs.begin();
			vector<DungeonMeshObj_t *>::iterator iMEnd2 = m_MeshObjs.end();
			for (; iMesh2 != iMEnd2; ++iMesh2)
			{
				DungeonMeshObj_t *pMesh2 = *iMesh2;
				if ( pMesh2->nObjRecord == pMesh->action.nTargetOffs )
				{
					bMatchFound = true;
					pMesh->action.pTarget  = pMesh2;
					pMesh2->action.pParent = pMesh;
					break;
				}
			}
			if ( bMatchFound == false )
			{
				pMesh->action.pTarget = NULL;
				pMesh->action.pParent = NULL;
			}
		}
		else
		{
			pMesh->action.pTarget = NULL;
		}
	}
#endif

	return pSector;
}
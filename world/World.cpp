#include "World.h"
#include "WorldCell.h"
#include "Object.h"
#include "Sector.h"
#include "Terrain.h"
#include "../ui/XL_Console.h"
#include "../world/ObjectManager.h"
#include "../math/Math.h"
#include "../fileformats/Location_Daggerfall.h"
#include "../fileformats/CellManager.h"
#include <string.h>
#include <stdio.h>

World::World()
{
	m_Player   = NULL;
	m_pCamera  = NULL;
	m_pTerrain = NULL;

	XL_Console::RegisterCmd("r_lockcam", (void*)CC_LockCamera, Console::CTYPE_FUNCTION, "Lock the camera so that the visibility stops updating.", this);
	XL_Console::RegisterCmd("r_maxRecursion", &Sector::s_MaxSecDrawCnt, Console::CTYPE_UINT, "Maximum portal recursion, 0 = default.", NULL);

	m_uSectorTypeVis = SECTOR_TYPE_EXTERIOR;
	//m_uSectorTypeVis = SECTOR_TYPE_DUNGEON;
}

World::~World()
{
	UnloadWorldCells();
}

void World::UnloadWorldCells()
{
	vector<WorldCell *>::iterator iCell = m_WorldCells.begin();
	vector<WorldCell *>::iterator eCell = m_WorldCells.end();
	for (; iCell != eCell; ++iCell)
	{
		xlDelete *iCell;
	}
	m_WorldCells.clear();
}

bool World::AddWorldCell(WorldCell *pCell)
{
	if ( pCell )
	{
		m_WorldCells.push_back( pCell );
		return true;
	}
	return false;
}

void World::RemoveWorldCell(WorldCell *pCell)
{
	vector<WorldCell *>::iterator iCell = m_WorldCells.begin();
	vector<WorldCell *>::iterator eCell = m_WorldCells.end();
	for (; iCell != eCell; ++iCell)
	{
		if ( *iCell == pCell )
		{
			xlDelete pCell;
			m_WorldCells.erase( iCell );
			break;
		}
	}
}

void World::Collide(Vector3 *p0, Vector3 *p1, uint32_t& uSector, f32 fRadius, bool bPassThruAdjoins)
{
	//only collide with cells that are close enough.
	if ( m_WorldCells.size() > 0 )
	{
		int32_t camX = m_pCamera->GetWorldPosX()>>3;
		int32_t camY = m_pCamera->GetWorldPosY()>>3;
		int32_t cx, cy;

		vector<WorldCell *>::iterator iCell = m_WorldCells.begin();
		vector<WorldCell *>::iterator eCell = m_WorldCells.end();
		for (; iCell != eCell; ++iCell)
		{
			WorldCell *pCell = *iCell;
			pCell->GetWorldPos(cx, cy);

			//only try to collide if in the same world cell or neighboring cell.
			if ( Math::abs(cx-camX) > 1 || Math::abs(cy-camY) > 1 )
				continue;
			
			pCell->Collide(p0, p1, uSector, fRadius, m_uSectorTypeVis, bPassThruAdjoins, m_pCamera->GetWorldPosX(), m_pCamera->GetWorldPosY());
		}
	}

	//for now only support downward rays for terrain, fix later.
	if ( (m_uSectorTypeVis&SECTOR_TYPE_EXTERIOR) && m_pTerrain && m_pTerrain->IsActive() && p0->x-p1->x == 0.0f && p0->y-p1->y == 0.0f )
	{
		float z = m_pTerrain->GetHeight(p0->x, p0->y);
		float z0 = p0->z - 8.9f;
		float z1 = p1->z - 8.9f;
		if ( z <= z0 && z > z1 )
		{
			float u = (z - z0) / (z1 - z0);
			if ( u >= 0.0f && u <= 1.0f )
			{
				*p1 = *p0 + (*p1-*p0)*u;
			}
		}
		if ( p1->z < z )
		{
			p1->z = z;
		}
	}
}

void World::RayCastAndActivate(Vector3 *p0, Vector3 *p1, uint32_t& uSector)
{
	//only collide with cells that are close enough.
	if ( m_WorldCells.size() > 0 )
	{
		int32_t camX = m_pCamera->GetWorldPosX()>>3;
		int32_t camY = m_pCamera->GetWorldPosY()>>3;
		int32_t cx, cy;

		vector<WorldCell *>::iterator iCell = m_WorldCells.begin();
		vector<WorldCell *>::iterator eCell = m_WorldCells.end();
		for (; iCell != eCell; ++iCell)
		{
			WorldCell *pCell = *iCell;
			pCell->GetWorldPos(cx, cy);

			//only try to collide if in the same world cell or neighboring cell.
			if ( Math::abs(cx-camX) > 1 || Math::abs(cy-camY) > 1 )
				continue;
			
			pCell->RayCastAndActivate(p0, p1, uSector, m_uSectorTypeVis, m_pCamera->GetWorldPosX(), m_pCamera->GetWorldPosY());
		}
	}
}

bool World::Raycast(Vector3 *p0, Vector3 *p1, Vector3 *pInter)
{
	bool bRayHit = false;
	//for now just worry about cell 0.
	if ( m_WorldCells.size() > 0 )
	{
		//only collide with cells that are close enough.
		if ( m_WorldCells.size() > 0 )
		{
			int32_t camX = m_pCamera->GetWorldPosX()>>3;
			int32_t camY = m_pCamera->GetWorldPosY()>>3;
			int32_t cx, cy;

			vector<WorldCell *>::iterator iCell = m_WorldCells.begin();
			vector<WorldCell *>::iterator eCell = m_WorldCells.end();
			for (; iCell != eCell; ++iCell)
			{
				WorldCell *pCell = *iCell;
				pCell->GetWorldPos(cx, cy);

				//only try to collide if in the same world cell or neighboring cell.
				if ( Math::abs(cx-camX) > 1 || Math::abs(cy-camY) > 1 )
					continue;
				
				bRayHit = pCell->Raycast(p0, p1, pInter, m_uSectorTypeVis, m_pCamera->GetWorldPosX(), m_pCamera->GetWorldPosY());
			}
		}


		//for now only support downward rays for terrain, fix later.
		if ( (m_uSectorTypeVis&SECTOR_TYPE_EXTERIOR) && m_pTerrain && m_pTerrain->IsActive() && p0->x-p1->x == 0.0f && p0->y-p1->y == 0.0f )
		{
			float z = m_pTerrain->GetHeight(p0->x, p0->y);
			if ( z <= p0->z && (z > pInter->z || bRayHit == false) )
			{
				float u = (z - p0->z) / (p1->z - p0->z);
				if ( u >= 0.0f && u <= 1.0f )
				{
					bRayHit = true;
					*pInter = *p0 + (*p1-*p0)*u;
				}
			}
		}
	}
	return bRayHit;
}

WorldCell *World::GetCameraWorldCell()
{
	WorldCell *pCell = NULL;
	if ( m_pCamera )
	{
		pCell = WorldMap::GetWorldCell( m_pCamera->GetWorldPosX()>>3, m_pCamera->GetWorldPosY()>>3 );
	}
	return pCell;
}

bool World::Update(float dt, IDriver3D *pDriver)
{
	Object *player = ObjectManager::FindObject("PLAYER");
	if ( !player || !m_pCamera )
		return false;

	vector<WorldCell *>::iterator iCell = m_WorldCells.begin();
	vector<WorldCell *>::iterator eCell = m_WorldCells.end();
	for (; iCell != eCell; ++iCell)
	{
		//add culling, won't affect games that only have a single cell..
		(*iCell)->Update( m_pCamera, dt, m_uSectorTypeVis );
	}
	if ( (m_uSectorTypeVis&SECTOR_TYPE_EXTERIOR) && m_pTerrain )
	{
		//Update dynamic object positions and the camera.
		//if we didn't teleport, then see if the player has moved too far...
		Vector3 vPlayerLoc;
		player->GetLoc(vPlayerLoc);
		int32_t dx = 0, dy = 0;
		const Vector3& vCamLoc = m_pCamera->GetLoc();
		Vector3 vNewCamLoc = vCamLoc;
		Vector3 vMove(0,0,0);

		while ( vPlayerLoc.x+vMove.x > 512.0f )
		{
			dx++;
			vMove.x -= 1024.0f;
			vNewCamLoc.x -= 1024.0f;
		};
		while ( vPlayerLoc.x+vMove.x < -512.0f )
		{
			dx--;
			vMove.x += 1024.0f;
			vNewCamLoc.x += 1024.0f;
		};

		while ( vPlayerLoc.y+vMove.y > 512.0f )
		{
			dy++;
			vMove.y -= 1024.0f;
			vNewCamLoc.y -= 1024.0f;
		};
		while ( vPlayerLoc.y+vMove.y < -512.0f )
		{
			dy--;
			vMove.y += 1024.0f;
			vNewCamLoc.y += 1024.0f;
		};

		if ( dx || dy )
		{
			vPlayerLoc = vPlayerLoc + vMove;
			player->SetLoc(vPlayerLoc);
			
			int32_t tx = player->GetWorldX();
			int32_t ty = player->GetWorldY();
			player->SetWorldPos(tx+dx, ty+dy);

			m_pCamera->SetLoc(vNewCamLoc);
		}
	}

	//Load and unload locations.
	bool bTerrainUpdated = false;
	if ( m_pTerrain->IsActive() )
	{
		int32_t nRectCnt = 0;
		LocationRect aLocRect[16];

		int32_t x0 = player->GetWorldX()-7, y0 = player->GetWorldY()-7;
		int32_t x1 = x0+15, y1 = y0+15;

		int32_t wx0 = (x0>>3)-1, wy0 = (y0>>3)-1;
		int32_t wx1 = (x1>>3)+1, wy1 = (y1>>3)+1;

		for (int32_t wy=wy0; wy<=wy1; wy++)
		{
			for (int32_t wx=wx0; wx<=wx1; wx++)
			{
				Location_Daggerfall *pLoc = WorldMap::GetLocation(wx, wy);
				if ( pLoc && !pLoc->m_bLoaded )
				{
					WorldCell *pWorldCell = CellManager::LoadFromLocation(pDriver, this, CELLTYPE_DAGGERFALL, pLoc);
					if ( pWorldCell )
					{
						AddWorldCell( pWorldCell );
					}
				}

				if ( pLoc && (pLoc->m_BlockWidth && pLoc->m_BlockHeight) )
				{
					aLocRect[nRectCnt].vRectInner.Set( pLoc->m_x, pLoc->m_y, pLoc->m_x+pLoc->m_BlockWidth, pLoc->m_y+pLoc->m_BlockHeight );
					aLocRect[nRectCnt].vRectOuter.x =  aLocRect[nRectCnt].vRectInner.x - 4;
					aLocRect[nRectCnt].vRectOuter.y =  aLocRect[nRectCnt].vRectInner.y - 4;
					aLocRect[nRectCnt].vRectOuter.z =  aLocRect[nRectCnt].vRectInner.z + 4;
					aLocRect[nRectCnt].vRectOuter.w =  aLocRect[nRectCnt].vRectInner.w + 4;
					aLocRect[nRectCnt].pLoc	   = pLoc;
					aLocRect[nRectCnt].fHeight = Math::Max( m_pTerrain->GetHeight_MapScale(wx, wy), 16.0f );
					nRectCnt++;
				}
			}
		}
		bTerrainUpdated = m_pTerrain->Update(player->GetWorldX(), player->GetWorldY(), nRectCnt, aLocRect);
	}

	m_pCamera->SetWorldPos(player->GetWorldX(), player->GetWorldY());

	//Collect valid pathing nodes.
	if ( bTerrainUpdated )
	{
		ClearPathingNodes(player->GetWorldX(), player->GetWorldY());

		WorldCell *pCell = GetCameraWorldCell();
		if ( pCell )
		{
			//collect any sectors in range, +/-1 tile.
			int32_t tx = player->GetWorldX(), ty = player->GetWorldY();
			for (uint32_t s=0; s<pCell->GetSectorCount(); s++)
			{
				Sector *pSector = pCell->GetSector(s);
				int32_t worldX = pSector->m_x, worldY = pSector->m_y;

				if ( Math::abs(worldX-tx) <= 1 && Math::abs(worldY-ty) <= 1 )
				{
					AddPathingNodes( worldX-tx, worldY-ty, pSector->m_pValidNodes );
				}
			}
		}
	}

	return bTerrainUpdated;
}

struct PathNode
{
	Vector2 localWorldPos;
	Vector2 localPos;
	int32_t worldX, worldY;

	bool bOpen;
};

PathNode m_PathNodes[2304];
bool m_bNodesAvail = false;

void World::ClearPathingNodes(int32_t wx, int32_t wy)
{
	float curY = -1536.0f + 32.0f;	//+32: start in the middle of the cell.
	for (uint32_t y=0; y<48; y++)
	{
		float curX = -1536.0f + 32.0f;	//+32: start in the middle of the cell.
		uint32_t uIndex = y*48;
		for (uint32_t x=0; x<48; x++, uIndex++)
		{
			m_PathNodes[uIndex].localWorldPos.x = curX;
			m_PathNodes[uIndex].localWorldPos.y = curY;

			m_PathNodes[uIndex].localPos.x = curX;
			if ( m_PathNodes[uIndex].localPos.x < -512.0f )
			{
				m_PathNodes[uIndex].localPos.x += 1024.0f;
			}
			else if ( m_PathNodes[uIndex].localPos.x >= 512.0f )
			{
				m_PathNodes[uIndex].localPos.x -= 1024.0f;
			}
			m_PathNodes[uIndex].localPos.y = curY;
			if ( m_PathNodes[uIndex].localPos.y < -512.0f )
			{
				m_PathNodes[uIndex].localPos.y += 1024.0f;
			}
			else if ( m_PathNodes[uIndex].localPos.y >= 512.0f )
			{
				m_PathNodes[uIndex].localPos.y -= 1024.0f;
			}

			m_PathNodes[uIndex].worldX = wx-1 + (x>>4);
			m_PathNodes[uIndex].worldY = wy-1 + (y>>4);

			m_PathNodes[uIndex].bOpen = false;

			curX += 64.0f;
		}
		curY += 64.0f;
	}

	m_bNodesAvail = false;
}

void World::AddPathingNodes(int32_t dwX, int32_t dwY, uint8_t *pValidNodes)
{
	if ( !pValidNodes )
		return;

	uint32_t x0 = (uint32_t)(dwX+1)<<4;
	uint32_t y0 = (uint32_t)(dwY+1)<<4;
	for (uint32_t y=y0; y<y0+16; y++)
	{
		for (uint32_t x=x0; x<x0+16; x++)
		{
			m_PathNodes[y*48+x].bOpen = pValidNodes[(y-y0)*16+x-x0]!=0;
			if ( m_PathNodes[y*48+x].bOpen )
			{
				m_bNodesAvail = true;
			}
		}
	}
}

void World::GetNodeVector(Vector2& vOffset, const Vector3& curPos, int32_t wx, int32_t wy, int32_t nodeX, int32_t nodeY)
{
	Vector3 vPos = curPos;
	vPos.x += (wx - m_pCamera->GetWorldPosX())*1024.0f;
	vPos.y += (wy - m_pCamera->GetWorldPosY())*1024.0f;

	vOffset.Set( m_PathNodes[nodeY*48+nodeX].localWorldPos.x - vPos.x, m_PathNodes[nodeY*48+nodeX].localWorldPos.y - vPos.y );
}

bool World::CheckNode(int32_t nodeX, int32_t nodeY)
{
	return m_PathNodes[nodeY*48+nodeX].bOpen;
}

bool World::GetRandomNode(int32_t& nodeX, int32_t& nodeY, Vector3& outPos, int32_t& outWorldX, int32_t& outWorldY)
{
	if ( !m_bNodesAvail )
	{
		return false;
	}
	
	bool bFound = false;
	int32_t nIter = 0;
	while (!bFound && nIter < 32)
	{
		nodeX = (rand()%46) + 1;
		nodeY = (rand()%46) + 1;

		if ( m_PathNodes[nodeY*48+nodeX].bOpen )
		{
			//make sure there is at least 1 open neighbor...
			if ( m_PathNodes[(nodeY-1)*48+nodeX].bOpen || m_PathNodes[(nodeY-1)*48+nodeX-1].bOpen || m_PathNodes[(nodeY-1)*48+nodeX+1].bOpen ||
				 m_PathNodes[(nodeY+1)*48+nodeX].bOpen || m_PathNodes[(nodeY+1)*48+nodeX-1].bOpen || m_PathNodes[(nodeY+1)*48+nodeX+1].bOpen ||
				 m_PathNodes[nodeY*48+nodeX-1].bOpen || m_PathNodes[nodeY*48+nodeX+1].bOpen )
			{
				outPos.x = m_PathNodes[nodeY*48+nodeX].localPos.x;
				outPos.y = m_PathNodes[nodeY*48+nodeX].localPos.y;
				outPos.z = m_pTerrain->GetHeight(m_PathNodes[nodeY*48+nodeX].localWorldPos.x, m_PathNodes[nodeY*48+nodeX].localWorldPos.y);

				outWorldX = m_PathNodes[nodeY*48+nodeX].worldX;
				outWorldY = m_PathNodes[nodeY*48+nodeX].worldY;

				bFound = true;
				break;
			}
		}
		nIter++;
	};

	return bFound;
}

void World::Render(IDriver3D *pDriver)
{
	if ( (m_uSectorTypeVis&SECTOR_TYPE_EXTERIOR) && m_pTerrain )
	{
		m_pTerrain->Render( m_pCamera );
	}

	vector<WorldCell *>::iterator iCell = m_WorldCells.begin();
	vector<WorldCell *>::iterator eCell = m_WorldCells.end();
	for (; iCell != eCell; ++iCell)
	{
		//add culling, won't affect games that only have a single cell..
		(*iCell)->Render( pDriver, m_pCamera, m_uSectorTypeVis );
	}
}

void World::CC_LockCamera(const vector<string>& args, void *pUserData)
{
	World *pThis = (World *)pUserData;

	bool bLock = false;	//if there are no arguments then just disable (the "default")
	if ( args.size() > 1 )
	{
		const char *arg = args[1].c_str();
		if ( arg[0] == '1' )
			bLock = true;
	}

	Camera::EnableCameraUpdating( !bLock );
	vector<WorldCell *>::iterator iCell = pThis->m_WorldCells.begin();
	vector<WorldCell *>::iterator eCell = pThis->m_WorldCells.end();
	for (; iCell != eCell; ++iCell)
	{
		//add culling, won't affect games that only have a single cell..
		(*iCell)->LockCamera( pThis->m_pCamera, bLock );
	}
}

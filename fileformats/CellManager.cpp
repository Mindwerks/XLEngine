#include "CellManager.h"
#include "ArchiveManager.h"
#include "CellLoader_BloodMap.h"
#include "CellLoader_OutlawsMap.h"
#include "CellLoader_Daggerfall.h"
#include "../world/LevelFuncMgr.h"
#include "../memory/ScratchPad.h"
#include "../ui/XL_Console.h"

CellLoader *CellManager::m_CellLoaders[CELLTYPE_COUNT];

void CellManager::Init()
{
	//create the cell loader list here.
	m_CellLoaders[CELLTYPE_BLOOD_MAP]   = xlNew CellLoader_BloodMap();
	m_CellLoaders[CELLTYPE_OUTLAWS_MAP] = xlNew CellLoader_OutlawsMap();
	m_CellLoaders[CELLTYPE_DAGGERFALL]  = xlNew CellLoader_Daggerfall();
}

void CellManager::Destroy()
{
	for (uint32_t c=0; c<CELLTYPE_COUNT; c++)
	{
		if ( m_CellLoaders[c] )
		{
			xlDelete m_CellLoaders[c];
		}
		m_CellLoaders[c] = NULL;
	}
}

WorldCell *CellManager::LoadFromLocation(IDriver3D *pDriver, World *pWorld, uint32_t uCellType, void *pLocPtr)
{
	WorldCell *pCell = NULL;

	if ( uCellType >= CELLTYPE_COUNT )	//unsupported type.
		return NULL;

	CellLoader *pLoader = m_CellLoaders[uCellType];

	//Currently only this option is supported for Loading directly from locations.
	if ( pLoader->UsesOwnFiles() )
	{
		pCell = pLoader->LoadFromLocation(pDriver, pWorld, pLocPtr);
	}

	return pCell;
}

WorldCell *CellManager::LoadCell(IDriver3D *pDriver, World *pWorld, uint32_t uCellType, Archive *pCellArchive, const string& sFile, int32_t worldX, int32_t worldY)
{
	WorldCell *pCell = NULL;

	if ( uCellType >= CELLTYPE_COUNT )	//unsupported type.
		return NULL;

	CellLoader *pLoader = m_CellLoaders[uCellType];

	if ( pLoader->UsesOwnFiles() )
	{
		pCell = pLoader->Load( pDriver, pWorld, NULL, 0, sFile, worldX, worldY );
	}
	else if ( ArchiveManager::GameFile_Open(pCellArchive, sFile.c_str()) )
	{
		ScratchPad::StartFrame();
		{
			uint32_t uLen  = ArchiveManager::GameFile_GetLength();
			uint8_t *pData = (uint8_t *)ScratchPad::AllocMem( uLen+1 );

			if ( pData )
			{
				LevelFuncMgr::Destroy();

				ArchiveManager::GameFile_Read(pData, uLen);
				ArchiveManager::GameFile_Close();

				pCell = pLoader->Load( pDriver, pWorld, pData, uLen, sFile, worldX, worldY );
			}
		}
		ScratchPad::FreeAllFrames();
	}

	return pCell;
}

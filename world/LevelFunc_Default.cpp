#include "LevelFuncMgr.h"
#include "LevelFunc_Default.h"
#include "WorldCell.h"
#include "Sector_2_5D.h"
#include "Object.h"
#include <cmath>

//SlidingDoor
void LFunc_SlidingDoor_SetValue(LevelFunc *pFunc, int32_t nSector, float value, bool bInstant)
{
    WorldCell *pCell = pFunc->GetWorldCell();
    const Vector3& vDir = pFunc->GetDirection();
    Vector2 dir2D(vDir.x, vDir.y);

    Sector_2_5D *pSector = (Sector_2_5D *)pCell->GetSector( nSector );
    for (uint32_t w=0; w<pSector->m_uWallCount; w++)
    {
        Wall *pWall = &pSector->m_Walls[w];
        uint16_t i0 = pWall->m_idx[0];
        uint16_t i1 = pWall->m_idx[1];
        if ( pWall->m_flags&Wall::WALL_FLAGS_MORPH )
        {
            pSector->m_pVertexCur[i0] = pSector->m_pVertexBase[i0] + dir2D*value;
            pSector->m_pVertexCur[i1] = pSector->m_pVertexBase[i1] + dir2D*value;

            //we must also move the attached "mirror" walls as well...
            if ( pWall->m_adjoin[0] != 0xffff )
            {
                Sector_2_5D *pMirrorSector = (Sector_2_5D *)pCell->GetSector( pWall->m_adjoin[0] );
                Wall *pMirror = &pMirrorSector->m_Walls[ pWall->m_mirror[0] ];

                i0 = pMirror->m_idx[0];
                i1 = pMirror->m_idx[1];

                pMirrorSector->m_pVertexCur[i0] = pMirrorSector->m_pVertexBase[i0] + dir2D*value;
                pMirrorSector->m_pVertexCur[i1] = pMirrorSector->m_pVertexBase[i1] + dir2D*value;
            }

            //we must also move the attached "mirror" walls as well...
            if ( pWall->m_adjoin[1] != 0xffff )
            {
                Sector_2_5D *pMirrorSector = (Sector_2_5D *)pCell->GetSector( pWall->m_adjoin[1] );
                Wall *pMirror = &pMirrorSector->m_Walls[ pWall->m_mirror[1] ];

                i0 = pMirror->m_idx[0];
                i1 = pMirror->m_idx[1];

                pMirrorSector->m_pVertexCur[i0] = pMirrorSector->m_pVertexBase[i0] + dir2D*value;
                pMirrorSector->m_pVertexCur[i1] = pMirrorSector->m_pVertexBase[i1] + dir2D*value;
            }
        }
        else if ( pWall->m_flags&Wall::WALL_FLAGS_INV_MORPH )
        {
            pSector->m_pVertexCur[i0] = pSector->m_pVertexBase[i0] - dir2D*value;
            pSector->m_pVertexCur[i1] = pSector->m_pVertexBase[i1] - dir2D*value;

            //we must also move the attached "mirror" walls as well...
            if ( pWall->m_adjoin[0] != 0xffff )
            {
                Sector_2_5D *pMirrorSector = (Sector_2_5D *)pCell->GetSector( pWall->m_adjoin[0] );
                Wall *pMirror = &pMirrorSector->m_Walls[ pWall->m_mirror[0] ];

                i0 = pMirror->m_idx[0];
                i1 = pMirror->m_idx[1];

                pMirrorSector->m_pVertexCur[i0] = pMirrorSector->m_pVertexBase[i0] - dir2D*value;
                pMirrorSector->m_pVertexCur[i1] = pMirrorSector->m_pVertexBase[i1] - dir2D*value;
            }

            //we must also move the attached "mirror" walls as well...
            if ( pWall->m_adjoin[1] != 0xffff )
            {
                Sector_2_5D *pMirrorSector = (Sector_2_5D *)pCell->GetSector( pWall->m_adjoin[1] );
                Wall *pMirror = &pMirrorSector->m_Walls[ pWall->m_mirror[1] ];

                i0 = pMirror->m_idx[0];
                i1 = pMirror->m_idx[1];

                pMirrorSector->m_pVertexCur[i0] = pMirrorSector->m_pVertexBase[i0] - dir2D*value;
                pMirrorSector->m_pVertexCur[i1] = pMirrorSector->m_pVertexBase[i1] - dir2D*value;
            }
        }
    }

    //handle sliding objects.
    uint32_t uClientObjCnt = pFunc->GetClientObjCount();
    for (uint32_t i=0; i<uClientObjCnt; i++)
    {
        LevelFunc::ClientObject *pObject = pFunc->GetClientObj(i);
        
        Vector3 vNewLoc = pObject->vInitialPos;
        if ( pObject->uFlags&Wall::WALL_FLAGS_INV_MORPH )
        {
            vNewLoc.x -= dir2D.x*value;
            vNewLoc.y -= dir2D.y*value;
        }
        else
        {
            vNewLoc.x += dir2D.x*value;
            vNewLoc.y += dir2D.y*value;
        }

        pObject->pObj->SetLoc( vNewLoc );
    }
}

//Slide - like the above, but everything goes.
void LFunc_Slide_SetValue(LevelFunc *pFunc, int32_t nSector, float value, bool bInstant)
{
    WorldCell *pCell = pFunc->GetWorldCell();
    const Vector3& vDir = pFunc->GetDirection();
    Vector2 dir2D(vDir.x, vDir.y);

    Sector_2_5D *pSector = (Sector_2_5D *)pCell->GetSector( nSector );
    for (uint32_t w=0; w<pSector->m_uWallCount; w++)
    {
        Wall *pWall = &pSector->m_Walls[w];
        uint16_t i0 = pWall->m_idx[0];
        uint16_t i1 = pWall->m_idx[1];

        pSector->m_pVertexCur[i0] = pSector->m_pVertexBase[i0] + dir2D*value;
        pSector->m_pVertexCur[i1] = pSector->m_pVertexBase[i1] + dir2D*value;

        //we must also move the attached "mirror" walls as well...
        if ( pWall->m_adjoin[0] != 0xffff )
        {
            Sector_2_5D *pMirrorSector = (Sector_2_5D *)pCell->GetSector( pWall->m_adjoin[0] );
            Wall *pMirror = &pMirrorSector->m_Walls[ pWall->m_mirror[0] ];

            i0 = pMirror->m_idx[0];
            i1 = pMirror->m_idx[1];

            pMirrorSector->m_pVertexCur[i0] = pMirrorSector->m_pVertexBase[i0] + dir2D*value;
            pMirrorSector->m_pVertexCur[i1] = pMirrorSector->m_pVertexBase[i1] + dir2D*value;
        }

        //we must also move the attached "mirror" walls as well...
        if ( pWall->m_adjoin[1] != 0xffff )
        {
            Sector_2_5D *pMirrorSector = (Sector_2_5D *)pCell->GetSector( pWall->m_adjoin[1] );
            Wall *pMirror = &pMirrorSector->m_Walls[ pWall->m_mirror[1] ];

            i0 = pMirror->m_idx[0];
            i1 = pMirror->m_idx[1];

            pMirrorSector->m_pVertexCur[i0] = pMirrorSector->m_pVertexBase[i0] + dir2D*value;
            pMirrorSector->m_pVertexCur[i1] = pMirrorSector->m_pVertexBase[i1] + dir2D*value;
        }
    }

    //handle sliding objects.
    uint32_t uClientObjCnt = pFunc->GetClientObjCount();
    for (uint32_t i=0; i<uClientObjCnt; i++)
    {
        LevelFunc::ClientObject *pObject = pFunc->GetClientObj(i);
        
        Vector3 vNewLoc = pObject->vInitialPos;
        vNewLoc.x += dir2D.x*value;
        vNewLoc.y += dir2D.y*value;

        pObject->pObj->SetLoc( vNewLoc );
    }
}

//Elevator_MoveFloor
void LFunc_Elevator_MoveFloor_SetValue(LevelFunc *pFunc, int32_t nSector, float value, bool bInstant)
{
    WorldCell *pCell = pFunc->GetWorldCell();
    Sector_2_5D *pSector = (Sector_2_5D *)pCell->GetSector( nSector );
    pSector->m_ZRangeCur.x = value;
}

//Elevator_MoveCeil
void LFunc_Elevator_MoveCeil_SetValue(LevelFunc *pFunc, int32_t nSector, float value, bool bInstant)
{
    WorldCell *pCell = pFunc->GetWorldCell();
    Sector_2_5D *pSector = (Sector_2_5D *)pCell->GetSector( nSector );
    pSector->m_ZRangeCur.y = value;
}

//Rotate
void LFunc_Rotate_SetValue(LevelFunc *pFunc, int32_t nSector, float value, bool bInstant)
{
    WorldCell *pCell = pFunc->GetWorldCell();
    Sector_2_5D *pSector = (Sector_2_5D *)pCell->GetSector( nSector );
    
    const Vector3 pivot3 = pFunc->GetPivot();
    Vector2 pivot( pivot3.x, pivot3.y );

    float cA = cosf(value);
    float sA = sinf(value);
    //go ahead and rotate the verts...
    for (uint32_t w=0; w<pSector->m_uWallCount; w++)
    {
        Wall *pWall = &pSector->m_Walls[w];
        uint16_t i0 = pWall->m_idx[0];
        uint16_t i1 = pWall->m_idx[1];
        
        Vector2 pC = pSector->m_pVertexBase[i0] - pivot;
        pSector->m_pVertexCur[i0].x = pC.x*cA + pC.y*sA + pivot.x;
        pSector->m_pVertexCur[i0].y = pC.y*cA - pC.x*sA + pivot.y;

        pC = pSector->m_pVertexBase[i1] - pivot;
        pSector->m_pVertexCur[i1].x = pC.x*cA + pC.y*sA + pivot.x;
        pSector->m_pVertexCur[i1].y = pC.y*cA - pC.x*sA + pivot.y;

        //we must also move the attached "mirror" walls as well...
        if ( pWall->m_adjoin[0] != 0xffff )
        {
            Sector_2_5D *pMirrorSector = (Sector_2_5D *)pCell->GetSector( pWall->m_adjoin[0] );
            Wall *pMirror = &pMirrorSector->m_Walls[ pWall->m_mirror[0] ];

            i0 = pMirror->m_idx[0];
            i1 = pMirror->m_idx[1];

            pC = pMirrorSector->m_pVertexBase[i0] - pivot;
            pMirrorSector->m_pVertexCur[i0].x = pC.x*cA + pC.y*sA + pivot.x;
            pMirrorSector->m_pVertexCur[i0].y = pC.y*cA - pC.x*sA + pivot.y;

            pC = pMirrorSector->m_pVertexBase[i1] - pivot;
            pMirrorSector->m_pVertexCur[i1].x = pC.x*cA + pC.y*sA + pivot.x;
            pMirrorSector->m_pVertexCur[i1].y = pC.y*cA - pC.x*sA + pivot.y;
        }

        //we must also move the attached "mirror" walls as well...
        if ( pWall->m_adjoin[1] != 0xffff )
        {
            Sector_2_5D *pMirrorSector = (Sector_2_5D *)pCell->GetSector( pWall->m_adjoin[1] );
            Wall *pMirror = &pMirrorSector->m_Walls[ pWall->m_mirror[1] ];

            i0 = pMirror->m_idx[0];
            i1 = pMirror->m_idx[1];

            pC = pMirrorSector->m_pVertexBase[i0] - pivot;
            pMirrorSector->m_pVertexCur[i0].x = pC.x*cA + pC.y*sA + pivot.x;
            pMirrorSector->m_pVertexCur[i0].y = pC.y*cA - pC.x*sA + pivot.y;

            pC = pMirrorSector->m_pVertexBase[i1] - pivot;
            pMirrorSector->m_pVertexCur[i1].x = pC.x*cA + pC.y*sA + pivot.x;
            pMirrorSector->m_pVertexCur[i1].y = pC.y*cA - pC.x*sA + pivot.y;
        }
    }
}

//LightFX
void LFunc_LightFX_SetValue(LevelFunc *pFunc, int32_t nSector, float value, bool bInstant)
{
    WorldCell *pCell = pFunc->GetWorldCell();
    Sector_2_5D *pSector = (Sector_2_5D *)pCell->GetSector( nSector );

    const int32_t flickerTable[] =
    { 
        1, 3, 6, 
        1, 3, 6, 
        6, 6, 6,
        1, 3, 6,
        1, 3, 6,
        6, 6, 6,
        6, 6, 6,
        1, 3,
        1, 3
    };

    int32_t index = (int32_t)( (value*25.0f) );
    index %= 25;
    
    pSector->m_aLightFX[0] = -flickerTable[index]*4 + 24;
    pSector->m_aLightFX[1] = -flickerTable[index]*4 + 24;
    pSector->m_aLightFX[2] = -flickerTable[index]*4 + 24;
}

//MotionFX
void LFunc_MotionFX_SetValue(LevelFunc *pFunc, int32_t nSector, float value, bool bInstant)
{
    WorldCell *pCell = pFunc->GetWorldCell();
    Sector_2_5D *pSector = (Sector_2_5D *)pCell->GetSector( nSector );

    const Vector3 scrollDir = pFunc->GetDirection();
    Vector3 scrollOffset = scrollDir * value;

    pSector->m_texOffset[0].Set( scrollOffset.x, scrollOffset.y );
}

//TriggerToggle
void LFunc_TriggerToggle_Activate(LevelFunc *pFunc, int32_t mask, int32_t items, bool bForce)
{
    uint32_t uClientCount = pFunc->GetClientCount();
    for (uint32_t c=0; c<uClientCount; c++)
    {
        LevelFunc *pClient = pFunc->GetClient(c);
        pClient->SendMessage( LevelFunc::IMSG_ACTIVATE, 0, 0 );
    }
}

//Global setup.
void SetupDefaultLevelFuncs()
{
    LevelFuncMgr::AddLevelFuncCB("SlidingDoor", nullptr, LFunc_SlidingDoor_SetValue);
    LevelFuncMgr::AddLevelFuncCB("Slide", nullptr, LFunc_Slide_SetValue);
    LevelFuncMgr::AddLevelFuncCB("Elevator_MoveFloor", nullptr, LFunc_Elevator_MoveFloor_SetValue);
    LevelFuncMgr::AddLevelFuncCB("Elevator_MoveCeil", nullptr, LFunc_Elevator_MoveCeil_SetValue);
    LevelFuncMgr::AddLevelFuncCB("Rotate", nullptr, LFunc_Rotate_SetValue);
    LevelFuncMgr::AddLevelFuncCB("LightFX", nullptr, LFunc_LightFX_SetValue);
    LevelFuncMgr::AddLevelFuncCB("MotionFX", nullptr, LFunc_MotionFX_SetValue);
    LevelFuncMgr::AddLevelFuncCB("TriggerToggle", LFunc_TriggerToggle_Activate, nullptr);
}
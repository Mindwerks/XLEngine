#include "CellLoader_BloodMap.h"
#include "../world/WorldCell.h"
#include "../world/Sector_2_5D.h"
#include "../world/ObjectManager.h"
#include "../world/Object.h"
#include "../world/OrientedSprite.h"
#include "../world/Sprite_ZAxis.h"
#include "../world/LevelFuncMgr.h"
#include "../render/TextureCache.h"
#include "../fileformats/TextureTypes.h"
#include "../fileformats/ArchiveTypes.h"
#include "../math/Math.h"
#include "../ui/XL_Console.h"
#include "../EngineSettings.h"

#include <string>
#include <cstdio>
#include <cassert>

#define MAX_TILES 9216

const float m_fWorldToTexel_X = 8.0f;
const float m_fWorldToTexel_Z = 8.0f;

CellLoader_BloodMap::CellLoader_BloodMap() : CellLoader()
{
    m_pBloodSectors = NULL;
    m_pBloodWalls = NULL;
    m_pBloodSprites = NULL;
}

CellLoader_BloodMap::~CellLoader_BloodMap()
{
}

void CellLoader_BloodMap::DecryptBuffer(uint8_t *pBuffer, const uint32_t uDataSize, uint8_t uDecryptKey)
{
    // Variables
    assert(pBuffer != NULL);

    // If the map isn't encrypted
    if (!m_bIsEncrypted)
        return;

    // Decryption
    for (uint16_t i = 0; i < uDataSize; i++)
    {
        pBuffer[i] ^= (uint8_t)(uDecryptKey + i);
    }
}

bool CellLoader_BloodMap::ParseHeader(char *pData, int32_t& index)
{
    // Variables
    HeaderPart1 header1;
    HeaderPart3 header3;

    char *pMapSignature;
    MapVersion *pMapVersion;

    pMapSignature = &pData[index]; index += 4;
    pMapVersion = (MapVersion *)&pData[index]; index += sizeof(MapVersion);

    if (pMapSignature[0] != 'B' || pMapSignature[1] != 'L' || pMapSignature[2] != 'M' || pMapSignature[3] != 0x1A )
    {
        XL_Console::PrintF("^1Error: The file's signature does not match the blood map signature.");
        return false;
    }

    XL_Console::PrintF("^8Info: Map version %u.%u", pMapVersion->Major, pMapVersion->Minor);

    if (pMapVersion->Major == 6 && pMapVersion->Minor == 3)
    {
        m_bIsEncrypted = false;
    }
    else if (pMapVersion->Major == 7 && pMapVersion->Minor == 0)
    {
        m_bIsEncrypted = true;
    }
    else
    {
        XL_Console::PrintF("^1Error: The XL Engine can only handle Blood maps version 6.3 and 7.0.");
        XL_Console::PrintF("^1Try bringing the map up in the latest version of mapedit, then save it.");
        XL_Console::PrintF("^1This should produce a 7.0 version map, which the XL Engine CAN load.");
        return false;
    }

    // Load and decrypt the first part of the header
    memcpy(&header1, &pData[index], sizeof(HeaderPart1)); index += sizeof(HeaderPart1);
    DecryptBuffer((uint8_t*)&header1, sizeof(HeaderPart1), 0x4D);

    // Store the info
    m_bloodData.pos[0]   = header1.startX;
    m_bloodData.pos[1]   = header1.startY;
    m_bloodData.pos[2]   = header1.startZ;
    m_bloodData.angle    = header1.startAngle;
    m_bloodData.startSec = header1.sectorNum;

    // Skip the part 2 of the header, since we don't know what it is. Investigate later.
    index += 11;

    // Load and decrypt the third part of the header
    memcpy(&header3, &pData[index], sizeof(HeaderPart3)); index += sizeof(HeaderPart3);
    DecryptBuffer((uint8_t*)&header3, sizeof(HeaderPart3), 0x68);

    // Store the map info.
    m_bloodData.revisions = header3.mapRevisions;
    m_bloodData.secCnt    = header3.numSectors;
    m_bloodData.wallCnt   = header3.numWalls;
    m_bloodData.spriteCnt = header3.numSprites;

    return true;
}

int32_t CellLoader_BloodMap::FindFirstSector(char *pData, int32_t& index)
{
    // Variables
    uint16_t uBuffer;

    if (!m_bIsEncrypted)
        return 75;

    index = 171;
    uBuffer = *((uint16_t *)&pData[index]);

    switch (uBuffer)
    {
        case 0x2120:
            return 203;

        case 0x0302:
            return 173;

        case 0x1110:   // E2M7
            return 187; // Try every 40 against Matt S. key if it doesn't work

        default:
            XL_Console::PrintF("^1Error: Unknown map format: %u", uBuffer);
            return -3;
    }
    return -1;
}

bool CellLoader_BloodMap::ExtractSectors(char *pData, int32_t& index)
{
    // Variables
    uint8_t Buffer[128];
    uint8_t uDecryptKey;

    uDecryptKey = ((m_bloodData.revisions * sizeof(BloodSector)) & 0xFF);
    m_pBloodSectors = xlNew BloodSector_Xtra[m_bloodData.secCnt];

    for (int32_t nInd = 0; nInd < m_bloodData.secCnt; nInd++)
    {
        memcpy(Buffer, &pData[index], sizeof(BloodSector));
        DecryptBuffer(Buffer, sizeof(BloodSector), uDecryptKey);

        memset( &m_pBloodSectors[nInd], 0, sizeof(BloodSector_Xtra) );
        memcpy( &m_pBloodSectors[nInd], Buffer, sizeof(BloodSector) );
        index += sizeof(BloodSector);

        // If extra data follow
        if (m_pBloodSectors[nInd].extra > 0) // offset to extra array
        {
            if ( nInd == 133 )
            {
                static int _x=0;
                _x++;
            }
            uint8_t *pExtData = (uint8_t *)&pData[index];

            m_pBloodSectors[nInd].startState = (pExtData[1]&64) ? 1 : 0;
            m_pBloodSectors[nInd].cmd  = (pExtData[9]>>2) + (pExtData[10]&63)*64;
            m_pBloodSectors[nInd].rxID = pExtData[8];
            m_pBloodSectors[nInd].txID = pExtData[6];

            m_pBloodSectors[nInd].triggerFlags = 0;
            if ( pExtData[22]&16 )
                m_pBloodSectors[nInd].triggerFlags |= TRIGGER_DECOUPLED;
            if ( pExtData[22]&32 )
                m_pBloodSectors[nInd].triggerFlags |= TRIGGER_ONE_SHOT;
            if ( pExtData[53]&1 )
                m_pBloodSectors[nInd].triggerFlags |= TRIGGER_LOCKED;
            if ( pExtData[13]&1 )
                m_pBloodSectors[nInd].triggerFlags |= TRIGGER_INTERRUPTABLE;
            if ( pExtData[55]&128 )
                m_pBloodSectors[nInd].triggerFlags |= TRIGGER_DUDE_LOCKOUT;
            
            m_pBloodSectors[nInd].bSendAtOn      = (pExtData[10]&4) ? 1 : 0;
            m_pBloodSectors[nInd].offOn_busyTime = pExtData[10]/16;
            m_pBloodSectors[nInd].offOn_wave     = pExtData[7]/4;
            m_pBloodSectors[nInd].offOn_waitTime = pExtData[12];
            //make sure some wait time if flagged... (?)
            if ( pExtData[15]&64 )
                m_pBloodSectors[nInd].offOn_waitTime = MAX(1, m_pBloodSectors[nInd].offOn_waitTime);

            m_pBloodSectors[nInd].bSendAtOff     = (pExtData[10]&8) ? 1 : 0;
            m_pBloodSectors[nInd].onOff_busyTime = pExtData[24]/4;
            m_pBloodSectors[nInd].onOff_wave     = pExtData[7]/32;
            m_pBloodSectors[nInd].onOff_waitTime = pExtData[25]/64 + pExtData[26]*4;
            //make sure some wait time if flagged... (?)
            if ( pExtData[15]&128 )
                m_pBloodSectors[nInd].onOff_waitTime = MAX(1, m_pBloodSectors[nInd].onOff_waitTime);

            m_pBloodSectors[nInd].triggerOnEvent = pExtData[23];
            m_pBloodSectors[nInd].exData         = pExtData[4];
            m_pBloodSectors[nInd].key            = (pExtData[22]>>7) + pExtData[23]*2;
            m_pBloodSectors[nInd].depth          = (pExtData[19]>>5);

            m_pBloodSectors[nInd].bUnderwater    = (pExtData[19]&16) ? 1 : 0;
            m_pBloodSectors[nInd].bCrushing      = (pExtData[48]&1)  ? 1 : 0;

            m_pBloodSectors[nInd].floorStates[0] = *((int32_t *)&pExtData[36]);
            m_pBloodSectors[nInd].floorStates[1] = *((int32_t *)&pExtData[40]);

            m_pBloodSectors[nInd].ceilStates[0] = *((int32_t *)&pExtData[28]);
            m_pBloodSectors[nInd].ceilStates[1] = *((int32_t *)&pExtData[32]);

            m_pBloodSectors[nInd].lightFX_wave = pExtData[17];
            uint8_t a0 = pExtData[14]&63;
            m_pBloodSectors[nInd].lightFX_amp = a0 < 32 ? (pExtData[13]>>6) + a0*4 : (pExtData[13]>>6) + (a0-32)*4 - 128;
            m_pBloodSectors[nInd].lightFX_freq = (pExtData[14]>>6) + (pExtData[15]&63)*4;
            m_pBloodSectors[nInd].lightFX_phase = pExtData[16];
            m_pBloodSectors[nInd].lightFX_flags = pExtData[17]&0xf0;
            if ( pExtData[24]&1 ) m_pBloodSectors[nInd].lightFX_flags |= LIGHTFX_COLORED_LIGHTS;
            m_pBloodSectors[nInd].lightFX_ceilPal2  = pExtData[27]>>4;
            m_pBloodSectors[nInd].lightFX_floorPal2 = pExtData[51]>>4;

            m_pBloodSectors[nInd].motionFX_speed = pExtData[20];
            m_pBloodSectors[nInd].motionFX_angle = *((uint16_t *)&pExtData[21]);
            m_pBloodSectors[nInd].motionFX_flags = pExtData[19]&0x0f;

            m_pBloodSectors[nInd].wind_vel = ( *((uint16_t *)&pExtData[53]) )>>1;
            m_pBloodSectors[nInd].wind_vel &= 1023;

            m_pBloodSectors[nInd].wind_ang = ( *((uint16_t *)&pExtData[54]) )>>3;
            m_pBloodSectors[nInd].wind_ang &= 2047;
            m_pBloodSectors[nInd].bWindAlways = (pExtData[55]&64) ? true : false;

            index += 60;
        }
        else 
        {
            m_pBloodSectors[nInd].lightFX_wave = 0;
            switch (m_pBloodSectors[nInd].extra)
            {
                case 0:
                case -1:
                    break;
                default:
                    XL_Console::PrintF("^1Error: Unknown extra data value (%hd).", m_pBloodSectors[nInd].extra);
                    return false;
            }
        }
    }

    return true;
}

bool CellLoader_BloodMap::ExtractWalls(char *pData, int32_t& index)
{
    // Variables
    uint8_t Buffer[128];
    uint8_t uDecryptKey;

    assert( m_bloodData.wallCnt > 0 );
    uDecryptKey = (((m_bloodData.revisions * sizeof (BloodSector)) | 0x4d) & 0xFF);

    m_pBloodWalls = xlNew BloodWall_Xtra[m_bloodData.wallCnt];
    
    for (int32_t nInd = 0; nInd < m_bloodData.wallCnt; nInd++)
    {
        memcpy(Buffer, &pData[index], sizeof(BloodWall));
        DecryptBuffer(Buffer, sizeof(BloodWall), uDecryptKey);

        memcpy(&m_pBloodWalls[nInd], Buffer, sizeof(BloodWall));
        index += sizeof(BloodWall);

        // If extra data follow
        if (m_pBloodWalls[nInd].extra > 0) // TESTME: <= 511 && WallPtr->extra >= 256
        {
            uint8_t *pExtData = (uint8_t *)&pData[index];

            m_pBloodWalls[nInd].rxID = 0;
            m_pBloodWalls[nInd].txID = pExtData[6];

            // skip the next 24 bytes
            index += 24;
        }
        else
        {
            switch (m_pBloodWalls[nInd].extra)
            {
                case 0:
                case -1:
                    break;
                default:
                    XL_Console::PrintF("^1Error: Unknown extra data value (%hd)", m_pBloodWalls[nInd].extra);
                    return false;
            }
        }
    }

    return true;
}

bool CellLoader_BloodMap::ExtractSprites(char *pData, int32_t& index)
{
    // Variables
    uint8_t Buffer[128];
    uint8_t uDecryptKey;

    uDecryptKey = (((m_bloodData.revisions * sizeof(BloodSprite)) | 0x4d) & 0xFF);
    m_pBloodSprites = xlNew BloodSprite_Xtra[m_bloodData.spriteCnt];

    for (int32_t nInd = 0; nInd < m_bloodData.spriteCnt; nInd++)
    {
        memcpy(Buffer, &pData[index], sizeof(BloodSprite));
        DecryptBuffer(Buffer, sizeof(BloodSprite), uDecryptKey);

        memcpy(&m_pBloodSprites[nInd], Buffer, sizeof(BloodSprite));
        index += sizeof(BloodSprite);

        // If extra data follow
        if (m_pBloodSprites[nInd].extra > 0) //<= 2047 && SpritePtr->extra >= 1024)
        {
            if ( nInd == 152 )
            {
                static int _x=0;
                _x++;
            }

            m_pBloodSprites[nInd].data[0] = ((uint8_t *)&pData[index])[16];
            m_pBloodSprites[nInd].data[1] = ((uint8_t *)&pData[index])[17];
            m_pBloodSprites[nInd].data[2] = ((uint8_t *)&pData[index])[18];
            m_pBloodSprites[nInd].data[3] = ((uint8_t *)&pData[index])[19];

            m_pBloodSprites[nInd].txID    = ((uint8_t *)&pData[index])[4];
            // skip the next 56 bytes
            index += 56;
        }
        else 
        {
            m_pBloodSprites[nInd].data[0] = 0;
            m_pBloodSprites[nInd].data[1] = 0;
            m_pBloodSprites[nInd].data[2] = 0;
            m_pBloodSprites[nInd].data[3] = 0;

            switch (m_pBloodSprites[nInd].extra)
            {
                case -1: // The end ?
                    break;
                default:
                    XL_Console::PrintF("^1Error: Unknown extra data value (%hd)", m_pBloodSprites[nInd].extra);
                    return false;
            }
        }
    }

    return true;
}

TextureHandle CellLoader_BloodMap::AddBloodTile(int32_t picnum, uint32_t uPalIdx, uint32_t& uWidth, uint32_t& uHeight, bool bMip)
{
    int32_t nArtIndex = picnum / 256;
    char szTileArchive[64];
    char szTileName[32];

    if ( picnum >= MAX_TILES )
         picnum = 0;

    if ( uPalIdx > 14 )
        uPalIdx = 0;

    sprintf(szTileArchive, "TILES%03d.ART", nArtIndex);
    sprintf(szTileName, "%d", picnum);

    TextureHandle hTex = TextureCache::GameFile_LoadTexture(TEXTURETYPE_ART, uPalIdx, ARCHIVETYPE_ART, szTileArchive, szTileName, bMip);
    if ( hTex != XL_INVALID_TEXTURE )
    {
        float fRelSizeX, fRelSizeY;
        int32_t nOffsX, nOffsY;
        TextureCache::GetTextureSize(nOffsX, nOffsY, uWidth, uHeight, fRelSizeX, fRelSizeY);
    }

    return hTex;
}

WorldCell *CellLoader_BloodMap::Load( IDriver3D *pDriver, World *pWorld, uint8_t *pData, uint32_t uLen, const std::string& sFile, int32_t worldX, int32_t worldY )
{
    WorldCell *pCell = NULL;
    ObjectManager::FreeAllObjects();
    if ( uLen )
    {
        int32_t index = 0;
        ParseHeader((char *)pData, index);
        int32_t offset = FindFirstSector((char *)pData, index);
        if ( offset < 0 )
        {
            return NULL;
        }
        index = offset;
        if ( ExtractSectors((char *)pData, index) == false )
        {
            return NULL;
        }
        if ( ExtractWalls((char *)pData, index) == false )
        {
            return NULL;
        }
        if ( ExtractSprites((char *)pData, index) == false )
        {
            return NULL;
        }

        pCell = xlNew WorldCell();
        LevelFuncMgr::SetWorldCell( pCell );

        const float fHorizScale = (256.0f*0.75f);
        const float fVertScale = (2048.0f*1.5f);
        const float fFloorTexScale = fHorizScale*128.0f/2048.0f;

        int32_t secLink = -1;
        int32_t secLinkBase = -1;

        for (int32_t i=0; i<m_bloodData.secCnt; i++)
        {
            Sector_2_5D *pSector = xlNew Sector_2_5D();
            uint32_t uWidth, uHeight;
            if ( EngineSettings::get().IsServer() == false )
            {
                pSector->m_hFloorTex = AddBloodTile( m_pBloodSectors[i].floorpicnum, m_pBloodSectors[i].floorpal, uWidth, uHeight );
                pSector->m_FloorTexScale.Set( fFloorTexScale / (float)uWidth, fFloorTexScale / (float)uHeight );

                pSector->m_hCeilTex  = AddBloodTile( m_pBloodSectors[i].ceilingpicnum, m_pBloodSectors[i].ceilingpal, uWidth, uHeight );
                pSector->m_CeilTexScale.Set( fFloorTexScale / (float)uWidth, fFloorTexScale / (float)uHeight );

                pSector->m_texOffset[0].x = (float)m_pBloodSectors[i].floorxpanning / (float)uWidth;
                pSector->m_texOffset[0].y = (float)m_pBloodSectors[i].floorypanning / (float)uHeight;
                pSector->m_texOffset[1].x = (float)m_pBloodSectors[i].ceilingxpanning / (float)uWidth;
                pSector->m_texOffset[1].y = (float)m_pBloodSectors[i].ceilingypanning / (float)uHeight;
            }
            else
            {   //We don't load textures when running as a server.
                pSector->m_hFloorTex = XL_INVALID_TEXTURE;
                pSector->m_hCeilTex  = XL_INVALID_TEXTURE;
                pSector->m_FloorTexScale.Set( fFloorTexScale / 64.0f, fFloorTexScale / 64.0f );
                pSector->m_CeilTexScale.Set( fFloorTexScale / 64.0f, fFloorTexScale / 64.0f );

                pSector->m_texOffset[0].Set(0,0);
                pSector->m_texOffset[1].Set(0,0);
            }

            pSector->m_uVertexCount = m_pBloodSectors[i].wallnum;
            pSector->m_uWallCount   = m_pBloodSectors[i].wallnum; 
            pSector->m_ZRangeBase.x = -(float)m_pBloodSectors[i].floorz   / fVertScale;
            pSector->m_ZRangeBase.y = -(float)m_pBloodSectors[i].ceilingz / fVertScale;
            pSector->m_ZRangeCur = pSector->m_ZRangeBase;

            pSector->m_uFlags = Sector_2_5D::SEC_FLAGS_NONE;
            if ( m_pBloodSectors[i].floorstat&1 )   pSector->m_uFlags |= Sector_2_5D::SEC_FLAGS_SKYFLOOR;
            if ( m_pBloodSectors[i].ceilingstat&1 ) pSector->m_uFlags |= Sector_2_5D::SEC_FLAGS_EXTERIOR;
            if ( m_pBloodSectors[i].floorstat&2 )   pSector->m_uFlags |= Sector_2_5D::SEC_FLAGS_FLOOR_SLOPE;
            if ( m_pBloodSectors[i].ceilingstat&2 ) pSector->m_uFlags |= Sector_2_5D::SEC_FLAGS_CEIL_SLOPE;

            if ( EngineSettings::get().IsServer() == false )
            {
                if ( pSector->m_uFlags&Sector_2_5D::SEC_FLAGS_EXTERIOR )
                {
                    for (uint32_t s=0; s<16; s++)
                    {
                        pCell->SetSkyTex( s, AddBloodTile( m_pBloodSectors[i].ceilingpicnum+s, 0, uWidth, uHeight ), uWidth, uHeight );
                    }
                    pCell->SetSkyTexCount(16);
                }
            }

            if ( m_pBloodSectors[i].floorstat&0x40 )   
                pSector->m_uFlags |= Sector_2_5D::SEC_FLAGS_FLOOR_FLIP;

            const float fSlopeScale = 0.75f;
            pSector->m_fFloorSlope = -(float)m_pBloodSectors[i].floorheinum   / fVertScale * fSlopeScale;
            pSector->m_fCeilSlope  = -(float)m_pBloodSectors[i].ceilingheinum / fVertScale * fSlopeScale;
            pSector->m_auSlopeAnchor[0] = 0;
            pSector->m_auSlopeAnchor[1] = 0;
            pSector->m_auSlopeSector[0] = i;
            pSector->m_auSlopeSector[1] = i;

            int32_t nAmbient = 255 - m_pBloodSectors[i].floorshade*4;
            if ( nAmbient < 0 ) nAmbient = 0;
            if ( nAmbient > 255 ) nAmbient = 255;
            pSector->m_uAmbientFloor = (uint8_t)nAmbient;

            nAmbient = 255 - m_pBloodSectors[i].ceilingshade*4;
            if ( nAmbient < 0 ) nAmbient = 0;
            if ( nAmbient > 255 ) nAmbient = 255;
            pSector->m_uAmbientCeil = (uint8_t)nAmbient;

            int32_t w;
            //extract vertices.
            pSector->m_pVertexBase = xlNew Vector2[pSector->m_uVertexCount];
            pSector->m_pVertexCur  = xlNew Vector2[pSector->m_uVertexCount];
            for (w=0; w<m_pBloodSectors[i].wallnum; w++)
            {
                int32_t ww = m_pBloodSectors[i].wallptr + w;
                pSector->m_pVertexBase[w].x =  (float)m_pBloodWalls[ww].x / fHorizScale;
                pSector->m_pVertexBase[w].y = -(float)m_pBloodWalls[ww].y / fHorizScale;
                pSector->m_pVertexCur[w] = pSector->m_pVertexBase[w];
            }

            /****************** Setup LevelFunc ****************/
            //is this a sliding door sector?
            pSector->m_pFunc = NULL;
            int16_t rxID = -1;
            int16_t waitTime = 0;
            bool bLink = false;
            bool bTriggerAllWalls = false;
            if ( m_pBloodSectors[i].lotag == 600 )  //Z-Motion
            {
                //z-motion - like an elevator.
                pSector->m_pFunc = NULL;

                //floor or ceiling, add for both?
                if ( m_pBloodSectors[i].floorStates[0] != m_pBloodSectors[i].floorStates[1] )
                {
                    pSector->m_pFunc = LevelFuncMgr::CreateLevelFunc("Elevator_MoveFloor", i, -1);
                    pSector->m_pFunc->SetSpeed( 0.1f );
                    pSector->m_pFunc->SetAccel( 0.025f );
                    if ( m_pBloodSectors[i].startState == 0 )
                    {
                        //this is the start location.
                        pSector->m_pFunc->AddState( -(float)m_pBloodSectors[i].floorStates[0] / fVertScale, LevelFunc::ST_HOLD, 0 );
                        //this is the end location.
                        pSector->m_pFunc->AddState( -(float)m_pBloodSectors[i].floorStates[1] / fVertScale, m_pBloodSectors[i].offOn_waitTime == 0 ? LevelFunc::ST_TERMINATE : LevelFunc::ST_TIME, (float)m_pBloodSectors[i].offOn_waitTime * 4.5f );
                    }
                    else    //do this or just change the initial state?
                    {
                        //this is the start location.
                        pSector->m_pFunc->AddState( -(float)m_pBloodSectors[i].floorStates[1] / fVertScale, LevelFunc::ST_HOLD, 0 );
                        //this is the end location.
                        pSector->m_pFunc->AddState( -(float)m_pBloodSectors[i].floorStates[0] / fVertScale, m_pBloodSectors[i].onOff_waitTime == 0 ? LevelFunc::ST_TERMINATE : LevelFunc::ST_TIME, (float)m_pBloodSectors[i].onOff_waitTime * 4.5f );
                    }
                }
                else if ( m_pBloodSectors[i].ceilStates[0] != m_pBloodSectors[i].ceilStates[1] )
                {
                    pSector->m_pFunc = LevelFuncMgr::CreateLevelFunc("Elevator_MoveCeil", i, -1);
                    pSector->m_pFunc->SetSpeed( 0.1f );
                    pSector->m_pFunc->SetAccel( 0.025f );
                    if ( m_pBloodSectors[i].startState == 0 )
                    {
                        //this is the start location.
                        pSector->m_pFunc->AddState( -(float)m_pBloodSectors[i].ceilStates[0] / fVertScale, LevelFunc::ST_HOLD, 0 );
                        //this is the end location.
                        pSector->m_pFunc->AddState( -(float)m_pBloodSectors[i].ceilStates[1] / fVertScale, m_pBloodSectors[i].offOn_waitTime == 0 ? LevelFunc::ST_TERMINATE : LevelFunc::ST_TIME, (float)m_pBloodSectors[i].offOn_waitTime * 4.5f );
                    }
                    else    //do this or just change the initial state?
                    {
                        //this is the start location.
                        pSector->m_pFunc->AddState( -(float)m_pBloodSectors[i].ceilStates[1] / fVertScale, LevelFunc::ST_HOLD, 0 );
                        //this is the end location.
                        pSector->m_pFunc->AddState( -(float)m_pBloodSectors[i].ceilStates[0] / fVertScale, m_pBloodSectors[i].onOff_waitTime == 0 ? LevelFunc::ST_TERMINATE : LevelFunc::ST_TIME, (float)m_pBloodSectors[i].onOff_waitTime * 4.5f );
                    }
                }
                //else
                //{
                    //what are we supposed to do now?
                    //assert(0);
                //}

                if ( pSector->m_pFunc && (m_pBloodSectors[i].triggerOnEvent&TRIGGERON_WALLPUSH) )
                {
                    bTriggerAllWalls = true;
                }
            }
            else if ( m_pBloodSectors[i].lotag == 614 ) //Sliding Marked
            {
                //ok now which walls and sprites are we going to look at?
                rxID = m_pBloodSectors[i].rxID;
                //get the wait time.
                waitTime = m_pBloodSectors[i].offOn_waitTime;

                pSector->m_pFunc = LevelFuncMgr::CreateLevelFunc("SlidingDoor", i, -1);

                //now we have to find the appropriate sprites to set this up...
                int32_t onMarker = -1, offMarker = -1;
                for (int32_t s=0; s<m_bloodData.spriteCnt; s++)
                {
                    if ( m_pBloodSprites[s].sectnum != i && m_pBloodSprites[s].owner != i )
                        continue;

                    if ( m_pBloodSprites[s].lotag == 3 )    //Off Marker
                    {
                        offMarker = s;
                    }
                    else if ( m_pBloodSprites[s].lotag == 4 ) //On Marker.
                    {
                        onMarker = s;
                    }

                    if ( offMarker > -1 && onMarker > -1 )
                        break;
                }

                if ( offMarker > -1 && onMarker > -1 )
                {
                    Vector3 vDir;
                    Vector3 A, B;
                    A.x =  (float)m_pBloodSprites[onMarker].x / fHorizScale;
                    A.y = -(float)m_pBloodSprites[onMarker].y / fHorizScale;
                    A.z = -(float)m_pBloodSprites[onMarker].z / fVertScale;

                    B.x =  (float)m_pBloodSprites[offMarker].x / fHorizScale;
                    B.y = -(float)m_pBloodSprites[offMarker].y / fHorizScale;
                    B.z = -(float)m_pBloodSprites[offMarker].z / fVertScale;

                    vDir = B - A;
                    float m = vDir.Normalize();

                    //set the speed and direction.
                    pSector->m_pFunc->SetSpeed( (float)(m_pBloodSprites[offMarker].xvel) / (fHorizScale*10.0f) );
                    pSector->m_pFunc->SetAccel( 0.025f );
                    pSector->m_pFunc->SetDirection( vDir );
                    if ( m_pBloodSectors[i].startState == 0 )
                    {
                        //this is the start location.
                        pSector->m_pFunc->AddState( m, LevelFunc::ST_HOLD, 0 );
                        //this is the end location.
                        pSector->m_pFunc->AddState( 0.0f, m_pBloodSectors[i].offOn_waitTime == 0 ? LevelFunc::ST_TERMINATE : LevelFunc::ST_TIME, (float)m_pBloodSectors[i].offOn_waitTime * 4.5f );
                    }
                    else    //do this or just change the initial state?
                    {
                        //this is the start location.
                        pSector->m_pFunc->AddState( 0.0f, LevelFunc::ST_HOLD, 0 );
                        //this is the end location.
                        pSector->m_pFunc->AddState( m, m_pBloodSectors[i].onOff_waitTime == 0 ? LevelFunc::ST_TERMINATE : LevelFunc::ST_TIME, (float)m_pBloodSectors[i].onOff_waitTime * 4.5f );
                    }
                }
            }
            else if ( m_pBloodSectors[i].lotag == 616 ) //Sliding
            {
                //ok now which walls and sprites are we going to look at?
                rxID = m_pBloodSectors[i].rxID;
                //get the wait time.
                waitTime = m_pBloodSectors[i].offOn_waitTime;

                pSector->m_pFunc = LevelFuncMgr::CreateLevelFunc("Slide", i, -1);

                //now we have to find the appropriate sprites to set this up...
                int32_t onMarker = -1, offMarker = -1;
                for (int32_t s=0; s<m_bloodData.spriteCnt; s++)
                {
                    if ( m_pBloodSprites[s].sectnum != i && m_pBloodSprites[s].owner != i )
                        continue;

                    if ( m_pBloodSprites[s].lotag == 3 )    //Off Marker
                    {
                        offMarker = s;
                    }
                    else if ( m_pBloodSprites[s].lotag == 4 ) //On Marker.
                    {
                        onMarker = s;
                    }

                    if ( offMarker > -1 && onMarker > -1 )
                        break;
                }

                if ( offMarker > -1 && onMarker > -1 )
                {
                    Vector3 vDir;
                    Vector3 A, B;
                    A.x =  (float)m_pBloodSprites[onMarker].x / fHorizScale;
                    A.y = -(float)m_pBloodSprites[onMarker].y / fHorizScale;
                    A.z = -(float)m_pBloodSprites[onMarker].z / fVertScale;

                    B.x =  (float)m_pBloodSprites[offMarker].x / fHorizScale;
                    B.y = -(float)m_pBloodSprites[offMarker].y / fHorizScale;
                    B.z = -(float)m_pBloodSprites[offMarker].z / fVertScale;

                    vDir = B - A;
                    float m = vDir.Normalize();

                    //set the speed and direction.
                    pSector->m_pFunc->SetSpeed( (float)(m_pBloodSprites[offMarker].xvel) / (fHorizScale*10.0f) );
                    pSector->m_pFunc->SetAccel( 0.025f );
                    pSector->m_pFunc->SetDirection( vDir );

                    if ( m_pBloodSectors[i].startState == 0 )
                    {
                        //this is the start location.
                        pSector->m_pFunc->AddState( m, LevelFunc::ST_HOLD, 0 );
                        //this is the end location.
                        pSector->m_pFunc->AddState( 0.0f, m_pBloodSectors[i].offOn_waitTime == 0 ? LevelFunc::ST_TERMINATE : LevelFunc::ST_TIME, (float)m_pBloodSectors[i].offOn_waitTime * 4.5f );
                    }
                    else    //do this or just change the initial state?
                    {
                        //this is the start location.
                        pSector->m_pFunc->AddState( 0.0f, LevelFunc::ST_HOLD, 0 );
                        //this is the end location.
                        pSector->m_pFunc->AddState( m, m_pBloodSectors[i].onOff_waitTime == 0 ? LevelFunc::ST_TERMINATE : LevelFunc::ST_TIME, (float)m_pBloodSectors[i].onOff_waitTime * 4.5f );
                    }
                }
            }
            else if ( m_pBloodSectors[i].lotag == 617 ) //Rotate
            {
                pSector->m_pFunc = LevelFuncMgr::CreateLevelFunc("Rotate", i, -1);

                bLink = false;
                if ( m_pBloodSectors[i].cmd == 5 ) //LINK
                {
                    secLink = m_pBloodSectors[i].txID;
                    secLinkBase = i;
                }
                else if ( m_pBloodSectors[i].rxID == secLink )
                {
                    bLink = true;
                    Sector_2_5D *pLink = (Sector_2_5D *)pCell->GetSector( secLinkBase );
                    //we have to add this as a client to the previous sector's walls...
                    for (w=0; w<m_pBloodSectors[secLinkBase].wallnum; w++)
                    {
                        if ( pLink->m_Walls[w].m_pFunc )
                        {
                            pLink->m_Walls[w].m_pFunc->AddClient( pSector->m_pFunc );
                        }
                    }
                }

                //now we have to find the appropriate sprites to set this up...
                int32_t pivot = -1;
                for (int32_t s=0; s<m_bloodData.spriteCnt; s++)
                {
                    if ( m_pBloodSprites[s].sectnum != i && m_pBloodSprites[s].owner != i )
                        continue;

                    if ( m_pBloodSprites[s].lotag == 5 )    //Pivot
                    {
                        pivot = s;
                        break;
                    }
                }

                if ( pivot > -1 )
                {
                    float angle = -(float)m_pBloodSprites[pivot].ang/2048.0f * MATH_TWO_PI;
                    pSector->m_pFunc->SetAccel( 0.0125f );

                    Vector3 pivotPos;
                    pivotPos.x =  (float)m_pBloodSprites[pivot].x / fHorizScale;
                    pivotPos.y = -(float)m_pBloodSprites[pivot].y / fHorizScale;
                    pivotPos.z = -(float)m_pBloodSprites[pivot].z / fVertScale;

                    pSector->m_pFunc->SetPivot( pivotPos );
                    //use the linked stats...
                    int32_t nSec = i;
                    if ( bLink )
                    {
                        nSec = secLinkBase;
                    }

                    if ( m_pBloodSectors[nSec].startState == 0 )
                    {
                        //this is the start location.
                        pSector->m_pFunc->AddState( angle, LevelFunc::ST_HOLD, 0 );
                        //this is the end location.
                        pSector->m_pFunc->AddState( 0.0f, m_pBloodSectors[nSec].offOn_waitTime == 0 ? LevelFunc::ST_TERMINATE : LevelFunc::ST_TIME, (float)m_pBloodSectors[nSec].offOn_waitTime * 4.5f );

                        pSector->m_pFunc->SetSpeed( fabsf(angle)*0.5f/(float)m_pBloodSectors[nSec].offOn_busyTime );
                    }
                    else    //do this or just change the initial state?
                    {
                        //this is the start location.
                        pSector->m_pFunc->AddState( 0.0f, LevelFunc::ST_HOLD, 0 );
                        //this is the end location.
                        pSector->m_pFunc->AddState( angle, m_pBloodSectors[nSec].onOff_waitTime == 0 ? LevelFunc::ST_TERMINATE : LevelFunc::ST_TIME, (float)m_pBloodSectors[nSec].onOff_waitTime * 4.5f );

                        pSector->m_pFunc->SetSpeed( fabsf(angle)*0.5f/(float)m_pBloodSectors[nSec].onOff_busyTime );
                    }
                }

                if ( bLink || m_pBloodSectors[i].cmd == 5 || (m_pBloodSectors[i].triggerOnEvent&TRIGGERON_WALLPUSH) )
                {
                    bTriggerAllWalls = true;
                }
            }

            if ( /*pSector->m_pFunc == NULL && */(m_pBloodSectors[i].motionFX_flags&MOTIONFX_PANALWAYS) )
            {
                pSector->m_pFunc = LevelFuncMgr::CreateLevelFunc("MotionFX", i, -1);
                pSector->m_pFunc->SetSpeed( (float)m_pBloodSectors[i].motionFX_speed/(64.0f*30.0f) );

                float angle = (float)m_pBloodSectors[i].motionFX_angle/2048.0f * MATH_TWO_PI;

                Vector3 dir;
                dir.x = -cosf(angle);
                dir.y =  sinf(angle);
                dir.z = 0.0f;
                dir.Normalize();

                pSector->m_pFunc->SetDirection(dir);
            }
            else if ( pSector->m_pFunc == NULL && m_pBloodSectors[i].lightFX_wave > 0 )
            {
                pSector->m_pFunc = LevelFuncMgr::CreateLevelFunc("LightFX", i, -1);
                pSector->m_pFunc->SetSpeed( 0.33f/60.0f );
                pSector->m_pFunc->AddState( 0.0f, LevelFunc::ST_TIME, 0 );
                pSector->m_pFunc->AddState( 1.0f, LevelFunc::ST_TIME, 0 );
            }

            /************** Setup walls *******************/
            //extract walls.
            float dz = pSector->m_ZRangeBase.y - pSector->m_ZRangeBase.x;
            pSector->m_Walls = xlNew Wall[pSector->m_uWallCount];
            for (w=0; w<m_pBloodSectors[i].wallnum; w++)
            {
                int32_t ww = m_pBloodSectors[i].wallptr + w;

                pSector->m_Walls[w].m_pFunc = NULL;
                if ( (rxID > -1 && m_pBloodWalls[ww].txID == rxID) || bTriggerAllWalls )
                {
                    pSector->m_Walls[w].m_pFunc = LevelFuncMgr::CreateLevelFunc("TriggerToggle", i, w);
                    pSector->m_Walls[w].m_pFunc->AddClient( pSector->m_pFunc );

                    if ( bLink )
                    {
                        Sector_2_5D *pLink = (Sector_2_5D *)pCell->GetSector( secLinkBase );
                        pSector->m_Walls[w].m_pFunc->AddClient( pLink->m_pFunc );
                    }
                }

                //compute the wall length.
                pSector->m_Walls[w].m_idx[0] = w;
                pSector->m_Walls[w].m_idx[1] = m_pBloodWalls[ww].point2 - m_pBloodSectors[i].wallptr;
                Vector2 delta = pSector->m_pVertexBase[ pSector->m_Walls[w].m_idx[1] ] - pSector->m_pVertexBase[ pSector->m_Walls[w].m_idx[0] ];
                float m2 = delta.Dot(delta);

                if ( m2 > 0.0001f )
                    pSector->m_Walls[w].m_wallLen = sqrtf( m2 );
                else
                    pSector->m_Walls[w].m_wallLen = 1.0f;

                //compute texture scale.
                pSector->m_Walls[w].m_flags = 0;
                TextureHandle hTex = XL_INVALID_TEXTURE;
                float dx = 0.0f, dy = 0.0f, sx = 1.0f, sy = 1.0f;
                if ( EngineSettings::get().IsServer() == false )
                {
                    hTex = AddBloodTile( m_pBloodWalls[ww].picnum, m_pBloodWalls[ww].pal, uWidth, uHeight );

                    dx =  (float)m_pBloodWalls[ww].xpanning / (float)uWidth;
                    dy =  -2.0f*(float)m_pBloodWalls[ww].ypanning / (float)uHeight;

                    sx = (float)m_pBloodWalls[ww].xrepeat * 8.0f / (pSector->m_Walls[w].m_wallLen * (float)uWidth);
                    sy = 12.0f * (float)m_pBloodWalls[ww].yrepeat / (float)(uHeight*8);

                    pSector->m_Walls[w].m_textures[Wall::WALL_TEX_TOP] = hTex;
                    pSector->m_Walls[w].m_texOffset[Wall::WALL_TEX_TOP].Set(dx, dy);
                    pSector->m_Walls[w].m_texScale[Wall::WALL_TEX_TOP].Set(sx, sy);
                    pSector->m_Walls[w].m_textures[Wall::WALL_TEX_BOT] = hTex;
                    pSector->m_Walls[w].m_texOffset[Wall::WALL_TEX_BOT].Set(dx, dy);
                    pSector->m_Walls[w].m_texScale[Wall::WALL_TEX_BOT].Set(sx, sy);
                    pSector->m_Walls[w].m_textures[Wall::WALL_TEX_SIGN] = XL_INVALID_TEXTURE;
                    pSector->m_Walls[w].m_texOffset[Wall::WALL_TEX_SIGN].Set(0.0f, 0.0f);
                    pSector->m_Walls[w].m_texScale[Wall::WALL_TEX_SIGN].Set(1.0f, 1.0f);
                }

                if ( m_pBloodWalls[ww].nextsector > -1 )
                {
                    pSector->m_Walls[w].m_adjoin[0] = m_pBloodWalls[ww].nextsector;
                    pSector->m_Walls[w].m_mirror[0] = m_pBloodWalls[ww].nextwall - m_pBloodSectors[ m_pBloodWalls[ww].nextsector ].wallptr;

                    //is this a maskwall?
                    if ( m_pBloodWalls[ww].cstat&16 )
                    {
                        pSector->m_Walls[w].m_flags |= Wall::WALL_FLAGS_MASKWALL;
                        if ( m_pBloodWalls[ww].cstat&128 )
                        {
                            pSector->m_Walls[w].m_flags |= Wall::WALL_FLAGS_TRANS;
                        }

                        if ( EngineSettings::get().IsServer() == false )
                        {
                            pSector->m_Walls[w].m_textures[Wall::WALL_TEX_MID] = AddBloodTile( m_pBloodWalls[ww].overpicnum, m_pBloodWalls[ww].pal, uWidth, uHeight );;

                            dx =  (float)m_pBloodWalls[ww].xpanning / (float)uWidth;
                            dy =  -2.0f*(float)m_pBloodWalls[ww].ypanning / (float)uHeight;

                            sx = (float)m_pBloodWalls[ww].xrepeat * 8.0f / (pSector->m_Walls[w].m_wallLen * (float)uWidth);
                            sy = 12.0f * (float)m_pBloodWalls[ww].yrepeat / (float)(uHeight*8);

                            pSector->m_Walls[w].m_texOffset[Wall::WALL_TEX_MID].Set(dx, dy);
                            pSector->m_Walls[w].m_texScale[Wall::WALL_TEX_MID].Set(sx, sy);
                        }
                    }
                    else
                    {
                        pSector->m_Walls[w].m_textures[Wall::WALL_TEX_MID] = hTex;
                        pSector->m_Walls[w].m_texOffset[Wall::WALL_TEX_MID].Set(dx, dy);
                        pSector->m_Walls[w].m_texScale[Wall::WALL_TEX_MID].Set(sx, sy);
                    }
                }
                else
                {
                    pSector->m_Walls[w].m_adjoin[0] = SOLID_WALL;
                    pSector->m_Walls[w].m_mirror[0] = SOLID_WALL;

                    pSector->m_Walls[w].m_textures[Wall::WALL_TEX_MID] = hTex;
                    pSector->m_Walls[w].m_texOffset[Wall::WALL_TEX_MID].Set(dx, dy);
                    pSector->m_Walls[w].m_texScale[Wall::WALL_TEX_MID].Set(sx, sy);
                }

                pSector->m_Walls[w].m_adjoin[1] = SOLID_WALL;
                pSector->m_Walls[w].m_mirror[1] = SOLID_WALL;

                if ( pSector->m_Walls[w].m_idx[0] >= m_pBloodSectors[i].wallnum )
                {
                    assert(0);
                }
                nAmbient = 255 - m_pBloodWalls[ww].shade*4;
                if ( nAmbient < 0 ) nAmbient = 0;
                if ( nAmbient > 255 ) nAmbient = 255;
                pSector->m_Walls[w].m_lightDelta = nAmbient;
                
                pSector->m_Walls[w].m_adjoin[1] = SOLID_WALL;
                pSector->m_Walls[w].m_mirror[1] = SOLID_WALL;

                if ( m_pBloodWalls[ww].cstat&8 )
                {
                    pSector->m_Walls[w].m_flags |= Wall::WALL_FLAGS_XFLIP;
                }
                if ( m_pBloodWalls[ww].cstat&32 )
                {
                    pSector->m_Walls[w].m_flags |= Wall::WALL_FLAGS_SOLIDTEX;
                }
                if ( m_pBloodWalls[ww].cstat&256 )
                {
                    pSector->m_Walls[w].m_flags |= Wall::WALL_FLAGS_YFLIP;
                }
                if ( m_pBloodWalls[ww].cstat&16384 )
                {
                    pSector->m_Walls[w].m_flags |= Wall::WALL_FLAGS_MORPH;
                }
                if ( m_pBloodWalls[ww].cstat&32768 )
                {
                    pSector->m_Walls[w].m_flags |= Wall::WALL_FLAGS_INV_MORPH;
                }
            }

            pCell->AddSector( pSector );
        }

        int32_t _nPlayerStart = -1;
        for (int32_t i=0; i<m_bloodData.spriteCnt; i++)
        {
            //let's make a unique list of lotags for this level...
            int32_t lotag = m_pBloodSprites[i].lotag;

            if ( (m_pBloodSprites[i].cstat&0x8000) == 0 )
            {
                //floor oriented sprite.
                if ( m_pBloodSprites[i].cstat&0x20 )
                {
                    if ( m_pBloodSprites[i].picnum == 0 || m_pBloodSprites[i].lotag == 408 || EngineSettings::get().IsServer() )
                        continue;

                    uint32_t uWidth, uHeight;
                    TextureHandle hTex = AddBloodTile( m_pBloodSprites[i].picnum, m_pBloodSprites[i].pal, uWidth, uHeight, false );

                    Object *pObj = ObjectManager::CreateObject("OrientedSprite");
                    if ( pObj )
                    {
                        Sector_2_5D *pSector = (Sector_2_5D *)pCell->GetSector( m_pBloodSprites[i].sectnum );
                        float fSpriteIntens = 1.0f - ((float)m_pBloodSprites[i].shade/63.0f);

                        uint32_t uObjID = pObj->GetID();
                        pObj->SetSector( m_pBloodSprites[i].sectnum );
                        
                        Vector3 vLoc;
                        vLoc.x =  (float)m_pBloodSprites[i].x / fHorizScale;
                        vLoc.y = -(float)m_pBloodSprites[i].y / fHorizScale;
                        vLoc.z = -(float)m_pBloodSprites[i].z / fVertScale;

                        Vector3 vDir = Vector3(0, 0, 1);

                        float yaw = (float)m_pBloodSprites[i].ang/2048.0f * MATH_TWO_PI + MATH_PI;
                        Vector3 vUp;
                        vUp.x =  cosf(yaw);
                        vUp.y = -sinf(yaw);
                        vUp.z = 0.0f;
                        vUp.Normalize();
                        pObj->SetUp(vUp);
                        pObj->SetDir(vDir);

                        //
                        Vector3 vScale;
                        vScale.x = (float)uWidth*m_pBloodSprites[i].xrepeat * 0.125f*0.33f/64.0f;
                        vScale.z = (float)uWidth*m_pBloodSprites[i].xrepeat * 0.125f*0.33f/64.0f;
                        vScale.y = (float)uHeight*m_pBloodSprites[i].yrepeat * 0.125f*0.33f/64.0f;

                        float vx = vScale.x*vUp.y + vScale.y*vUp.x;
                        float vy = vScale.x*vUp.x + vScale.y*vUp.y;
                        vScale.x = vx; vScale.y = vy;

                        pObj->SetScale(vScale);

                        //offset...
                        float cz = (pSector->m_ZRangeCur.y + pSector->m_ZRangeCur.x) * 0.5f;
                        if ( vLoc.z < cz )
                            vLoc.z += 0.1f;
                        else
                            vLoc.z -= 0.1f;
                        pObj->SetLoc(vLoc);

                        OrientedSprite *pSprite = xlNew OrientedSprite();
                        pSprite->SetTextureHandle( hTex );
                        pSprite->SetBaseIntensity( fSpriteIntens );

                        bool bFlipX=false, bFlipY=false;
                        if ( m_pBloodSprites[i].cstat&4 )
                            bFlipX = true;
                        if ( m_pBloodSprites[i].cstat&8 ) 
                            bFlipY = true;

                        bool bFlipAxis = false;
                        if ( fabsf(vUp.x) > fabsf(vUp.y) )
                            bFlipAxis = true;

                        pSprite->SetUV_Flip(bFlipX, bFlipY, bFlipAxis);
                        pSprite->SetAlpha( (m_pBloodSprites[i].cstat&2) ? 0.5f : 1.0f );

                        pObj->SetRenderComponent( pSprite );

                        pSector->AddObject( uObjID );
                    }
                }
                else if ( m_pBloodSprites[i].cstat&0x10 )   //wall oriented sprite.
                {
                    if ( m_pBloodSprites[i].picnum == 0 || m_pBloodSprites[i].lotag == 408 || EngineSettings::get().IsServer() )
                        continue;

                    uint32_t uWidth, uHeight;
                    TextureHandle hTex = AddBloodTile( m_pBloodSprites[i].picnum, m_pBloodSprites[i].pal, uWidth, uHeight, false);

                    Object *pObj = ObjectManager::CreateObject("OrientedSprite");
                    if ( pObj )
                    {
                        uint32_t uObjID = pObj->GetID();
                        Sector_2_5D *pSector = (Sector_2_5D *)pCell->GetSector( m_pBloodSprites[i].sectnum );
                        pObj->SetSector( m_pBloodSprites[i].sectnum );
                        
                        Vector3 vLoc;
                        vLoc.x =  (float)m_pBloodSprites[i].x / fHorizScale;
                        vLoc.y = -(float)m_pBloodSprites[i].y / fHorizScale;
                        vLoc.z = -(float)m_pBloodSprites[i].z / fVertScale;

                        float yaw = (float)m_pBloodSprites[i].ang/2048.0f * MATH_TWO_PI + MATH_PI;
                        Vector3 vDir;
                        vDir.x =  cosf(yaw);
                        vDir.y = -sinf(yaw);
                        vDir.z = 0.0f;
                        vDir.Normalize();
                        pObj->SetDir(vDir);

                        //slight offset to avoid z-fighting....
                        //
                        Vector3 vScale;
                        vScale.x = (float)uWidth *m_pBloodSprites[i].xrepeat * 0.125f*0.33f/64.0f;
                        vScale.y = (float)uWidth *m_pBloodSprites[i].xrepeat * 0.125f*0.33f/64.0f;
                        vScale.z = (float)uHeight*m_pBloodSprites[i].yrepeat * 0.125f*0.33f/64.0f;
                        pObj->SetScale(vScale);

                        //this should be 128 but that doesn't always work. Must look at this later.
                        if ( m_pBloodSprites[i].cstat&2 )
                        {
                            vLoc.z = vLoc.z - vScale.z;
                        }
                        vLoc = vLoc - vDir*0.1f;
                        pObj->SetLoc(vLoc);

                        OrientedSprite *pSprite = xlNew OrientedSprite();
                        pSprite->SetTextureHandle( hTex );

                        bool bFlipX=false, bFlipY=false, bInvMorph=false;
                        if ( m_pBloodSprites[i].cstat&4 )
                            bFlipX = true;
                        if ( m_pBloodSprites[i].cstat&8 ) 
                            bFlipY = true;
                        if ( m_pBloodSprites[i].cstat&16384 )
                            bInvMorph = true;

                        pSprite->SetUV_Flip(bFlipX, bFlipY);
                        pSprite->SetAlpha( (m_pBloodSprites[i].cstat&2) ? 0.5f : 1.0f );
                        pObj->SetRenderComponent( pSprite );

                        //does this sprite match the sector ID?
                        if ( pSector->m_pFunc && m_pBloodSectors[m_pBloodSprites[i].sectnum].rxID == m_pBloodSprites[i].txID )
                        {
                            pSector->m_pFunc->AddClientObj( pObj, bInvMorph ? Wall::WALL_FLAGS_INV_MORPH : 0 );

                            //right now there is no raycast collision against oriented sprites, so we can find the closest adjoin wall of the parent sector
                            //and give it a trigger...
                            int32_t nClosest = -1;
                            float fClosest = 1000000.0f;
                            Vector3 vObjLoc3;
                            pObj->GetLoc(vObjLoc3);
                            Vector2 vObjLoc( vObjLoc3.x, vObjLoc3.y );
                            for (uint32_t w=0; w<pSector->m_uWallCount; w++)
                            {
                                if ( pSector->m_Walls[w].m_adjoin[0] != 0xffff )
                                {
                                    Vector2 offs = pSector->m_pVertexBase[ pSector->m_Walls[w].m_idx[0] ] - vObjLoc;
                                    float dist = offs.Dot(offs);
                                    if ( dist < fClosest )
                                    {
                                        fClosest = dist;
                                        nClosest = w;
                                    }
                                }
                            }
                            if ( nClosest > -1 && pSector->m_Walls[nClosest].m_pFunc == NULL )
                            {
                                pSector->m_Walls[nClosest].m_pFunc = LevelFuncMgr::CreateLevelFunc("TriggerToggle", pSector->m_uID, nClosest);
                                pSector->m_Walls[nClosest].m_pFunc->AddClient( pSector->m_pFunc );
                                pSector->m_uFlags |= Sector_2_5D::SEC_FLAGS_ALLOW_NONSOLID_ACTIVATE;
                            }
                        }

                        pSector->AddObject( uObjID );
                    }
                }
                else if ( (m_pBloodSprites[i].cstat&48) == 0 )
                {
                    //verify that this object is visible...
                    if ( lotag == 408 || lotag == 459 || lotag >= 701 || lotag == 145 || lotag == 146 )
                        continue;

                    //enemy spawns...
                    if ( lotag >= 200 && lotag <= 220 )
                        continue;

                    //keys...
                    if ( lotag >= 100 && lotag <= 106 )
                        continue;

                    //weapons and ammo...
                    //if ( lotag >= 41 && lotag <= 79 )
                    //  continue;

                    //powerups...
                    //if ( lotag >= 107 && lotag <= 142 )
                    //  continue;

                    //need to control explode and gib objects as well... later?

                    if ( (lotag < 1 || lotag > 19) && m_pBloodSprites[i].picnum != 0 && m_pBloodSprites[i].picnum != 3566 && m_pBloodSprites[i].xrepeat > 0 && m_pBloodSprites[i].yrepeat > 0 )
                    {
                        uint32_t uWidth, uHeight;
                        TextureHandle hTex = XL_INVALID_TEXTURE;
                        TextureHandle ahAnimTex[32];
                        uint32_t auAnimWidth[32];
                        uint32_t auAnimHeight[32];
                        uint32_t uFrameCnt = 0;
                        if ( EngineSettings::get().IsServer() == false )
                        {
                            hTex = AddBloodTile( m_pBloodSprites[i].picnum, m_pBloodSprites[i].pal, uWidth, uHeight, false);
                            if ( m_pBloodSprites[i].picnum == 570 )
                            {
                                //This is a torch... it's special...
                                for (uint32_t pn = 2101; pn <= 2114; pn++)
                                {
                                    ahAnimTex[uFrameCnt] = AddBloodTile( pn, m_pBloodSprites[i].pal, auAnimWidth[uFrameCnt], auAnimHeight[uFrameCnt], false);
                                    uFrameCnt++;
                                }
                            }
                        }

                        Object *pObj = ObjectManager::CreateObject("Sprite_ZAxis");
                        if ( pObj )
                        {
                            uint32_t uObjID = pObj->GetID();
                            Sector_2_5D *pSector = (Sector_2_5D *)pCell->GetSector( m_pBloodSprites[i].sectnum );
                            pObj->SetSector( m_pBloodSprites[i].sectnum );
                            
                            Vector3 vLoc;
                            vLoc.x =  (float)m_pBloodSprites[i].x / fHorizScale;
                            vLoc.y = -(float)m_pBloodSprites[i].y / fHorizScale;
                            vLoc.z = -(float)m_pBloodSprites[i].z / fVertScale;

                            Vector3 vScale;
                            if ( EngineSettings::get().IsServer() == false )
                            {
                                vScale.x = (float)uWidth *m_pBloodSprites[i].xrepeat * 0.125f*0.33f/64.0f;
                                vScale.y = (float)uWidth *m_pBloodSprites[i].xrepeat * 0.125f*0.33f/64.0f;
                                vScale.z = (float)uHeight*m_pBloodSprites[i].yrepeat * 0.125f*0.33f/64.0f;
                            }
                            else
                            {
                                //a default scale for the server.
                                vScale.x = 64.0f * 0.125f*0.33f;
                                vScale.y = 64.0f * 0.125f*0.33f;
                                vScale.z = 64.0f * 0.125f*0.33f;
                            }
                            pObj->SetScale(vScale);

                            //this should be 128 but that doesn't always work. Must look at this later.
                            float sz = pSector->GetZ_Floor(vLoc.x, vLoc.y, pCell->GetSectors());
                            if ( (m_pBloodSprites[i].cstat&128) && vLoc.z < sz+vScale.z )
                            {
                                vLoc.z = sz + vScale.z;
                            }
                            pObj->SetLoc(vLoc);

                            if ( EngineSettings::get().IsServer() == false )  //There's no render component when running a server.
                            {
                                Sprite_ZAxis *pSprite = xlNew Sprite_ZAxis();
                                pSprite->SetTextureHandle( hTex );

                                bool bFlipX=false, bFlipY=false;
                                if ( m_pBloodSprites[i].cstat&4 )
                                    bFlipX = true;
                                if ( m_pBloodSprites[i].cstat&8 ) 
                                    bFlipY = true;

                                pSprite->SetUV_Flip(bFlipX, bFlipY);
                                pSprite->SetAlpha( (m_pBloodSprites[i].cstat&2) ? 0.5f : 1.0f );

                                for (uint32_t f=0; f<uFrameCnt; f++)
                                {
                                    pSprite->AddFX_Frame(ahAnimTex[f], auAnimWidth[f], auAnimHeight[f]);
                                }

                                pObj->SetRenderComponent( pSprite );
                            }

                            pSector->AddObject( uObjID );
                        }
                    }
                }
            }

            //lotag = 2, data = player
            if ( m_pBloodSprites[i].picnum >= 2522 && m_pBloodSprites[i].picnum <= 2529 && m_pBloodSprites[i].lotag == 2 )
            {
                int32_t player = m_pBloodSprites[i].data[0];

                //Add a MP start point to the object list.
                //The game can then find it later when it's time to spawn players.
                char szMP_Name[32];
                sprintf(szMP_Name, "mp0_start%d", player);
                Object *pObj = ObjectManager::CreateObject(szMP_Name);
                if ( pObj )
                {
                    uint32_t uObjID = pObj->GetID();
                    Sector_2_5D *pSector = (Sector_2_5D *)pCell->GetSector( m_pBloodSprites[i].sectnum );
                    pObj->SetSector( m_pBloodSprites[i].sectnum );
                    
                    Vector3 vLoc;
                    vLoc.x =  (float)m_pBloodSprites[i].x / fHorizScale;
                    vLoc.y = -(float)m_pBloodSprites[i].y / fHorizScale;
                    vLoc.z = -(float)m_pBloodSprites[i].z / fVertScale;

                    pObj->SetLoc(vLoc);
                    pSector->AddObject( uObjID );
                }
            }

            if ( m_pBloodSprites[i].picnum == 2529 && m_pBloodSprites[i].lotag == 1 )
            {
                _nPlayerStart = i;
            }
            if ( m_pBloodSprites[i].picnum == 2332 )
            {
                uint32_t idUpper = m_pBloodSprites[i].data[0];
                //we must look for a lower match...
                int32_t nMatchID = -1;
                for (int32_t ii=0; ii<m_bloodData.spriteCnt; ii++)
                {
                    if ( m_pBloodSprites[ii].picnum != 2331 )
                        continue;

                    uint32_t idLower = m_pBloodSprites[ii].data[0];
                    if ( idUpper != idLower )
                        continue;

                    //found match!
                    nMatchID = ii;
                    break;
                }
                assert( nMatchID>-1 );
                if ( nMatchID > -1 )
                {
                    Vector3 vLocUpper, vLocLower;
                    vLocUpper.x =  (float)m_pBloodSprites[i].x / fHorizScale;
                    vLocUpper.y = -(float)m_pBloodSprites[i].y / fHorizScale;
                    vLocUpper.z = -(float)m_pBloodSprites[i].z / fVertScale;

                    vLocLower.x =  (float)m_pBloodSprites[nMatchID].x / fHorizScale;
                    vLocLower.y = -(float)m_pBloodSprites[nMatchID].y / fHorizScale;
                    vLocLower.z = -(float)m_pBloodSprites[nMatchID].z / fVertScale;

                    //for now just shove it in the second adjoin...
                    int32_t secUpper = m_pBloodSprites[i].sectnum;
                    int32_t secLower = m_pBloodSprites[nMatchID].sectnum;

                    Sector_2_5D *pUpper = (Sector_2_5D *)pCell->GetSector(secUpper);
                    Sector_2_5D *pLower = (Sector_2_5D *)pCell->GetSector(secLower);

                    pUpper->m_vAdjoin[0] = secLower;
                    pUpper->m_vAdjOffset[0] = vLocUpper - vLocLower;
                    pUpper->m_vAdjOffset[0].z = pUpper->m_ZRangeBase.x - pLower->m_ZRangeBase.y;
                    pLower->m_vAdjoin[1] = secUpper;
                    pLower->m_vAdjOffset[1] = vLocLower - vLocUpper;
                    pLower->m_vAdjOffset[1].z = pLower->m_ZRangeBase.y - pUpper->m_ZRangeBase.x;

                    //is this water...
                    bool bIsWater = false;
                    if ( m_pBloodSprites[i].lotag == 9 )
                    {
                        pUpper->m_uFlags |= Sector_2_5D::SEC_FLAGS_FLOORWATER;
                        pLower->m_uFlags |= Sector_2_5D::SEC_FLAGS_UNDERWATER;
                    }
                }
            }
        }

        //now go through the sectors and setup any LevelFuncs
        uint32_t uSecCnt = pCell->GetSectorCount();
        for (uint32_t s=0; s<uSecCnt; s++)
        {
            Sector_2_5D *pSector = (Sector_2_5D *)pCell->GetSector(s);
            if ( pSector->m_pFunc )
            {
                pSector->m_pFunc->SetInitialState(0, m_pBloodSectors[s].cmd == 1 ? true : false);
            }
        }

        //start the player in the correct place.
        Object *player = ObjectManager::FindObject("PLAYER");
        if ( player && _nPlayerStart > -1 )
        {
            Vector3 vStartLoc;
            int32_t sectnum = -1;
            if ( !EngineSettings::get().GetStartPos(vStartLoc, sectnum) )
            {
                int32_t startID = _nPlayerStart;

                vStartLoc.x =  (float)m_pBloodSprites[startID].x / fHorizScale;
                vStartLoc.y = -(float)m_pBloodSprites[startID].y / fHorizScale;
                vStartLoc.z = -(float)m_pBloodSprites[startID].z / fVertScale;

                sectnum = m_pBloodSprites[startID].sectnum;
            }

            player->SetLoc(vStartLoc);
            player->SetSector( sectnum );
        }

        //now clean up memory.
        if ( m_pBloodSectors )
        {
            xlDelete [] m_pBloodSectors;
            m_pBloodSectors = NULL;
        }
        if ( m_pBloodWalls )
        {
            xlDelete [] m_pBloodWalls;
            m_pBloodWalls = NULL;
        }
        if ( m_pBloodSprites )
        {
            xlDelete [] m_pBloodSprites;
            m_pBloodSprites = NULL;
        }
    }

    return pCell;
}

#ifndef CELLLOADER_BLOODMAP_H
#define CELLLOADER_BLOODMAP_H

#include "../CommonTypes.h"
#include "CellLoader.h"

class WorldCell;
class World;

class CellLoader_BloodMap : public CellLoader
{
public:
    CellLoader_BloodMap();
    virtual ~CellLoader_BloodMap();

    WorldCell *Load( IDriver3D *pDriver, World *pWorld, uint8_t *pData, uint32_t uLen, const std::string& sFile, int32_t worldX, int32_t worldY ) override;

private:
    struct HeaderPart1
    {
        int32_t startX;
        int32_t startY;
        int32_t startZ;
        int16_t startAngle;
        int16_t sectorNum;
    };

    struct HeaderPart3
    {
        int32_t mapRevisions;
        int16_t numSectors;
        int16_t numWalls;
        int16_t numSprites;
    };

    struct BloodMapData
    {
        int32_t pos[3];
        int16_t angle;
        int16_t startSec;

        int16_t secCnt;
        int16_t wallCnt;
        int16_t spriteCnt;

        int32_t revisions;
    };

    struct BloodSector
    {
        int16_t wallptr;
        int16_t wallnum;
        int32_t ceilingz;
        int32_t floorz;
        int16_t ceilingstat;
        int16_t floorstat;
        int16_t ceilingpicnum;
        int16_t ceilingheinum;
        int8_t  ceilingshade;
        uint8_t  ceilingpal;
        uint8_t  ceilingxpanning;
        uint8_t  ceilingypanning;
        int16_t floorpicnum;
        int16_t floorheinum;
        int8_t  floorshade;
        uint8_t  floorpal;
        uint8_t  floorxpanning;
        uint8_t  floorypanning;
        uint8_t  visibility;
        uint8_t  filler; // Filler "should" == 0
        int16_t lotag;
        int16_t hitag;
        int16_t extra;
    };

    enum TriggerFlags_e
    {
        TRIGGER_DECOUPLED       = (1<<0),
        TRIGGER_ONE_SHOT        = (1<<1),
        TRIGGER_LOCKED          = (1<<2),
        TRIGGER_INTERRUPTABLE   = (1<<3),
        TRIGGER_DUDE_LOCKOUT    = (1<<4)
    };

    enum TriggerOn_e
    {
        TRIGGERON_PUSH      = (1<<2),
        TRIGGERON_VECTOR    = (1<<3),
        TRIGGERON_RESERVED  = (1<<4),
        TRIGGERON_ENTER     = (1<<5),
        TRIGGERON_EXIT      = (1<<6),
        TRIGGERON_WALLPUSH  = (1<<7),
    };

    enum LightFX_Flags_e
    {
        LIGHTFX_COLORED_LIGHTS  = (1<<0),
        LIGHTFX_SHADEALWAYS     = (1<<4),
        LIGHTFX_SHADEFLOOR      = (1<<5),
        LIGHTFX_SHADECEIL       = (1<<6),
        LIGHTFX_SHADEWALLS      = (1<<7)
    };

    enum MotionFX_Flags_e
    {
        MOTIONFX_PANALWAYS = (1<<0),
        MOTIONFX_PANFLOOR  = (1<<1),
        MOTIONFX_PANCEIL   = (1<<2),
        MOTIONFX_DRAG      = (1<<3)
    };

    struct BloodSector_Xtra
    {
        int16_t wallptr;
        int16_t wallnum;
        int32_t ceilingz;
        int32_t floorz;
        int16_t ceilingstat;
        int16_t floorstat;
        int16_t ceilingpicnum;
        int16_t ceilingheinum;
        int8_t  ceilingshade;
        uint8_t  ceilingpal;
        uint8_t  ceilingxpanning;
        uint8_t  ceilingypanning;
        int16_t floorpicnum;
        int16_t floorheinum;
        int8_t  floorshade;
        uint8_t  floorpal;
        uint8_t  floorxpanning;
        uint8_t  floorypanning;
        uint8_t  visibility;
        uint8_t  filler; // Filler "should" == 0
        int16_t lotag;
        int16_t hitag;
        int16_t extra;

        uint8_t startState;
        uint8_t cmd;
        uint8_t rxID;
        uint8_t txID;

        uint8_t triggerFlags;
        
        uint8_t bSendAtOn;
        uint8_t offOn_busyTime;
        uint8_t offOn_wave;
        uint8_t offOn_waitTime;

        uint8_t bSendAtOff;
        uint8_t onOff_busyTime;
        uint8_t onOff_wave;
        uint8_t onOff_waitTime;

        uint8_t triggerOnEvent;
        uint8_t exData;
        uint8_t key;
        uint8_t depth;

        uint8_t bUnderwater;
        uint8_t bCrushing;
        
        int32_t floorStates[2];
        int32_t ceilStates[2];

        uint8_t lightFX_wave;
        int8_t lightFX_amp;
        uint8_t lightFX_freq;
        uint8_t lightFX_phase;
        uint8_t lightFX_flags;
        uint8_t lightFX_ceilPal2;
        uint8_t lightFX_floorPal2;

        uint8_t motionFX_speed;
        uint16_t motionFX_angle;
        uint8_t motionFX_flags;

        uint16_t wind_vel;
        uint16_t wind_ang;
        uint8_t bWindAlways;
    };

    struct BloodSprite
    {
        int32_t x, y, z;
        int16_t cstat;
        int16_t picnum;
        int8_t shade;
        uint8_t pal;
        uint8_t clipdist;
        uint8_t filler;
        uint8_t xrepeat;
        uint8_t yrepeat;
        int8_t xoffset;
        int8_t yoffset;
        int16_t sectnum;
        int16_t statnum;
        int16_t ang;
        int16_t owner;
        int16_t xvel;
        int16_t yvel;
        int16_t zvel;
        int16_t lotag;
        int16_t hitag;
        int16_t extra;
    };

    struct BloodSprite_Xtra
    {
        int32_t x, y, z;
        uint16_t cstat;
        int16_t picnum;
        int8_t shade;
        uint8_t pal;
        uint8_t clipdist;
        uint8_t filler;
        uint8_t xrepeat;
        uint8_t yrepeat;
        int8_t xoffset;
        int8_t yoffset;
        int16_t sectnum;
        int16_t statnum;
        int16_t ang;
        int16_t owner;
        int16_t xvel;
        int16_t yvel;
        int16_t zvel;
        int16_t lotag;
        int16_t hitag;
        int16_t extra;
        uint8_t data[4];
        uint8_t txID;
    };

    struct BloodWall
    {
       int32_t x, y;
       int16_t point2;
       int16_t nextwall;
       int16_t nextsector;
       int16_t cstat;
       int16_t picnum;
       int16_t overpicnum;
       int8_t  shade;
       uint8_t  pal;
       uint8_t  xrepeat;
       uint8_t  yrepeat;
       uint8_t  xpanning;
       uint8_t  ypanning;
       int16_t lotag;
       int16_t hitag;
       int16_t extra;
    };

    struct BloodWall_Xtra
    {
       int32_t x, y;
       int16_t point2;
       int16_t nextwall;
       int16_t nextsector;
       uint16_t cstat;
       int16_t picnum;
       int16_t overpicnum;
       int8_t  shade;
       uint8_t  pal;
       uint8_t  xrepeat;
       uint8_t  yrepeat;
       uint8_t  xpanning;
       uint8_t  ypanning;
       int16_t lotag;
       int16_t hitag;
       int16_t extra;

       int16_t rxID;
       int16_t txID;
    };

    struct MapVersion
    {
        uint8_t Minor;
        uint8_t Major;
    };

    bool m_bIsEncrypted;
    BloodMapData m_bloodData;
    BloodSector_Xtra *m_pBloodSectors;
    BloodWall_Xtra   *m_pBloodWalls;
    BloodSprite_Xtra *m_pBloodSprites;

private:
    void DecryptBuffer(uint8_t *pBuffer, const uint32_t uDataSize, uint8_t uDecryptKey);
    bool ParseHeader(char *pData, int32_t& index);
    int32_t  FindFirstSector(char *pData, int32_t& index);
    bool ExtractSectors(char *pData, int32_t& index);
    bool ExtractWalls(char *pData, int32_t& index);
    bool ExtractSprites(char *pData, int32_t& index);
    TextureHandle AddBloodTile(int32_t picnum, uint32_t uPalIdx, uint32_t& uWidth, uint32_t& uHeight, bool bMip=true);
};

#endif //CELLLOADER_BLOODMAP_H

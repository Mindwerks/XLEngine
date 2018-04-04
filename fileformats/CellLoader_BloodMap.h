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
	~CellLoader_BloodMap();

	WorldCell *Load( IDriver3D *pDriver, World *pWorld, u8 *pData, u32 uLen, const string& sFile, s32 worldX, s32 worldY );

private:
	struct HeaderPart1
	{
		s32 startX;
		s32 startY;
		s32 startZ;
		s16 startAngle;
		s16 sectorNum;
	};

	struct HeaderPart3
	{
		s32 mapRevisions;
		s16 numSectors;
		s16 numWalls;
		s16 numSprites;
	};

	struct BloodMapData
	{
		s32 pos[3];
		s16 angle;
		s16 startSec;

		s16 secCnt;
		s16 wallCnt;
		s16 spriteCnt;

		s32 revisions;
	};

	struct BloodSector
	{
		s16 wallptr;
		s16 wallnum;
		s32 ceilingz;
		s32 floorz;
		s16 ceilingstat;
		s16 floorstat;
		s16 ceilingpicnum;
		s16 ceilingheinum;
		s8  ceilingshade;
		u8  ceilingpal;
		u8  ceilingxpanning;
		u8  ceilingypanning;
		s16 floorpicnum;
		s16 floorheinum;
		s8  floorshade;
		u8  floorpal;
		u8  floorxpanning;
		u8  floorypanning;
		u8  visibility;
		u8  filler; // Filler "should" == 0
		s16 lotag;
		s16 hitag;
		s16 extra;
	};

	enum TriggerFlags_e
	{
		TRIGGER_DECOUPLED		= (1<<0),
		TRIGGER_ONE_SHOT		= (1<<1),
		TRIGGER_LOCKED			= (1<<2),
		TRIGGER_INTERRUPTABLE	= (1<<3),
		TRIGGER_DUDE_LOCKOUT	= (1<<4)
	};

	enum TriggerOn_e
	{
		TRIGGERON_PUSH		= (1<<2),
		TRIGGERON_VECTOR	= (1<<3),
		TRIGGERON_RESERVED	= (1<<4),
		TRIGGERON_ENTER		= (1<<5),
		TRIGGERON_EXIT		= (1<<6),
		TRIGGERON_WALLPUSH	= (1<<7),
	};

	enum LightFX_Flags_e
	{
		LIGHTFX_COLORED_LIGHTS	= (1<<0),
		LIGHTFX_SHADEALWAYS		= (1<<4),
		LIGHTFX_SHADEFLOOR		= (1<<5),
		LIGHTFX_SHADECEIL		= (1<<6),
		LIGHTFX_SHADEWALLS		= (1<<7)
	};

	enum MotionFX_Flags_e
	{
		MOTIONFX_PANALWAYS = (1<<0),
		MOTIONFX_PANFLOOR  = (1<<1),
		MOTIONFX_PANCEIL   = (1<<2),
		MOTIONFX_DRAG	   = (1<<3)
	};

	struct BloodSector_Xtra
	{
		s16 wallptr;
		s16 wallnum;
		s32 ceilingz;
		s32 floorz;
		s16 ceilingstat;
		s16 floorstat;
		s16 ceilingpicnum;
		s16 ceilingheinum;
		s8  ceilingshade;
		u8  ceilingpal;
		u8  ceilingxpanning;
		u8  ceilingypanning;
		s16 floorpicnum;
		s16 floorheinum;
		s8  floorshade;
		u8  floorpal;
		u8  floorxpanning;
		u8  floorypanning;
		u8  visibility;
		u8  filler; // Filler "should" == 0
		s16 lotag;
		s16 hitag;
		s16 extra;

		u8 startState;
		u8 cmd;
		u8 rxID;
		u8 txID;

		u8 triggerFlags;
		
		u8 bSendAtOn;
		u8 offOn_busyTime;
		u8 offOn_wave;
		u8 offOn_waitTime;

		u8 bSendAtOff;
		u8 onOff_busyTime;
		u8 onOff_wave;
		u8 onOff_waitTime;

		u8 triggerOnEvent;
		u8 exData;
		u8 key;
		u8 depth;

		u8 bUnderwater;
		u8 bCrushing;
		
		s32 floorStates[2];
		s32 ceilStates[2];

		u8 lightFX_wave;
		s8 lightFX_amp;
		u8 lightFX_freq;
		u8 lightFX_phase;
		u8 lightFX_flags;
		u8 lightFX_ceilPal2;
		u8 lightFX_floorPal2;

		u8 motionFX_speed;
		u16 motionFX_angle;
		u8 motionFX_flags;

		u16 wind_vel;
		u16 wind_ang;
		u8 bWindAlways;
	};

	struct BloodSprite
	{
		s32 x, y, z;
		s16 cstat;
		s16 picnum;
		s8 shade;
		u8 pal;
		u8 clipdist;
		u8 filler;
		u8 xrepeat;
		u8 yrepeat;
		s8 xoffset;
		s8 yoffset;
		s16 sectnum;
		s16 statnum;
		s16 ang;
		s16 owner;
		s16 xvel;
		s16 yvel;
		s16 zvel;
		s16 lotag;
		s16 hitag;
		s16 extra;
	};

	struct BloodSprite_Xtra
	{
		s32 x, y, z;
		u16 cstat;
		s16 picnum;
		s8 shade;
		u8 pal;
		u8 clipdist;
		u8 filler;
		u8 xrepeat;
		u8 yrepeat;
		s8 xoffset;
		s8 yoffset;
		s16 sectnum;
		s16 statnum;
		s16 ang;
		s16 owner;
		s16 xvel;
		s16 yvel;
		s16 zvel;
		s16 lotag;
		s16 hitag;
		s16 extra;
		u8 data[4];
		u8 txID;
	};

	struct BloodWall
	{
	   s32 x, y;
	   s16 point2;
	   s16 nextwall;
	   s16 nextsector;
	   s16 cstat;
	   s16 picnum;
	   s16 overpicnum;
	   s8  shade;
	   u8  pal;
	   u8  xrepeat;
	   u8  yrepeat;
	   u8  xpanning;
	   u8  ypanning;
	   s16 lotag;
	   s16 hitag;
	   s16 extra;
	};

	struct BloodWall_Xtra
	{
	   s32 x, y;
	   s16 point2;
	   s16 nextwall;
	   s16 nextsector;
	   u16 cstat;
	   s16 picnum;
	   s16 overpicnum;
	   s8  shade;
	   u8  pal;
	   u8  xrepeat;
	   u8  yrepeat;
	   u8  xpanning;
	   u8  ypanning;
	   s16 lotag;
	   s16 hitag;
	   s16 extra;

	   s16 rxID;
	   s16 txID;
	};

	struct MapVersion
    {
        u8 Minor;
		u8 Major;
    };

	bool m_bIsEncrypted;
	BloodMapData m_bloodData;
	BloodSector_Xtra *m_pBloodSectors;
	BloodWall_Xtra   *m_pBloodWalls;
	BloodSprite_Xtra *m_pBloodSprites;

private:
	void DecryptBuffer(u8 *pBuffer, const u32 uDataSize, u8 uDecryptKey);
	bool ParseHeader(char *pData, s32& index);
	s32  FindFirstSector(char *pData, s32& index);
	bool ExtractSectors(char *pData, s32& index);
	bool ExtractWalls(char *pData, s32& index);
	bool ExtractSprites(char *pData, s32& index);
	TextureHandle AddBloodTile(s32 picnum, u32 uPalIdx, u32& uWidth, u32& uHeight, bool bMip=true);
};

#endif //CELLLOADER_BLOODMAP_H

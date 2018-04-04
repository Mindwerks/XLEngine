#ifndef ENGINESETTINGS_H
#define ENGINESETTINGS_H

#include "CommonTypes.h"
#include "math/Vector3.h"
#include <string>

using namespace std;

class EngineSettings
{
    public:
		enum
		{
			FULLSCREEN=(1<<0),
			EMULATE_320x200=(1<<1),
			CPU_LIGHT=(1<<2),
			VSYNC=(1<<3),
			FAKE_BUMP=(1<<4),
			SMOOTH_SKY=(1<<5),
			BLOOM=(1<<6),
		};

		enum
		{
			RENDERER_OPENGL = 0,
			RENDERER_SOFT32,
			RENDERER_SOFT8,
			RENDERER_COUNT
		};

		static void Init();
		static bool Load( const char *pszSettingsFile );
		static void SetStartMap( const char *pszMapName );
		static void SetStartPos( const Vector3 *pos, s32 nSector );
		static void SetMultiplayerData( s32 nServer_PlayerCnt, s32 nPort, const char *pszJoinIP );

		static const char *GetGameDataDir() { return m_szGameDataDir; }
		//desired screen width and height (may be different then final screen width and height, so use the Driver3D values
		//while in-game.
		static s32 GetScreenWidth()      { return m_nScreenWidth; }
		static s32 GetScreenHeight()     { return m_nScreenHeight; }
		//the start map.
		static const char *GetStartMap() { return m_szMapName; }
		static void GetStartMap_StrOut(string& sMapName) { sMapName = m_szMapName; }
		//MP data.
		static XL_BOOL IsServer() { return m_nServerPlayerCnt > 0 ? XL_TRUE : XL_FALSE; }
		static XL_BOOL IsClient_MP() { return m_szServerIP[0]!=0 ? XL_TRUE : XL_FALSE; }
		static s32 GetMaxPlayerCount() { return m_nServerPlayerCnt; }
		static s32 GetPort() { return m_nPort; }
		static const char *GetServerIP() { return m_szServerIP; }
		static bool GetStartPos(Vector3& pos, s32& sector) 
		{ 
			if ( m_bOverridePos )
			{
				pos = m_vStartPos; 
				sector = m_nStartSec;
			}
			return m_bOverridePos; 
		}

		//display settings
		static void SetDisplaySettings(float brightness=1.0f, float contrast=1.0f, float gamma=1.0f);
		static void GetDisplaySettings(float& brightness, float& contrast, float& gamma);

		static bool IsFeatureEnabled(u32 uFeature);

		static void SetGameDir(const char *pszGame);
		static const char *GetGameDir() { return m_szGameDir; }

		static s32 GetRenderer() { return m_nRenderer; }
		static void SetRenderer(s32 renderer) { m_nRenderer = renderer; }
    private:
		//Game Data Root.
		static char m_szGameDataDir[260];
		static char m_szGameDir[260];
		static char m_szMapName[260];
		static s32 m_nScreenWidth;
		static s32 m_nScreenHeight;
		static s32 m_nRenderer;

		//MP Data
		static s32 m_nServerPlayerCnt;
		static s32 m_nPort;
		static char m_szServerIP[32];
		static bool m_bOverridePos;
		static s32 m_nStartSec;
		static Vector3 m_vStartPos;

		//Display settings.
		static float m_fBrightness;
		static float m_fContrast;
		static float m_fGamma;

		static u32 m_uFlags;
};

#endif // ENGINESETTINGS_H

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
		static void SetStartPos( const Vector3 *pos, int32_t nSector );
		static void SetMultiplayerData( int32_t nServer_PlayerCnt, int32_t nPort, const char *pszJoinIP );

		static const char *GetGameDataDir() { return m_szGameDataDir; }
		//desired screen width and height (may be different then final screen width and height, so use the Driver3D values
		//while in-game.
		static int32_t GetScreenWidth()      { return m_nScreenWidth; }
		static int32_t GetScreenHeight()     { return m_nScreenHeight; }
		//the start map.
		static const char *GetStartMap() { return m_szMapName; }
		static void GetStartMap_StrOut(string& sMapName) { sMapName = m_szMapName; }
		//MP data.
		static XL_BOOL IsServer() { return m_nServerPlayerCnt > 0 ? true : false; }
		static XL_BOOL IsClient_MP() { return m_szServerIP[0]!=0 ? true : false; }
		static int32_t GetMaxPlayerCount() { return m_nServerPlayerCnt; }
		static int32_t GetPort() { return m_nPort; }
		static const char *GetServerIP() { return m_szServerIP; }
		static bool GetStartPos(Vector3& pos, int32_t& sector)
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

		static bool IsFeatureEnabled(uint32_t uFeature);

		static void SetGameDir(const char *pszGame);
		static const char *GetGameDir() { return m_szGameDir; }

		static int32_t GetRenderer() { return m_nRenderer; }
		static void SetRenderer(int32_t renderer) { m_nRenderer = renderer; }
    private:
		//Game Data Root.
		static char m_szGameDataDir[260];
		static char m_szGameDir[260];
		static char m_szMapName[260];
		static int32_t m_nScreenWidth;
		static int32_t m_nScreenHeight;
		static int32_t m_nRenderer;

		//MP Data
		static int32_t m_nServerPlayerCnt;
		static int32_t m_nPort;
		static char m_szServerIP[32];
		static bool m_bOverridePos;
		static int32_t m_nStartSec;
		static Vector3 m_vStartPos;

		//Display settings.
		static float m_fBrightness;
		static float m_fContrast;
		static float m_fGamma;

		static uint32_t m_uFlags;
};

#endif // ENGINESETTINGS_H

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

        static EngineSettings &get() noexcept { return s_Settings; }

        bool Load( const char *pszSettingsFile );
        void SetStartMap( const char *pszMapName );
        void SetStartPos( const Vector3 *pos, int32_t nSector );
        void SetMultiplayerData( int32_t nServer_PlayerCnt, int32_t nPort, const char *pszJoinIP );

        const char *GetGameDataDir() { return m_szGameDataDir.c_str(); }
        //desired screen width and height (may be different then final screen width and height, so use the Driver3D values
        //while in-game.
        int32_t GetScreenWidth()      { return m_nScreenWidth; }
        int32_t GetScreenHeight()     { return m_nScreenHeight; }
        //the start map.
        const char *GetStartMap() { return m_szMapName.c_str(); }
        void GetStartMap_StrOut(string& sMapName) { sMapName = m_szMapName; }
        //MP data.
        XL_BOOL IsServer() { return m_nServerPlayerCnt > 0 ? true : false; }
        XL_BOOL IsClient_MP() { return m_szServerIP[0]!=0 ? true : false; }
        int32_t GetMaxPlayerCount() { return m_nServerPlayerCnt; }
        int32_t GetPort() { return m_nPort; }
        const char *GetServerIP() { return m_szServerIP; }
        bool GetStartPos(Vector3& pos, int32_t& sector)
        { 
            if ( m_bOverridePos )
            {
                pos = m_vStartPos; 
                sector = m_nStartSec;
            }
            return m_bOverridePos; 
        }

        //display settings
        void SetDisplaySettings(float brightness=1.0f, float contrast=1.0f, float gamma=1.0f);
        void GetDisplaySettings(float& brightness, float& contrast, float& gamma);

        bool IsFeatureEnabled(uint32_t uFeature);

        void SetGameDir(const char *pszGame);
        const char *GetGameDir() { return m_szGameDir.c_str(); }

        int32_t GetRenderer() { return m_nRenderer; }
        void SetRenderer(int32_t renderer) { m_nRenderer = renderer; }

        //callback methods
        static const char *GetStartMapCB() { return s_Settings.GetStartMap(); }
        static XL_BOOL IsServerCB() { return s_Settings.IsServer(); }
        static void GetStartMap_StrOutCB(string& sMapName)
        { s_Settings.GetStartMap_StrOut(sMapName); }

    private:
        static EngineSettings s_Settings;

        EngineSettings();
        EngineSettings(EngineSettings&&) = delete;
        EngineSettings(const EngineSettings&) = delete;
        EngineSettings& operator=(EngineSettings&&) = delete;
        EngineSettings& operator=(const EngineSettings&) = delete;

        //Game Data Root.
        std::string m_szGameDataDir;
        std::string m_szGameDir;
        std::string m_szMapName;
        int32_t m_nScreenWidth;
        int32_t m_nScreenHeight;
        int32_t m_nRenderer;

        //MP Data
        int32_t m_nServerPlayerCnt;
        int32_t m_nPort;
        char m_szServerIP[32];
        bool m_bOverridePos;
        int32_t m_nStartSec;
        Vector3 m_vStartPos;

        //Display settings.
        float m_fBrightness;
        float m_fContrast;
        float m_fGamma;

        uint32_t m_uFlags;
};

#endif // ENGINESETTINGS_H

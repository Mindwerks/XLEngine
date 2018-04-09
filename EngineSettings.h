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

        static EngineSettings &get() noexcept { return sSettings; }

        bool Load(const char *settings_file);
        void SetStartMap(const char *map_name);
        void SetStartPos(const Vector3 *pos, int32_t sector);
        void SetMultiplayerData(int32_t server_PlayerCnt, int32_t port, const char *joinIP);

        const char *GetGameDataDir() { return mGameDataDir.c_str(); }
        //desired screen width and height (may be different then final screen width and height, so use the Driver3D values
        //while in-game.
        int32_t GetScreenWidth()  { return mScreenWidth; }
        int32_t GetScreenHeight() { return mScreenHeight; }
        //the start map.
        const char *GetStartMap() { return mMapName.c_str(); }
        void GetStartMap_StrOut(std::string& map_name) { map_name = mMapName; }
        //MP data.
        XL_BOOL IsServer() { return mServerPlayerCnt > 0 ? true : false; }
        XL_BOOL IsClient_MP() { return mServerIP[0]!=0 ? true : false; }
        int32_t GetMaxPlayerCount() { return mServerPlayerCnt; }
        int32_t GetPort() { return mPort; }
        const char *GetServerIP() { return mServerIP.c_str(); }
        bool GetStartPos(Vector3& pos, int32_t& sector)
        { 
            if(mOverridePos)
            {
                pos = mStartPos;
                sector = mStartSec;
            }
            return mOverridePos;
        }

        //display settings
        void SetDisplaySettings(float brightness=1.0f, float contrast=1.0f, float gamma=1.0f);
        void GetDisplaySettings(float& brightness, float& contrast, float& gamma);

        bool IsFeatureEnabled(uint32_t feature);

        void SetGameDir(const char *game);
        const char *GetGameDir() { return mGameDir.c_str(); }

        int32_t GetRenderer() { return mRenderer; }
        void SetRenderer(int32_t renderer) { mRenderer = renderer; }

        //callback methods
        static const char *GetStartMapCB() { return sSettings.GetStartMap(); }
        static XL_BOOL IsServerCB() { return sSettings.IsServer(); }
        static void GetStartMap_StrOutCB(string& map_name)
        { sSettings.GetStartMap_StrOut(map_name); }

    private:
        static EngineSettings sSettings;

        EngineSettings();
        EngineSettings(EngineSettings&&) = delete;
        EngineSettings(const EngineSettings&) = delete;
        EngineSettings& operator=(EngineSettings&&) = delete;
        EngineSettings& operator=(const EngineSettings&) = delete;

        //Game Data Root.
        std::string mGameDataDir;
        std::string mGameDir;
        std::string mMapName;
        int32_t mScreenWidth = 1024;
        int32_t mScreenHeight = 768;
        int32_t mRenderer = RENDERER_SOFT8;

        //MP Data
        int32_t mServerPlayerCnt = 0;
        int32_t mPort = 0;
        std::string mServerIP;
        bool mOverridePos = false;
        int32_t mStartSec = -1;
        Vector3 mStartPos{0.0f, 0.0f, 0.0f};

        //Display settings.
        float mBrightness = 1.0f;
        float mContrast = 1.0f;
        float mGamma = 1.0f;

        uint32_t mFlags = 0;
};

#endif // ENGINESETTINGS_H

#ifndef ENGINE_H
#define ENGINE_H

#include "CommonTypes.h"

class IDriver3D;
class Camera;
class XLFont;
class World;
class Vector4;
struct XLEngine_Plugin_API;

class Engine
{
    public:
        Engine();
        virtual ~Engine();

        bool Loop(float fDeltaTime, bool bFullspeed);

        void ChangeWindowSize(int32_t w, int32_t h);

        bool Init(void **winParam, int32_t paramCnt, int32_t w, int32_t h);
        void InitGame(const char *pszGameLib);

        XLEngine_Plugin_API *GetEngineAPI() { return m_pPluginAPI; }
        World *GetWorld() { return m_pWorld; }
        IDriver3D *GetDriver() { return m_pDriver3D; }

        XLFont *GetSystemFont(int32_t size);
        static void PostExitMessage();

        void AddDisplayMessage(const char *pszMsg, Vector4 *color=0, float fShowTime=0.0f);

        void WorldUpdate(int32_t newWorldX, int32_t newWorldY);

        float GetCurrentBrightness();
        float GetCurrentSpeed();
    protected:
        IDriver3D *m_pDriver3D;
        Camera    *m_pCamera;
        World     *m_pWorld;
        float m_fTotalTime;
        float m_FPS;
        int32_t m_nWidth;
        int32_t m_nHeight;
        XLFont *m_pSystemFont16;
        XLFont *m_pSystemFont24;
        XLFont *m_pSystemFont32;

        //Plugin/Game API.
        XLEngine_Plugin_API *m_pPluginAPI;

        //Time to exit?
        static bool m_bContinueLoop;

        static void Engine_SetCameraData(float *pos, float *dir, float fSkew, float fSpeed, uint32_t uSector);
        static XL_BOOL Engine_AllowPlayerControls(void);
        static void Object_GetCameraVector(uint32_t uObjID, float& x, float& y, float& z);
    private:
        void Destroy();
        void SetupPluginAPI();
        void DisplayMessages(float fDeltaTime);
};

#endif // ENGINE_H

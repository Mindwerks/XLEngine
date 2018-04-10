#ifndef UISYSTEM_H
#define UISYSTEM_H

#include "../CommonTypes.h"
#include "../scriptsystem/ScriptSystem.h"
#include "../math/Vector2.h"

#include <string>
#include <vector>
#include <map>

class asIScriptArray;
class IDriver3D;
class Engine;
class LFD_Anim;

enum UI_State_e
{
    UI_State_Normal = 0,
    UI_State_Pressed,
    UI_State_MouseOver,
    UI_State_Count
};

enum UIWinFlags_e
{
    UIWinFlag_None = 0,
    UIWinFlag_NonInteractive = (1<<0),  //just for looks.
    UIWinFlag_ParentRelative = (1<<1),  //
};

class UI_Window;

struct UI_RenderFrame
{
    uint32_t ID;            //ID
    bool bInUse;    //is this frame currently allocated?

    TextureHandle hFrame;       //the image handle.
    int16_t width;      //the image width.
    int16_t height;     //the image height.
    float fRelWidth;    //Relative width and height.
    float fRelHeight;   //used to determine uv's when image is fit into a pow of 2 texture.
};

class UI_Screen
{
public:
    UI_Screen();
    ~UI_Screen();
public:
    std::string m_sName;
    SHANDLE m_hOnEnter;
    SHANDLE m_hOnExit;
    SHANDLE m_hOnRender;
    SHANDLE m_hOnPostRender;
    SHANDLE m_hOnUpdate;
    SHANDLE m_hOnKey;
    uint32_t m_uFlags;
    uint32_t m_uState;
    float m_fVirt_Scr_W;
    float m_fVirt_Scr_H;

    UI_Screen *m_parent;
    UI_Screen *m_child;
    //child Windows - things like buttons.
    UI_Window *m_childWindow;
};

class UI_Window
{
public:
    UI_Window();
    ~UI_Window();
    
    void Draw(int x, int y);
    void Update(bool bMouseOver, int nMouseX, int nMouseY, int x, int y);
public:
    uint32_t m_uType;
    int32_t m_x, m_y;
    int32_t m_w, m_h;
    int32_t m_prevX;
    int32_t m_prevY;
    bool m_bMouseHeld;
    bool m_bMoving;
    bool m_bEnabled;
    SHANDLE m_hOnRender;
    SHANDLE m_hOnUpdate;
    SHANDLE m_hOnKey;
    SHANDLE m_hOnRelease;

    static bool s_bWindowMoving;

    uint32_t m_uFlags;
    uint32_t m_uState;

    std::string m_name;
    std::string m_text;

    UI_Screen *m_parentScreen;
    UI_Window *m_parent;
    UI_Window *m_child;
    UI_Window *m_sibling;
private:
    void RenderWindow(int x, int y, int w, int h, const std::string& name);
    void RenderButton(int x, int y, int w, const std::string& text, int state);
};

class UI_System
{
friend UI_Window;
public:
    static bool Init(IDriver3D *pDriver, Engine *pEngine);
    static void Destroy();
    static void StartScript(const char *pszFile);
    static void Update();
    static void Render();

    static UI_Screen *FindScreen(const std::string& sName);

    static void UpdateChildWindows(UI_Window *pWin, int x, int y, int nMouseX, int nMouseY);
    static void RenderChildWindows(UI_Window *pWin, int x, int y);

    enum
    {
        UIFLAG_NONE = 0,
        UIFLAG_OVERLAY = (1<<0)
    };
private:
    
    enum
    {
        UI_Align_Left   = 0,
        UI_Align_Right  = 1,
        UI_Align_Center = 2,

        UI_Align_Bottom = 0,
        UI_Align_Top    = 1,
    };

    static IDriver3D *m_pDriver;
    static Engine *m_pEngine;

    static bool m_bScriptsLoaded;
    static bool m_bScriptExeSucceeded;
    //static float m_fVirt_Scr_W;
    //static float m_fVirt_Scr_H;
    static Vector2 m_uvTop;
    static Vector2 m_uvBot;
    static bool m_bFlipX;
    static bool m_bFlipY;
    static UI_Screen *m_Top;
    static UI_Screen *m_Leaf;
    static UI_Screen *m_Context;
    static UI_Screen *m_PendingScreenChange;
    static UI_Screen *m_PushScreen;
    static UI_Screen *m_PopScreen;

    static std::map<std::string, UI_Screen *> m_ScreenMap;
    static std::vector<UI_Screen *> m_ScreenList;
    static std::vector<UI_Window *> m_WindowList;

    static UI_RenderFrame *m_pRenderFramePool;
    static LFD_Anim *m_pLFD_Anim_List[];

    static uint32_t m_auImage_TexType[];
    static uint32_t m_auImage_ArchiveType[];

    //Add a new screen, if it hasn't already been added.
    static UI_Screen *AddScreen(const std::string& sName);
    static UI_Window *AddWindow(const std::string& sName, const std::string& sText, uint32_t uType, int32_t x, int32_t y, int32_t w, int32_t h, uint32_t flags, UI_Window *parent=nullptr);

    //Script Functions, these are call by the UI Scripts.
    //UI_StartScreen is the first UI screen to start up, the program entry point.
    //There will probably be other entry points due to parameters passed in or other data.
    static void UI_StartScreen(std::string& sUI_Start);
    static void UI_CreateWindow(std::string& sName, std::string& sText, int type, int x, int y, int w, int h, int flags);
    static void UI_PushWindow(std::string& sName, std::string& sText, int type, int x, int y, int w, int h, int flags);
    static void UI_PopWindow();
    static void UI_EnableWindow(std::string& sName, int enable);
    static void UI_CreateWindow_FromLFDFrame(std::string& sName, int LFDAnim_ID, int frame, int x0, int y0);
    static int  UI_AddImage(std::string& sImage, int cutoutMinIdx, int cutoutMaxIdx);
    static int  UI_AddGameImage(uint32_t uImageType, std::string &sArchive, std::string &sImage);
    static void UI_FreeImage(int hImageHandle);
    static void UI_EnableImageFilter(int enable);
    static void UI_EnableImageBlending(int enable);
    static void UI_SetImageRenderProp(int flipX, int flipY);
    static void UI_RenderImage(int hImage, int x, int y, float intensity, int alignHoriz, int alignVert);
    static void UI_GetImageSize(int hImage, int& w, int& h);
    static void UI_RenderImageRect(int hImage, int x, int y, int w, int h, float intensity, int alignHoriz, int alignVert);
    static void UI_SetImageUV_Range(float u0, float v0, float u1, float v1);
    static void UI_SetImageUV_RangeI(int hImage, int u0, int v0, int w, int h);
    static void UI_RenderPolygon(int npt, asIScriptArray *x, asIScriptArray *y, float r, float g, float b, float a, int alignHoriz, int alignVert);
    static void UI_RenderRect(int x, int y, int w, int h, float r, float g, float b, float a, int alignHoriz, int alignVert);
    static void UI_RenderString(const std::string& sString, int x, int y, int size, float r, float g, float b, float a);
    static void UI_PrintMousePos(int x, int y, int size);
    static void UI_GetMousePos(int& x, int& y);
    static void UI_SetVirtualScreenSize(int w, int h);
    static void UI_GetVirtualScreenSize(int& w, int& h);
    static void UI_GetScreenSize(int& w, int& h);
    static void UI_SetPalette(int pal, int colMap);
    static int UI_GetVirtualKey(int key);
    static float UI_GetCurrentBrightness();
    static float UI_GetSpeed();
    //LFD_Anim
    static int UI_CreateLFD_Anim(std::string& sArchive, std::string& sAnim, std::string& sPal);
    static void UI_DestroyLFD_Anim(int ID);
    static void UI_RenderLFD_Anim(int ID, int frame, int x, int y);
    //use different flags for overlay versus fullscreen.
    static void UI_PushScreen(const std::string& uiName, int flags, int backgrndFX);
    //pops the current screen off the stack, returning control to the previous.
    static void UI_PopScreen();

    //RenderFrame Memory Management.
    static void AllocRenderFramePool();
    static void FreeRenderFramePool();
    static UI_RenderFrame *AllocRenderFrame();
    static void FreeRenderFrame(uint32_t uFrameID);
    static UI_RenderFrame *GetRenderFrame(uint32_t uFrameID);

    //
    static void KeyDownCallback(int32_t key);
};

#endif //UISYSTEM_H
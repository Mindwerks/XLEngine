// main.cpp : Defines the entry point for the application.
//
#include "main.h"
#include "../Engine.h"
#include "../OS/Input.h"
#include "../OS/Clock.h"
#include "../networking/NetworkMgr.h"
#include "../EngineSettings.h"

#ifndef WM_XBUTTONDOWN
    #define WM_XBUTTONDOWN   0x020B
    #define WM_XBUTTONUP     0x020C
    #define WM_XBUTTONDBLCLK 0x020D
    #define MK_XBUTTON1      0x0020
    #define MK_XBUTTON2      0x0040
    #define VK_XBUTTON1      0x05
    #define VK_XBUTTON2      0x06
#endif

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
TCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int, int, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

Engine *m_pEngine;
HWND _hwnd;

bool g_bFullScreen=false;
bool g_bHasFocus=true;

#define _DEFAULT_SERVER_PORT 5030
#define _DEFAULT_CLIENT_PORT 5029

void ParseCommandLine(size_t len, const char *pszCmdLine);
bool GetOption(const char *pszOption, char *pszValue);
int32_t  GetOptionint32_t(const char *pszOption);

#ifdef _CRTDBG_MAP_ALLOC
    _CRT_ALLOC_HOOK _prevAllocHook;

    int _allocCB(int allocType, void *userData, size_t size, int blockType, long requestNumber, const unsigned char *filename, int lineNumber)
    {
    #if 0   //this allows me to break on allocations based on something like size
        if ( size == 64000 )
        {
            static int _x=0;
            _x++;
        }
    #endif
        return _prevAllocHook(allocType, userData, size, blockType, requestNumber, filename, lineNumber);
    }
#endif

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);

    // TODO: Place code here.
    MSG msg;
    HACCEL hAccelTable;

    #ifdef _CRTDBG_MAP_ALLOC
        _CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_CHECK_ALWAYS_DF );
        _prevAllocHook = _CrtSetAllocHook( _allocCB );
    #endif

    // Initialize global strings
    LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadString(hInstance, IDC_XLENGINE, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    //figure out which game we're going to load.
    //this is done before the engine is initialized so we know which config file to load.
    size_t l = strlen(lpCmdLine);
    char szGame[32];
    ParseCommandLine( l, lpCmdLine );
    if ( !GetOption("g", szGame) )
    {
        return FALSE;
    }

    int32_t nServer_PlayerCnt = GetOptionint32_t("server");
    int32_t nPort = GetOptionint32_t("port");

    char szMap[32];
    if ( !GetOption("map", szMap) )
    {
        nServer_PlayerCnt = 0;  //there's no point in setting up a server if a MAP isn't specified.
        szMap[0] = 0;
    }

    if ( nPort == -1 ) 
    {
        nPort = nServer_PlayerCnt>0 ? _DEFAULT_SERVER_PORT : _DEFAULT_CLIENT_PORT;
    }

    char szJoinIP[32];
    if ( !GetOption("join", szJoinIP) )
    {
        szJoinIP[0] = 0;
    }

    char szServerIP[32];
    if ( !GetOption("serverIP", szServerIP) )
    {
        szServerIP[0] = 0;
    }

    char szPlayerName[32];
    if ( !GetOption("player", szPlayerName) )
    {
        strcpy(szPlayerName, "Default_Player");
    }
    NetworkMgr::SetLocalPlayerName(szPlayerName);
    
    //Engine settings.
    EngineSettings &settings = EngineSettings::get();

    //We have to load the engine settings before created the window and setting up
    //so that we can pick the correct resolution, fullscreen, etc.
    char szSettingsFile[260];
    sprintf(szSettingsFile, "%s/%s.conf", szGame, szGame);
    settings.SetGameDir(szGame);
    settings.Load( szSettingsFile );
    settings.SetStartMap( szMap );
    settings.SetMultiplayerData( nServer_PlayerCnt, nPort, nServer_PlayerCnt > 0 ? szServerIP : szJoinIP );

    // Perform application initialization:
    if ( !InitInstance(hInstance, nCmdShow, settings.GetScreenWidth(), settings.GetScreenHeight()) )
    {
        return FALSE;
    }

    //Setup Engine systems.
    m_pEngine = xlNew Engine();
    void *win_param[] = { (void *)_hwnd };
    bool bInit = m_pEngine->Init(win_param, 1, settings.GetScreenWidth(), settings.GetScreenHeight());
    if ( bInit == false )
    {
        return FALSE;
    }
    //Init the current game.
    m_pEngine->InitGame( szGame );
    
    hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_XLENGINE));

    #ifdef _CRTDBG_MAP_ALLOC
        _CrtCheckMemory();
    #endif

    const float fMaxDelta  = 0.1f;
    float fDeltaTime = 0.0f;
    bool bPrevMouseLock = false;
    // Main message loop:
    while (true)
    {
        if ( PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) )
        {
            if ( msg.message == WM_QUIT )
                break;

            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            bool bContinue;
            Clock::StartTimer();
            {
                POINT point, center;
                if ( Input::LockMouse() && (g_bHasFocus || g_bFullScreen) )
                {
                    //Get the current mouse cursor position.
                    GetCursorPos(&point);
                    //compute the center point of the window.
                    center.x = settings.GetScreenWidth()>>1;
                    center.y = settings.GetScreenHeight()>>1;
                    ClientToScreen(_hwnd, &center);
                    //compute the delta from the window center to the current mouse cursor position.
                    int dx = point.x - center.x;
                    int dy = point.y - center.y;
                    //Set the mouse cursor position to the window center.
                    SetCursorPos(center.x, center.y);
                    //Pass the mouse cursor delta to the input system.
                    //This is used for mouse look.
                    Input::AddMouseDelta(dx, dy);
                    //turn off mouse cursor drawing.
                    //if ( bPrevMouseLock == false )
                    //{
                        ShowCursor(FALSE);
                    //}
                    bPrevMouseLock = true;
                }
                else if ( g_bHasFocus || g_bFullScreen )
                {
                    //turn on mouse cursor drawing.
                    //if the mouse is not locked, the engine gets mouse cursor position updates using the windows messaging system
                    //which is handled by WndProc() below.
                    GetCursorPos(&point);
                    ClientToScreen(_hwnd, &point);
                    ShowCursor(FALSE);
                    /*if ( point.x < 0 || point.x > settings.GetScreenWidth() || point.y < 0 || point.y > settings.GetScreenHeight() )
                    {
                        ShowCursor(TRUE);
                    }
                    else
                    {
                        ShowCursor(FALSE);
                    }*/
                    bPrevMouseLock = false;
                }
                else
                {
                    bPrevMouseLock = false;
                    ShowCursor(TRUE);
                }

                bContinue = m_pEngine->Loop(fDeltaTime, g_bHasFocus||g_bFullScreen);
            }
            fDeltaTime = Clock::GetDeltaTime(fMaxDelta);

            if ( bContinue == false )
            {
                PostQuitMessage(0);
            }

            #ifdef _CRTDBG_MAP_ALLOC
                _CrtCheckMemory();
            #endif
        }
    };

    //Cleanup engine.
    if ( m_pEngine )
    {
        xlDelete m_pEngine;
        m_pEngine = NULL;
    }

    if ( settings.IsServer() )
    {
       FreeConsole();
    }

    return (int)msg.wParam;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEX wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_XLENGINE));
    wcex.hCursor        = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground  = 0;
    wcex.lpszMenuName   = 0;
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow, int w, int h)
{
    HWND hWnd;

    hInst = hInstance; // Store instance handle in our global variable
    EngineSettings &settings = EngineSettings::get();
    g_bFullScreen = false;
    if ( settings.IsFeatureEnabled(EngineSettings::FULLSCREEN) && !settings.IsServer() )
    {
        g_bFullScreen = true;

        DEVMODE dmScreenSettings;                               // Device Mode
        memset(&dmScreenSettings,0,sizeof(dmScreenSettings));   // Makes Sure Memory's Cleared
        dmScreenSettings.dmSize=sizeof(dmScreenSettings);       // Size Of The Devmode Structure
        dmScreenSettings.dmPelsWidth    = w;                    // Selected Screen Width
        dmScreenSettings.dmPelsHeight   = h;                    // Selected Screen Height
        dmScreenSettings.dmBitsPerPel   = 32;                   // Selected Bits Per Pixel
        dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT;

        // Try To Set Selected Mode And Get Results.  NOTE: CDS_FULLSCREEN Gets Rid Of Start Bar.
        if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL)
        {
            // If The Mode Fails, Offer Two Options.  Quit Or Use Windowed Mode.
            if ( MessageBox(NULL,"The Requested Fullscreen Mode Is Not Supported By\nYour Video Card. Use Windowed Mode Instead?", "XL Engine", MB_YESNO|MB_ICONEXCLAMATION) == IDYES )
            {
                g_bFullScreen = false;      // Windowed Mode Selected.  Fullscreen = FALSE
            }
            else
            {
                // Pop Up A Message Box Letting User Know The Program Is Closing.
                MessageBox(NULL,"Program Will Now Close.", "ERROR", MB_OK|MB_ICONSTOP);
                return FALSE;                                   // Return FALSE
            }
        }
    }

    //make the base window tiny...
    if ( settings.IsServer() )
    {
        w = 256; h = 64;
    }

    if ( !g_bFullScreen )
    {
        //The window style.
        DWORD dwStyle = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_BORDER | WS_MINIMIZEBOX;

        //compute the final window size to get a specific client window size.
        RECT desiredSize;
        desiredSize.left   = 0;
        desiredSize.top    = 0;
        desiredSize.right  = w;
        desiredSize.bottom = h;
        AdjustWindowRect(&desiredSize, dwStyle, FALSE);

        hWnd = CreateWindow(szWindowClass, szTitle, dwStyle, CW_USEDEFAULT, 0, desiredSize.right-desiredSize.left, desiredSize.bottom-desiredSize.top, NULL, NULL, hInstance, NULL);
    }
    else
    {
        DWORD dwExStyle=WS_EX_APPWINDOW;                            // Window Extended Style
        DWORD dwStyle=WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN; // Windows Style
        ShowCursor(FALSE);

        hWnd = CreateWindowEx(dwExStyle, szWindowClass, szTitle, dwStyle, 0, 0, w, h, NULL, NULL, hInstance, NULL);
    }

    if (!hWnd)
    {
        return FALSE;
    }

    _hwnd = hWnd;

    //Open up a console window so that standard printf works.
    //This allows for seamless output without worrying about Windows
    //controls.
    if ( settings.IsServer() )
    {
        AllocConsole();

        freopen("CONOUT$","wb",stdout);  // reopen stout handle as console window output
        freopen("CONOUT$","wb",stderr);  // reopen stderr handle as console window output
    }

    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);

    return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int wmId, wmEvent;
    PAINTSTRUCT ps;
    HDC hdc;

    switch (message)
    {
    case WM_COMMAND:
        wmId    = LOWORD(wParam);
        wmEvent = HIWORD(wParam);
        // Parse the menu selections:
        switch (wmId)
        {
        case IDM_ABOUT:
            DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
            break;
        case IDM_EXIT:
            DestroyWindow(hWnd);
            break;
        default:
            return DefWindowProc(hWnd, message, wParam, lParam);
        }
        break;
    case WM_SETFOCUS:
        g_bHasFocus = true;
        break;
    case WM_KILLFOCUS:
        g_bHasFocus = false;
        break;
    case WM_PAINT:
        hdc = BeginPaint(hWnd, &ps);
        EndPaint(hWnd, &ps);
        break;
    case WM_KEYDOWN:
    if ( g_bHasFocus || g_bFullScreen )
    {
        int keyCode = (int)wParam;
        if ( wParam == VK_SHIFT )
        {
            if ( GetAsyncKeyState(VK_LSHIFT) < 0 )
            {
                keyCode = VK_LSHIFT;
            }
            else if ( GetAsyncKeyState(VK_RSHIFT) < 0 )
            {
                keyCode = VK_RSHIFT;
            }
        }
        else if ( wParam == VK_CONTROL )
        {
            if ( GetAsyncKeyState(VK_LCONTROL) < 0 )
            {
                keyCode = VK_LCONTROL;
            }
            else if ( GetAsyncKeyState(VK_RCONTROL) < 0 )
            {
                keyCode = VK_RCONTROL;
            }
        }
        Input::SetKeyDown( keyCode );
    }
        break;
    case WM_KEYUP:
    if ( g_bHasFocus || g_bFullScreen )
    {
        int keyCode = (int)wParam;
        if ( wParam == VK_SHIFT )
        {
            Input::SetKeyUp( VK_LSHIFT );
            Input::SetKeyUp( VK_RSHIFT );
        }
        else if ( wParam == VK_CONTROL )
        {
            Input::SetKeyUp( VK_LCONTROL );
            Input::SetKeyUp( VK_RCONTROL );
        }
        Input::SetKeyUp( keyCode );
    }
        break;
    case WM_LBUTTONDOWN:
        if ( g_bHasFocus || g_bFullScreen )
            Input::SetKeyDown(VK_LBUTTON);
        break;
    case WM_RBUTTONDOWN:
        if ( g_bHasFocus || g_bFullScreen )
            Input::SetKeyDown(VK_RBUTTON);
        break;
    case WM_MBUTTONDOWN:
        if ( g_bHasFocus || g_bFullScreen )
            Input::SetKeyDown(VK_MBUTTON);
        break;
    case WM_XBUTTONDOWN:
        if ( g_bHasFocus || g_bFullScreen )
            Input::SetKeyDown( HIWORD(wParam)==MK_XBUTTON1 ? VK_XBUTTON1 : VK_XBUTTON2 );
        break;
    case WM_LBUTTONUP:
        if ( g_bHasFocus || g_bFullScreen )
            Input::SetKeyUp(VK_LBUTTON);
        break;
    case WM_RBUTTONUP:
        if ( g_bHasFocus || g_bFullScreen )
            Input::SetKeyUp(VK_RBUTTON);
        break;
    case WM_MBUTTONUP:
        if ( g_bHasFocus || g_bFullScreen )
            Input::SetKeyUp(VK_MBUTTON);
        break;
    case WM_XBUTTONUP:
        if ( g_bHasFocus || g_bFullScreen )
            Input::SetKeyUp( HIWORD(wParam)==MK_XBUTTON1 ? VK_XBUTTON1 : VK_XBUTTON2 );
        break;
    case WM_MOUSEMOVE:
        if ( g_bHasFocus || g_bFullScreen )
        {
            float xPos, yPos;
            xPos = (float)LOWORD(lParam);
            yPos = (float)HIWORD(lParam);
            Input::SetMousePos(xPos, yPos);
        }
        break;
    case WM_CHAR:
        if ( g_bHasFocus || g_bFullScreen )
            Input::SetCharacterDown( (char)(wParam&0x7f) );
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

struct CmdLineOptions
{
    char szOption[32];
    char szValue[64];
};
CmdLineOptions options[16];
uint32_t optCount = 0;

void ParseCommandLine(size_t len, const char *pszCmdLine)
{
    for (size_t c=0; c<len && optCount < 16;)
    {
        if ( pszCmdLine[c] == '-' ) //option found.
        {
            c++;
            //read the option.
            size_t start = c;
            while (pszCmdLine[c] != ' ' && c < len)
            {
                options[optCount].szOption[c-start] = pszCmdLine[c];
                c++;
            };
            options[optCount].szOption[c-start] = 0;
            //now read the value.
            c++;
            start = c;
            while (pszCmdLine[c] != ' ' && c < len)
            {
                options[optCount].szValue[c-start] = pszCmdLine[c];
                c++;
            }
            options[optCount].szValue[c-start] = 0;
            optCount++;
            c++;
        }
        else
        {
            c++;
        }
    }
}

bool GetOption(const char *pszOption, char *pszValue)
{
    for (uint32_t i=0; i<optCount; i++)
    {
        if ( stricmp(options[i].szOption, pszOption) == 0 )
        {
            strcpy(pszValue, options[i].szValue);
            return true;
        }
    }
    return false;
}

int32_t GetOptionint32_t(const char *pszOption)
{
    char *pszEndPtr;
    for (uint32_t i=0; i<optCount; i++)
    {
        if ( stricmp(options[i].szOption, pszOption) == 0 )
        {
            return strtol(options[i].szValue, &pszEndPtr, 10);
        }
    }
    return -1;
}

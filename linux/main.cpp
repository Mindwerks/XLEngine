
#include <stdio.h>
#include <stdlib.h>

#include <time.h>
#include <unistd.h>
#include <string.h>

#include "SDL.h"

#include "../Engine.h"
#include "../EngineSettings.h"
#include "../os/Input.h"
#include "../os/Clock.h"


namespace
{

SDL_Window *win;
SDL_GLContext glctx;

Engine *m_pEngine;

bool g_bFullScreen=false;
bool g_bHasFocus=true;

void fatalError(const char *message)
{
    fprintf(stderr, "main: %s\n", message);
    exit(1);
}

void checkSDLError(int ret)
{
    if(ret != 0)
    {
        char msg[256];
        snprintf(msg, sizeof(msg), "SDL error: %s", SDL_GetError());
        fatalError(msg);
    }
}

} // namespace


int main(int argc, char **argv)
{
    const char *game_name = NULL;

    for(int i = 1;i < argc;i++)
    {
        if(strcmp(argv[i], "-g") == 0)
        {
            if(argc-1 > i)
                game_name = argv[++i];
            else
                fatalError("Missing game name");
        }
        else
            fprintf(stderr, "Unhandled command line option: %s\n", argv[i]);
    }
    if(!game_name)
        fatalError("No game specified.\n"
"Usage: XLEngine -g <game>\n"
"\tValid games: DarkXL, DaggerXL, BloodXL, OutlawsXL");

    //Engine settings.
    EngineSettings::Init();

    //We have to load the engine settings before created the window and setting up
    //so that we can pick the correct resolution, fullscreen, etc.
    {
        std::string settingsFile;
        const char *conf_dir = getenv("XDG_CONFIG_HOME");
        if(conf_dir && conf_dir[0])
            settingsFile = conf_dir;
        else
        {
            const char *home_dir = getenv("HOME");
            if(home_dir && home_dir[0])
            {
                settingsFile = home_dir;
                settingsFile += "/.config/";
            }
        }
        if(!settingsFile.empty() && settingsFile.back() != '/')
            settingsFile += '/';
        settingsFile += "XLEngine/";
        settingsFile += game_name;
        settingsFile += ".conf";

        EngineSettings::SetGameDir(game_name);
        EngineSettings::Load(settingsFile.c_str());
    }

    // Init everything but audio
    SDL_Init(SDL_INIT_EVERYTHING & ~SDL_INIT_AUDIO);

    Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;
    if(EngineSettings::IsFeatureEnabled(EngineSettings::FULLSCREEN))
        flags |= SDL_WINDOW_FULLSCREEN;
    int screen = 0;
    int pos_x = SDL_WINDOWPOS_CENTERED_DISPLAY(screen);
    int pos_y = SDL_WINDOWPOS_CENTERED_DISPLAY(screen);
    int width = EngineSettings::GetScreenWidth();
    int height = EngineSettings::GetScreenHeight();

    checkSDLError(SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8));
    checkSDLError(SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8));
    checkSDLError(SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8));
    checkSDLError(SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8));
    checkSDLError(SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24));
    checkSDLError(SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, SDL_TRUE));

    win = SDL_CreateWindow("XL Engine", pos_x, pos_y, width, height, flags);
    if(!win)
        fatalError("Failed to create window");

    glctx = SDL_GL_CreateContext(win);
    if(!glctx)
    {
        SDL_DestroyWindow(win);
        win = nullptr;
        fatalError("Failed to create OpenGL context");
    }

    SDL_GL_SetSwapInterval(EngineSettings::IsFeatureEnabled(EngineSettings::VSYNC) ? 1 : 0);

    //Setup Engine with Linux specific data.
    {
        m_pEngine = new Engine();
        void *linux_param[] = { (void*)win };
        m_pEngine->Init(linux_param, 1, EngineSettings::GetScreenWidth(),
                        EngineSettings::GetScreenHeight());
        m_pEngine->InitGame(game_name);
    }

    SDL_SetRelativeMouseMode(SDL_FALSE);
    Input::EnableMouseLocking(false);
    bool MouseWasLocked = false;

    float fDeltaTime = 0.0f;
    float fMaxDelta  = 0.1f;
    bool bDone = false;
    while(!bDone)
    {
        if(Input::LockMouse() && (g_bHasFocus || g_bFullScreen))
        {
            if(!MouseWasLocked)
            {
                // FIXME: At least DaggerXL doesn't release the mouse once it
                // gets in-game, and there's no way to close the game without
                // alt-tabbing away and being careful. So for now, just don't
                // grab the mouse.
                //SDL_SetRelativeMouseMode(SDL_TRUE);
                MouseWasLocked = true;
            }
        }
        else if(MouseWasLocked)
        {
            SDL_SetRelativeMouseMode(SDL_FALSE);
            SDL_ShowCursor(SDL_TRUE);
            MouseWasLocked = false;
        }

        SDL_Event event;
        while(SDL_PollEvent(&event) == 1)
        {
            switch(event.type)
            {
                case SDL_KEYDOWN:
                    if(event.key.repeat)
                        break;
                    Input::SetKeyDown(event.key.keysym.sym&0xff);
                    break;
                case SDL_KEYUP:
                    if(event.key.repeat)
                        break;
                    Input::SetKeyUp(event.key.keysym.sym&0xff);
                    break;
                case SDL_TEXTINPUT:
                    // This ignores non-ASCII-7 characters (Unicode/UTF-8)
                    for(size_t i = 0;i < sizeof(event.text.text) && event.text.text[i];i++)
                    {
                        if(event.text.text[i] >= 32)
                            Input::SetCharacterDown(event.text.text[i]);
                    }
                    break;

                case SDL_MOUSEBUTTONDOWN:
                    if(event.button.button == 0)
                        Input::SetKeyDown(XL_LBUTTON);
                    else if(event.button.button == 1)
                        Input::SetKeyDown(XL_RBUTTON);
                    else
                        Input::SetKeyDown(XL_MBUTTON+event.button.button-2);
                    break;
                case SDL_MOUSEBUTTONUP:
                    if(event.button.button == 0)
                        Input::SetKeyUp(XL_LBUTTON);
                    else if(event.button.button == 1)
                        Input::SetKeyUp(XL_RBUTTON);
                    else
                        Input::SetKeyUp(XL_MBUTTON+event.button.button-2);
                    break;

                case SDL_MOUSEMOTION:
                    if(MouseWasLocked)
                        Input::AddMouseDelta(event.motion.xrel, event.motion.yrel);
                    else
                        Input::SetMousePos(event.motion.x, event.motion.y);
                    break;

                case SDL_QUIT:
                    bDone = true;
                    break;
            }
        }

        Clock::StartTimer();
        {
            if(!m_pEngine->Loop(fDeltaTime, g_bHasFocus||g_bFullScreen))
                bDone = true;
        }
        fDeltaTime = Clock::GetDeltaTime(fMaxDelta);

        usleep(1000);
    }

    delete m_pEngine;
    m_pEngine = NULL;

    SDL_GL_MakeCurrent(0, 0);
    SDL_GL_DeleteContext(glctx);
    glctx = 0;
    SDL_DestroyWindow(win);
    win = 0;

    SDL_Quit();

    return 0;
}

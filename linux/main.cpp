/* A simple program to show how to set up an X window for OpenGL rendering.
 * X86 compilation: gcc -o -L/usr/X11/lib   main main.c -lGL -lX11
 * X64 compilation: gcc -o -L/usr/X11/lib64 main main.c -lGL -lX11
 */
#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>
#include <GL/glx.h>    /* this includes the necessary X headers */
#include <GL/gl.h>

#include <X11/X.h>    /* X11 constant (e.g. TrueColor) */
#include <X11/keysym.h>

#include <time.h>
#include <unistd.h>
#include <string.h>

#include "../Engine.h"
#include "../os/Input.h"
#include "../os/Clock.h"

static int dblBuf[]  = {GLX_RGBA, GLX_RED_SIZE, 8, GLX_GREEN_SIZE, 8, GLX_BLUE_SIZE, 8, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None};

static Display   *dpy;
static Window     win;

static Engine *m_pEngine;

static bool g_bFullScreen=false;
static bool g_bHasFocus=true;

static void fatalError(const char *message)
{
  fprintf(stderr, "main: %s\n", message);
  exit(1);
}

int main(int argc, char **argv)
{
  const char          *game_name = NULL;
  XVisualInfo         *vi;
  Colormap             cmap;
  XSetWindowAttributes swa;
  GLXContext           cx;
  XEvent               event;
  int                  dummy;
  int                  i;
  Atom wmDelete;

  for(i = 1;i < argc;i++)
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

  /*** (1) open a connection to the X server ***/
  dpy = XOpenDisplay(NULL);
  if (dpy == NULL)
    fatalError("could not open display");

  /*** (2) make sure OpenGL's GLX extension supported ***/
  if(!glXQueryExtension(dpy, &dummy, &dummy))
    fatalError("X server has no OpenGL GLX extension");

  /*** (3) find an appropriate visual ***/
  /* find an OpenGL-capable RGB visual with depth buffer */
  vi = glXChooseVisual(dpy, DefaultScreen(dpy), dblBuf);

  /*** (4) create an OpenGL rendering context  ***/
  /* create an OpenGL rendering context */
  cx = glXCreateContext(dpy, vi, /* no shared dlists */ None, /* direct rendering if possible */ GL_TRUE);
  if (cx == NULL)
    fatalError("could not create rendering context");

  /*** (5) create an X window with the selected visual ***/
  /* create an X colormap since probably not using default visual */
  cmap = XCreateColormap(dpy, RootWindow(dpy, vi->screen), vi->visual, AllocNone);
  swa.colormap = cmap;
  swa.border_pixel = 0;
  swa.event_mask = KeyPressMask | KeyReleaseMask | ExposureMask
                 | ButtonPressMask | ButtonReleaseMask | StructureNotifyMask;
  win = XCreateWindow(dpy, RootWindow(dpy, vi->screen), 0, 0,
                      1024, 768, 0, vi->depth, InputOutput, vi->visual,
                      CWBorderPixel | CWColormap | CWEventMask, &swa);

  /* only set window title and handle wm_delete_events if in windowed mode */
  wmDelete = XInternAtom(dpy, "WM_DELETE_WINDOW", True);
  XSetWMProtocols(dpy, win, &wmDelete, 1);

  XSetStandardProperties(dpy, win, "XL Engine", "XL Engine", None, argv, argc, NULL);

  XMapRaised(dpy, win);

  /*** (6) bind the rendering context to the window ***/
  glXMakeCurrent(dpy, win, cx);

  /*** (7) request the X window to be displayed on the screen ***/
  XMapWindow(dpy, win);

//Setup Engine with Linux specific data.
  m_pEngine = new Engine();
  void *linux_param[] = { (void*)dpy, (void*)win };
  m_pEngine->Init(linux_param, 2, 1024, 768);
  m_pEngine->InitGame( game_name );

  char buffer[32];
  int bufsize=32;
  KeySym keysym;
  XComposeStatus compose;
  int count;
  float fDeltaTime = 0.0f;
  float fMaxDelta  = 0.1f;

  /*** (9) dispatch X events ***/
  bool bDone = false;
  while ( !bDone )
  {
    while(XPending(dpy) > 0)
    {
      XNextEvent(dpy, &event);
      switch (event.type)
      {
        case Expose:
            if (event.xexpose.count != 0)
                break;
            Clock::StartTimer();
            {
                m_pEngine->Loop(fDeltaTime, g_bHasFocus||g_bFullScreen);
            }
            fDeltaTime = Clock::GetDeltaTime(fMaxDelta);
            break;
        case KeyPress:
        {
          /* It is necessary to convert the keycode to a
           * keysym before checking if it is an escape */
          XKeyEvent *kevent = (XKeyEvent *) &event;
          count = XLookupString(kevent, buffer, bufsize, &keysym, &compose);

          if ( count > 0 )
          {
            Input::SetKeyDown( (int)keysym&0xff );
            if ( keysym >= 32 && keysym < 128 )
            {
                Input::SetCharacterDown( (char)keysym );
            }
          }
          break;
        }
        case KeyRelease:
        {
          /* It is necessary to convert the keycode to a
           * keysym before checking if it is an escape */
          XKeyEvent *kevent = (XKeyEvent *) &event;
          count = XLookupString(kevent, buffer, bufsize, &keysym, &compose);

          if ( count > 0 )
            Input::SetKeyUp( (int)keysym&0xff );
          break;
        }
        case ButtonPress:
          switch (event.xbutton.button)
          {
            case 1:
              Input::SetKeyDown( XL_LBUTTON );
              break;
            case 2:
              Input::SetKeyDown( XL_RBUTTON );
              break;
            default:
              if ( event.xbutton.button > 0 )
              {
                Input::SetKeyDown( XL_MBUTTON+event.xbutton.button-3 );
              }
              break;
          }
          break;
        case ButtonRelease:
          switch (event.xbutton.button)
          {
            case 1:
              Input::SetKeyUp( XL_LBUTTON );
              break;
            case 2:
              Input::SetKeyUp( XL_RBUTTON );
              break;
            default:
              if ( event.xbutton.button > 0 )
              {
                Input::SetKeyUp( XL_MBUTTON+event.xbutton.button-3 );
              }
              break;
          }
          break;
        case ClientMessage:
            if (strcmp(XGetAtomName(dpy, event.xclient.message_type), "WM_PROTOCOLS") == 0)
            {
                bDone = True;
            }
            break;
      }
    }; /* loop to compress events */
    Clock::StartTimer();
    {
        m_pEngine->Loop(fDeltaTime, g_bHasFocus||g_bFullScreen);
    }
    fDeltaTime = Clock::GetDeltaTime(fMaxDelta);

    usleep(1000);
  }

  if ( m_pEngine )
  {
    delete m_pEngine;
  }
  m_pEngine = NULL;

    if( cx )
    {
        if( !glXMakeCurrent(dpy, None, NULL))
        {
            printf("Could not release drawing context.\n");
        }
        /* destroy the context */
        glXDestroyContext(dpy, cx);
        cx = NULL;
    }
    XCloseDisplay(dpy);

  return 0;
}

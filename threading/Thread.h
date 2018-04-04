#ifndef THREAD_H
#define THREAD_H

#include <errno.h>
#include "../CommonTypes.h"

#if PLATFORM_WIN
  #include "Win32/Thread.h"
#else
  #include "Posix/Thread.h"
#endif

#endif // THREAD_H

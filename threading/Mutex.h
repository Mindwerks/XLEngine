#ifndef MUTEX_H
#define MUTEX_H

#include <errno.h>
#include "../CommonTypes.h"

#if PLATFORM_WIN
#include "Win32/Mutex.h"
#else

#include "Posix/Mutex.h"

#endif

#endif // MUTEX_H

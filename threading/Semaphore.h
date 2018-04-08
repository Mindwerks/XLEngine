#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <errno.h>
#include "../CommonTypes.h"

#if PLATFORM_WIN
#include "Win32/Semaphore.h"
#else

#include "Posix/Semaphore.h"

#endif

#endif // SEMAPHORE_H

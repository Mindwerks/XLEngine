#ifndef PLATFORM_DEF_H
#define PLATFORM_DEF_H

//platform defines.
#ifdef _WIN32
#define PLATFORM_WIN 1
#else
#define PLATFORM_WIN 0
#endif

#ifdef __linux__
#define    PLATFORM_LINUX 1
#else
#define PLATFORM_LINUX 0
#endif

#ifdef __APPLE__
#define PLATFORM_OSX 1
#else
#define PLATFORM_OSX 0
#endif

#endif //PLATFORM_DEF_H
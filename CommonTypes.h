#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#include "PlatformDef.h"
#include <cstring>
#include <cstdint>
#include <cstdbool>

#if PLATFORM_WIN

//Enable this to check memory allocations.
//This is currently only available on Windows
#ifdef _CRTDBG_MAP_ALLOC
    #include <stdlib.h>
    #include <CRTDBG.h>

    #define xlMalloc(s)       _malloc_dbg(s, _NORMAL_BLOCK, __FILE__, __LINE__)
    #define xlCalloc(c, s)    _calloc_dbg(c, s, _NORMAL_BLOCK, __FILE__, __LINE__)
    #define xlRealloc(p, s)   _realloc_dbg(p, s, _NORMAL_BLOCK, __FILE__, __LINE__)
    #define xlExpand(p, s)    _expand_dbg(p, s, _NORMAL_BLOCK, __FILE__, __LINE__)
    #define xlFree(p)         _free_dbg(p, _NORMAL_BLOCK)
    #define xlMemSize(p)      _msize_dbg(p, _NORMAL_BLOCK)

    #define xlNew new(_NORMAL_BLOCK, __FILE__, __LINE__)
    #define xlDelete delete
#else
    #define xlMalloc malloc
    #define xlCalloc calloc
    #define xlRealloc realloc
    #define xlExpand _expand
    #define xlFree free
    #define xlMemSize _msize

    #define xlNew new
    #define xlDelete delete
#endif


#elif PLATFORM_LINUX


#define stricmp strcasecmp
#define strnicmp strncasecmp

//memory
#define xlMalloc malloc
#define xlCalloc calloc
#define xlRealloc realloc
#define xlExpand _expand
#define xlFree free
#define xlMemSize _msize

#define xlNew new
#define xlDelete delete

#endif

typedef int32_t XL_BOOL;

#define XL_INVALID_TEXTURE 0xffffffff
typedef uint32_t TextureHandle;

//Make sure that the point is non-null before delete. Set to NULL.
#define SafeDeleteArr(p) if (p) { xlDelete [] (p); (p) = NULL; }
//p must be non-null before deletion. Test and set to null.
#define SafeDeleteArr_Test(p) { assert(p); if (p) { xlDelete [] (p); (p) = NULL; } }

#endif  //COMMON_TYPES

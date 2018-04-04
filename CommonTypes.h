#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H

#include "PlatformDef.h"

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

typedef unsigned char       u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;

typedef char       s8;
typedef short     s16;
typedef int       s32;
typedef long long s64;

typedef float  f32;
typedef double f64;

#elif PLATFORM_LINUX

typedef unsigned char       u8;
typedef unsigned short     u16;
typedef unsigned int       u32;
typedef unsigned long long u64;

typedef char       s8;
typedef short     s16;
typedef int       s32;
typedef long long s64;

typedef float  f32;
typedef double f64;

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

typedef s32 XL_BOOL;
#define XL_TRUE  1
#define XL_FALSE 0

#define XL_INVALID_TEXTURE 0xffffffff
typedef u32 TextureHandle;

//Make sure that the point is non-null before delete. Set to NULL.
#define SafeDeleteArr(p) if (p) { xlDelete [] (p); (p) = NULL; }
//p must be non-null before deletion. Test and set to null.
#define SafeDeleteArr_Test(p) { assert(p); if (p) { xlDelete [] (p); (p) = NULL; } }

#endif	//COMMON_TYPES

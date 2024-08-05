#pragma once

// by Alexander Ilyin
// belongs at Drag Watcher project

// *I find these types rather old; u8, u16, u32 - are more convenient
// define unsigned types
typedef unsigned char byte;
typedef unsigned short ushort;
typedef unsigned int u32;
typedef unsigned long ulong;
typedef unsigned __int64 uint64;

// define signed types
typedef __int64 int64;
typedef signed __int64 sint64;

// C4244 is more disturbing and forcing you write extra code, than helping
// have to fight with this on a proper level
#pragma warning(disable:4244)
// even a conversions to bool I am making are intentional, and such a conversion appears erroneous very very rare
#pragma warning(disable:4800)

// *here first I tried to define types like 'i1' for 8 bit, but appears it is not good idea (for example 's1' makes a collide between a type and a var which we prefer to use)
// *with types like '__int8', these typedefs are looking nice, but later appear problems of incompatibility
// a pointer to such type is not compatible with lets say 'byte', and 'std::cout' doesn't know what to do with them either

// define unsigned types
#if 0
typedef unsigned __int8 u8;
typedef unsigned __int16 u16;
typedef unsigned __int32 u32;
typedef unsigned __int64 u64;
#endif
typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned __int64 u64;
// '__int128' basicly is not supported on x86
//typedef unsigned __int128 u128;

// define signed types
#if 0
typedef signed __int8 s8;
typedef signed __int16 s16;
typedef signed __int32 s32;
typedef signed __int64 s64;
//typedef signed __int128 s128;
#endif
typedef signed char s8;
typedef signed short s16;
typedef signed int s32;
typedef signed __int64 s64;
//typedef signed __int128 s128;

// testing; maybe these will appear convenient in use too
typedef float fl32;
typedef double fl64;

// for MSVC 6.0
//#ifndef wchar_t
#if defined(_MSC_VER) && _MSC_VER <= 1200
//typedef u16 wchar_t;
typedef ushort wchar_t;
#endif

#define PTRBYTESNUM (sizeof(void*))
#define PTRBITSNUM (PTRBYTESNUM * 8)


// *no define for it in SDK or DDK (GTI_SECONDS only)
#define TI_MICROSECOND ((uint64)10)
#define TI_MILLISECOND (1000 * TI_MICROSECOND)
#define TI_SECOND (1000 * TI_MILLISECOND)
#define TI_MINUTE (60 * TI_SECOND)


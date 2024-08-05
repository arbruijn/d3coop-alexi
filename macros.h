#pragma once

// belongs at Discoloured project

#include "types.h"
// 'decltype' is introduced in MSVC 2010
// *for GCC there is '__typeof__'
#if defined(_MSC_VER) && _MSC_VER < 1600
#include "typeof.h"
//#define decltype BOOST_TYPEOF
//#define var_type_id BOOST_TYPEOF_INDEX
#endif


// Binary constant generator macro
// By Tom Torfs - donated to the public domain

/* All macro's evaluate to compile-time constants */

/* *** helper macros *** */

/* turn a numeric literal into a hex constant
(avoids problems with leading zeroes)
8-bit constants max value 0x11111111, always fits in unsigned long
*/
#define HEX__(n) 0x##n##LU

/* 8-bit conversion function */
#define B8__(x) ((x&0x0000000FLU)?1:0) \
+((x&0x000000F0LU)?2:0) \
+((x&0x00000F00LU)?4:0) \
+((x&0x0000F000LU)?8:0) \
+((x&0x000F0000LU)?16:0) \
+((x&0x00F00000LU)?32:0) \
+((x&0x0F000000LU)?64:0) \
+((x&0xF0000000LU)?128:0)

/* *** user macros *** */

/* for upto 8-bit binary constants */
#define B8(d) ((unsigned char)B8__(HEX__(d)))

// Alex :  'B32' is removed; in a slightly better, modified (stringized) shape you can get it from the "chess" project

#if 0
// Sample usage:
B8(01010101) = 85
B16(10101010,01010101) = 43605
B32(10000000,11111111,10101010,01010101) = 2164238933
#endif


// This self-explanatory example was sent by Bill Finke
#define Ob(x)  ((unsigned)Ob_(0 ## x ## uL))
#define Ob_(x) (x & 1 | x >> 2 & 2 | x >> 4 & 4 | x >> 6 & 8 |		\
	x >> 8 & 16 | x >> 10 & 32 | x >> 12 & 64 | x >> 14 & 128)

// Alex :  further goes mine stuff

#define PI 3.14159265358979323846264338327950288
#define PI2 (PI * 2)
// *'E()' is already in use for much more common stuff - an error
#define E_ 2.71828182845904523536028747135266250

#define ISDIGIT(_char) ((_char) >= '0' && (_char) <= '9')

//#ifndef __cplusplus
#ifndef max
#define max(_a, _b)    (((_a) > (_b)) ? (_a) : (_b))
#endif
#ifndef min
#define min(_a, _b)    (((_a) < (_b)) ? (_a) : (_b))
#endif
//#endif

// this macro is for a defined value for the comparison;
// for the case, where the comparison will need a calculation, and will be necessary a new var, use the second macros,
// which may appear slower, if will be used there, where is necessary this macro (where no calculation is necessary)
#define MINIMIZE(_var, _value) do {if ((_var) > (_value)) _var = (_value);} while (0)
#define MAXIMIZE(_var, _value) do {if ((_var) < (_value)) _var = (_value);} while (0)
// **can't do it now proper, with C++ 2003; only with the next standard it will be possible (due to a C type definition limits)
// minimize with a calculated argument
#define MINIMIZE_VAR(_var, _value) do {decltype(_value) result_val; result_val = _value; if ((_var) > result_val) _var = result_val;} while (0)
#define MAXIMIZE_VAR(_var, _value) do {decltype(_value) result_val; result_val = _value; if ((_var) < result_val) _var = result_val;} while (0)

// C must reserve 'numof' keyword for that
// *in MSVC aka '_countof'
// '*' has a problem
#define NUMOF(_obj) (sizeof(_obj) / sizeof(*(_obj)))
//#define NUMOF(_obj) (sizeof(_obj) / sizeof((_obj)[0]))

// *on MSVC 6.0 it can be used only once per '{}' body, because there the variable is declared outside of the 'for' scope
//#define FORI(_begin, _end) for (i = (_begin); i < (_end); i++)
#define _FORx(_x, _begin, _end) for (u32 _x = (_begin); _x < (unsigned)(_end); _x++)
#define FORI(_begin, _end) _FORx(i, _begin, _end)
// *this is prefered to 'j', because 'ii' has more obvious difference than 'j', when comparing them to 'i'
#define FORII(_begin, _end) _FORx(ii, _begin, _end)
// *don't use 'iiii'; that is a lack of thinking :)
#define FORIIII(_begin, _end) _FORx(iiii, _begin, _end)
#define FORJ(_begin, _end) _FORx(j, _begin, _end)
#define FORX(_begin, _end) _FORx(x, _begin, _end)
#define FORY(_begin, _end) _FORx(y, _begin, _end)
#define FORI0(_end) FORI(0, _end)
#define FORI1(_end) FORI(1, _end)
#define FORII0(_end) FORII(0, _end)
#define FORIIII0(_end) FORIIII(0, _end)
#define FORJ0(_end) FORJ(0, _end)
#define FORX0(_end) FORX(0, _end)
#define FORY0(_end) FORY(0, _end)
// *from end to begin
#define FORI_R(_begin, _end) for (u32 i = (_end) - 1, _before_begin = (_begin) - 1; i != (_before_begin); i--)
#define FORI0_R(_end) FORI_R(0, _end)
// hmmm, hmmmmmm
//#define FORI_R(_begin, _end) for (u32 i = (_begin) - 1, _i_end = (_end) - 1; i != (_i_end); i--)
//#define FORI0_R(_begin) FORI_R(_begin, 0)

// C++ STL oriented; for everything what has '::iterator'
// wow, does not compile on MSVC 6.0; GCC 4.6.0 compiles it
//#define FOREACH(_id, _arr) for (decltype((_arr).begin()) (_id) = (_arr).begin(); (_id) != (_arr).end(); (_id)++)
//#define FOREACH(_id, _arr) for (decltype((_arr).begin()) _id = (_arr).begin(); (_id) != (_arr).end(); (_id)++)
// this version is insensitive to occasional '_arr' change
#define FOREACH(_id, _arr) for (decltype((_arr).begin()) _id = (_arr).begin(), _id_end = (_arr).end(); (_id) != (_id_end); (_id)++)
// with external declaration (the iterator sometimes is being necessary after this loop; note, on MSVC 6.0 it also is available after 'FOREACH')
#define FOREACH_EXT(_id, _arr) decltype((_arr).begin()) _id, _id_end; for (_id = (_arr).begin(), _id_end = (_arr).end(); (_id) != (_id_end); (_id)++)

// remove this garbage if real 'static_assert()' is in support
// it gives "error C2466: cannot allocate an array of constant size 0" in case of a failed assert
// don't use '_STATIC_ASSERT' name - it will conflict with MSVCs like 8.0
// become too classic - don't write such long string; 'SA()' is completely enough for this importantiest macro
//#define STATIC_ASSERT(_expr) typedef char _static_assert_t[!! (_expr)]
#define SA(_expr) typedef char _static_assert_t[!! (_expr)]
// is this crutch necessary at all ? :/  I don't think so
//#define static_assert(_expr, _cstr) SA(_expr)

// *tricked; that is because otherwise will be stringized a define name, instead of its value
#if 0
#define STRINGIZE(_val) #_val
#else
#define STRINGIZE(_text) STRINGIZE_A((_text))
#define STRINGIZE_A(_arg) STRINGIZE_I _arg
#define STRINGIZE_I(_text) #_text
#endif

#define ARRCPY(_arr0, _arr1) do { SA(sizeof(_arr0) == sizeof(_arr1)); memcpy (_arr0, _arr1, sizeof(_arr0)); } while (0)


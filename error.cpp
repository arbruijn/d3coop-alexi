// by Alexander Ilyin

#include "error.h"
//#include <stdlib.h>
#include <stdio.h>
#ifdef WIN32
#include <windows.h>
#else
#include <stdarg.h>
#endif
#include "screen.h"


static va_list argptr;
// old GCC dislike backslashes
// *proper gcc version I don't know; 4.3.2 supports; 2.8.1 not supports
// __GNUC_MINOR__ __GNUC_PATCHLEVEL__
#if ((! defined __GNUC__) || (__GNUC__ >= 3))
#define VA_PRINTF \
	va_start (argptr, format); \
	vsprintf (buffer_string, format, argptr); \
	va_end (argptr);
#else
#define VA_PRINTF {va_start (argptr, format); vsprintf (buffer_string, format, argptr); va_end (argptr);}
#endif


void Error_NoExit (const char* format, ...)
{
static char buffer_string[1024];

	VA_PRINTF;

#ifdef WIN32
	Screen_Restore ();
	MessageBox (0, buffer_string, "Descent 3 Co-op Mod error", MB_OK);
	Screen_Restore ();
#endif
#ifdef __unix__
	printf ("\aDescent 3 Co-op Mod error: %s\n", buffer_string);
#endif
}

void Error (const char* format, ...)
{
static char buffer_string[1024];

	VA_PRINTF;

#ifdef WIN32
	Screen_Restore ();
	MessageBox (0, buffer_string, "Descent 3 Co-op Mod error", MB_OK);
// restore once again, for the debugger; after F10, the screen again goes black, and have to restore it
	Screen_Restore ();
#endif
#ifdef __unix__
	printf ("\aDescent 3 Co-op Mod error: %s\n", buffer_string);
#endif

// *I think 'exit()' sometimes hangs :  message box is closed, but "main.exe" still works
// lets try 'ExitProcess()' instead
//	exit (-1);
	ExitProcess (1);
}


#pragma once


void Error_NoExit (const char* format, ...);
// 'Error_NoExit ()' is a crappy name; simplify for the new name
#define ErrorReport Error_NoExit
void Error (const char* fmt, ...);
#define Assert(_condition_) \
if (! (_condition_)) \
	Error ("Assertion \"%s\" failed", #_condition_);
// duplicate
#define A Assert


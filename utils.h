#pragma once

#include "types.h"


//const char* Utils_GetDate ();
bool Utils_GetTime (u32* year = 0, u32* month = 0, u32* day = 0, u32* hour = 0, u32* minute = 0, u32* second = 0);
const char* Utils_GetDateString ();
const char* Utils_GetDateString_Month ();


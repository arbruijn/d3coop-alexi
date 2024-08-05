//

#include "utils.h"
#ifdef WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#endif
#include <stdio.h>


bool Utils_GetTime (u32* year, u32* month, u32* day, u32* hour, u32* minute, u32* second)
{
#ifdef WIN32
	SYSTEMTIME systemtime;

	GetSystemTime (&systemtime);
	if (year)
		*year = systemtime.wYear;
	if (month)
		*month = systemtime.wMonth;
	if (day)
		*day = systemtime.wDay;
	if (hour)
		*hour = systemtime.wHour;
	if (minute)
		*minute = systemtime.wMinute;
	if (second)
		*second = systemtime.wSecond;
	return 1;
#else
	bool result;
	time_t system_time;
//	struct tm sliced_time;
	tm sliced_time;

	time (&system_time);
	result = !! gmtime_r (&system_time, &sliced_time);
	if (! result)
		return 0;
	if (year)
		*year = sliced_time.tm_year;
	if (month)
		*month = sliced_time.tm_mon;
	if (day)
		*day = sliced_time.tm_mday;
	if (hour)
		*hour = sliced_time.tm_hour;
	if (minute)
		*minute = sliced_time.tm_min;
	if (second)
		*second = sliced_time.tm_sec;
	return 1;
#endif
}

// static string
// almost by ISO 8601, except that no UTC showing 'Z' is appended
const char* Utils_GetDateString ()
{
	bool result;
	static char buffer[128];
	u32 year;
	u32 month;
	u32 day;
	u32 hour;
	u32 minute;
	u32 second;

	result = Utils_GetTime (&year, &month, &day, &hour, &minute, &second);
	if (result)
		sprintf (buffer, "%04d-%02d-%02d %02d:%02d:%02d", year, month, day, hour, minute, second);
	else
		strcpy (buffer, "----------");

	return buffer;
}

// static string
const char* Utils_GetDateString_Month ()
{
	bool result;
	static char buffer[32];
	u32 year;
	u32 month;

	result = Utils_GetTime (&year, &month);
	if (result)
		sprintf (buffer, "%04d-%02d", year, month);
	else
		strcpy (buffer, "----------");

	return buffer;
}


// single thread oriented !

#include "log_file.h"
#ifdef WIN32
#include <windows.h>
#else
#include <sys/stat.h>
#endif
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "error.h"
#include "utils.h"
#include "coop_mod.h"

#ifdef WIN32
#define snprintf _snprintf
// not defined in MSVC 6.0 - 8.0 headers
#define PSEP "\\"
#else
#define PSEP "/"
#endif


#if LOG_CLIENTS
char log_file__name[1024];
//char* log_file__name = 0;
//char log_file__name_temp[1024];
char log_file__name_temp[1024] = "cl_list_temp.txt";
#endif
#ifndef WIN32
static FILE* log_file = 0;
#endif
// + directories +
string log_file__log_dir;
#if LOG_CONNECTIONS
string log_file__log_dir__connections;
#endif
#if LOG_CLIENTS
string log_file__log_dir__clients;
#endif
#if LOG_CHAT
string log_file__log_dir__chat;
#endif
// - directories -
string log_file__connections_log_path;
#if LOG_COMMON
string log_file__single_log_path;
#endif
string log_file__chat_log_path;
// + config +
// and this I think you would also like to see 0
//bool log_connections = 1;
bool log_connections = 0;
// Thomas, by your request :  it is set to 0 by default
//bool log_chat = 1;
bool log_chat = 0;
// - config -

static va_list argptr;
static char buffer_string[1024];
#if ((! defined __GNUC__) || (__GNUC__ >= 3))
#define VA_PRINTF \
	va_start (argptr, format); \
	vsprintf (buffer_string, format, argptr); \
	va_end (argptr);
#else
#define VA_PRINTF {va_start (argptr, format); vsprintf (buffer_string, format, argptr); va_end (argptr);}
#endif


#if LOG_CLIENTS
void LogFile_FindNextName ()
{
	static char buffer1[1024];
	int i;

// *not fastest algo
	for (i = 1; i < 0x10000; i++)
	{
		snprintf (buffer1, sizeof(buffer1) - 1, "%s"PSEP"%04x.txt", log_file__log_dir__clients, i);
#ifdef WIN32
		DWORD D_result;

		D_result = GetFileAttributes (buffer1);
		if (D_result == -1)
			break;
#else
		struct stat statbuf;
		int i_result;

		i_result = stat (buffer1, &statbuf);
		if (i_result)
// assume that the file does not exist
			break;
#endif
	}

	strcpy (log_file__name, buffer1);
//	Error ("Next log file is :  \"%s\"", log_file__name);
}
#endif

// check - does directory exist; if not - create
void LogFile_Directory_CheckCreate (const char* dir_path)
{
#ifdef WIN32
	BOOL B_result;
	DWORD D_result;
//	HANDLE handle_file = 0;

	D_result = GetFileAttributes (dir_path);
	if (D_result != -1)
	{
		if (D_result & FILE_ATTRIBUTE_DIRECTORY)
			return;
		else
			Error ("File \"%s\" exists, so can't create directory with that name.", dir_path);
	}
	B_result = CreateDirectory (dir_path, 0);
	if (! B_result)
		Error ("Can't create \"%s\" folder for logs.", dir_path);
#else
	struct stat statbuf;
	int i_result;

	i_result = stat (dir_path, &statbuf);
	if (! i_result)
	{
		if (S_ISDIR(stat.st_mode))
			return;
		else
			Error ("File \"%s\" exists, so can't create directory with that name.", dir_path);
	}
	i_result = mkdir (dir_path, 0777);
	if (i_result)
		Error ("Can't create \"%s\" folder for logs.", dir_path);
#endif
}

void LogFile_DefineLogNames (ushort port)
{
	static char buffer0[1024] = {0};

//	__asm int 3
#ifdef WIN32
	DWORD D_result;

// 0 terminated
	D_result = GetCurrentDirectory (sizeof(buffer0), buffer0);
	if (! D_result)
		return;
#else
	char* c_result;

	c_result = getcwd (buffer0, sizeof(buffer0) - 1);
	if (! c_result)
		return;
#endif

#if LOG_COMMON | LOG_CONNECTIONS | LOG_CLIENTS | LOG_CHAT
// + logs dir +
	log_file__log_dir = buffer0;
	log_file__log_dir += PSEP;
	log_file__log_dir += LOGS_DIR;
	LogFile_Directory_CheckCreate (log_file__log_dir.c_str());
// - logs dir -
#endif

#if LOG_CONNECTIONS
// + connections dir +
	log_file__log_dir__connections = log_file__log_dir;
	log_file__log_dir__connections += PSEP;
	log_file__log_dir__connections += "connections";
	LogFile_Directory_CheckCreate (log_file__log_dir__connections.c_str());
// - connections dir -
#endif

#if LOG_CLIENTS
// + clients dir +
	log_file__log_dir__clients = log_file__log_dir;
	log_file__log_dir__clients += PSEP;
	log_file__log_dir__clients += "clients";
	LogFile_Directory_CheckCreate (log_file__log_dir__clients.c_str());
// - clients dir -
#endif

#if LOG_CHAT
// + chat dir +
	log_file__log_dir__chat = log_file__log_dir;
	log_file__log_dir__chat += PSEP;
	log_file__log_dir__chat += "chat";
	LogFile_Directory_CheckCreate (log_file__log_dir__chat.c_str());
// - chat dir -
#endif

// renew every month
	const char* date_month;
	date_month = Utils_GetDateString_Month ();
	sprintf (buffer0, "%u", port);
#if LOG_COMMON
	log_file__single_log_path = log_file__log_dir;
	log_file__single_log_path += PSEP;
	log_file__single_log_path += "common.txt";
#endif
#if LOG_CONNECTIONS
	log_file__connections_log_path = log_file__log_dir__connections;
	log_file__connections_log_path += PSEP;
	log_file__connections_log_path += date_month;
// personal preference; turn it on, if there are few servers
#if 0
//#if 1
	log_file__connections_log_path += "_";
	log_file__connections_log_path += buffer0;
#endif
	log_file__connections_log_path += ".txt";
#endif
#if LOG_CLIENTS
	LogFile_FindNextName ();
#endif
#if LOG_CHAT
	log_file__chat_log_path = log_file__log_dir__chat;
	log_file__chat_log_path += PSEP;
	log_file__chat_log_path += date_month;
	log_file__chat_log_path += "_";
	log_file__chat_log_path += buffer0;
	log_file__chat_log_path += ".txt";
#endif
}

#if LOG_CLIENTS
void LogFile_Clear ()
{
//	assert(log_file__name);
	assert(log_file__name_temp[0]);

#ifdef WIN32
	BOOL B_result;
	HANDLE handle_file = 0;

	handle_file = CreateFile (log_file__name_temp, GENERIC_WRITE, FILE_SHARE_READ, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	B_result = CloseHandle (handle_file);
#else
	Assert(! log_file);

	log_file = fopen (log_file__name_temp, "w");
	fclose (log_file);
	log_file = 0;
#endif
}

void LogFile_Copy ()
{
#ifdef WIN32
	BOOL B_result;
	B_result = CopyFile (log_file__name_temp, log_file__name, 0);
#else
	ifstream in (log_file__name_temp);
	ofstream out (log_file__name);
	out << in.rdbuf ();
	in.close ();
	out.close ();
#endif
}
#endif

void LogFile_Print (const char* file_name, const char* str)
{
#ifdef WIN32
	int i;
	int string_length = strlen (str);
	BOOL B_result;
	HANDLE handle_file = 0;

//	assert(log_file__name);

	handle_file = CreateFile (file_name, GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if (handle_file == INVALID_HANDLE_VALUE)
		return;

	DWORD bytes_written = 0;
	DWORD D_result = SetFilePointer (handle_file, 0, NULL, FILE_END);
	if (D_result == 0xffffffff)
		Error ("'LogFile_Print ()' :  'SetFilePointer ()' fails");

//	B_result = WriteFile (handle_file, str, strlen (str), &bytes_written, NULL);
//	if (B_result == 0)
//		Error ("'LogFile_Print ()' :  'WriteFile ()' fails");

// handle line endings
	for (i = 0; i < string_length; i++)
	{
		char character = str[i];

		if (character == '\n')
		{
			B_result = WriteFile (handle_file, "\r", 1, &bytes_written, NULL);
			if (B_result == 0)
				Error ("'LogFile_Print ()' :  'WriteFile ()' fails");
		}

		B_result = WriteFile (handle_file, &character, 1, &bytes_written, NULL);
		if (B_result == 0)
			Error ("'LogFile_Print ()' :  'WriteFile ()' fails");
	}

	B_result = CloseHandle (handle_file);

	return;
#else
	assert(! log_file);

	log_file = fopen (file_name, "a");
	fprintf (log_file, str);
	fclose (log_file);
	log_file = 0;
#endif
}

void LogFile_PrintLine_WithDate (const char* file_name, const char* str)
{
	static char buffer[1024] = {0};
	const char* date;

// do not try to log if it is a client;
// todo :  currently, client gets the proper port number, while the server does not;
// atm, the client lacks of receiving of much various notifications (almost does not gets anything); so no sense to try to log;
//	if (! local_role_is_server)
//		return;

// compare it visually, which one is better readable :
// 2010-01-01 21:01:46   #2, "77.206.200.106:2092", [France]
// 2010-01-01 21:01:46   #2, "77.206.200.106:2092", [France]
// [2010-01-01 21:01:46] #2, "77.206.200.106:2092", [France]
// [2010-01-01 21:01:46] #2, "77.206.200.106:2092", [France]
// [2010-01-01 21:01:46]   #2, "77.206.200.106:2092", [France]
// [2010-01-01 21:01:46]   #2, "77.206.200.106:2092", [France]
	date = Utils_GetDateString ();
	snprintf (buffer, sizeof(buffer) - 1, "[%s] %s\n", date, str);
	LogFile_Print (file_name, buffer);
}

// this function seems is from another opera
void LogFile_Clients_Printf (const char* format, ...)
{
#if LOG_CLIENTS
	VA_PRINTF;

	LogFile_PrintLine_WithDate (log_file__name_temp, buffer_string);

	return;
#endif
}

void LogFile_Connections_Printf (const char* format, ...)
{
#if LOG_CONNECTIONS
	if (! log_connections)
		return;

	VA_PRINTF;

	LogFile_PrintLine_WithDate (log_file__connections_log_path.c_str(), buffer_string);

	return;
#endif
}

// and this one's name isn't very proper, since atm here collects much various information too
void LogFile_Chat_Printf (const char* format, ...)
{
#if LOG_CHAT
	if (! log_chat)
		return;
// do not try to log if it is a client;
// atm, the client lacks of receiving of much various notifications (almost does not get anything); so no sense try log it;
	if (! local_role_is_server)
		return;

	VA_PRINTF;

	LogFile_PrintLine_WithDate (log_file__chat_log_path.c_str(), buffer_string);

	return;
#endif
}

// also perhaps will not be used; chat logging currently takes almost all
void LogFile_Common_Printf (const char* format, ...)
{
#if LOG_COMMON
	VA_PRINTF;

	LogFile_PrintLine_WithDate (log_file__single_log_path.c_str(), buffer_string);

	return;
#endif
}


#pragma once

#include "types.h"
#include "config.h"


#ifndef LOG_COMMON
#error needs to be defined in the config
#endif
#ifndef LOG_CONNECTIONS
#error needs to be defined in the config
#endif
#ifndef LOG_CLIENTS
#error needs to be defined in the config
#endif
#ifndef LOG_CHAT
#error needs to be defined in the config
#endif
#ifndef LOGS_DIR
#error needs to be defined in the config
#endif


// + config +
//#ifndef LOG_CONNECTIONS
extern bool log_connections;
//#endif
//#ifndef LOG_CHAT
extern bool log_chat;
//#endif
// - config -


void LogFile_DefineLogNames (ushort port);
//#if LOG_CLIENTS
void LogFile_Clear ();
void LogFile_Copy ();
//#endif
//extern void LogFile_Print (const char* string);
void LogFile_Clients_Printf (const char* format, ...);
void LogFile_Connections_Printf (const char* format, ...);
void LogFile_Chat_Printf (const char* format, ...);
void LogFile_Common_Printf (const char* format, ...);


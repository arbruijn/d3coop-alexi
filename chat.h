#pragma once

#include "types.h"


void Chat_Init ();
//void Chat_Init (u32 port);
bool Chat_Message_In_Parse (const char* str);
//void Chat_Message_Sending (const char* str);
bool Chat_Message_Sending (const char* str);


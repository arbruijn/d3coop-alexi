//

#include "chat.h"
#include "coop_mod.h"
#include "hooks.h"
//#include "gamedll_header.h"
#include "coop.h"
#include "error.h"
#include "log_file.h"
#include "drop.h"
#include "cmd.h"

// for an incoming messages, the best place to tap is to get them from the incoming packets;
// but for outgoing messages, the situation significantly differs :
// can't take them from the outgoing packets (because of mess with a script messages), but have to get them right after the F8 subr.


//char chat_log_name[256] = {0};


void Chat_Init ()
//void Chat_Init (u32 port)
{
#if 0
	const char* date_month;
	date_month = Utils_GetDateString_Month ();
	sprintf (log_file__single_log_path, "%s" PSEP "common.txt", log_file__log_dir);
	sprintf (log_file__connections_log_path, "%s" PSEP "%s.txt", log_file__log_dir__connections, date_month);
	sprintf (chat_log_name, "%s" PSEP "chat" PSEP "_%u.txt", port);
#endif
//	Cmd_Init ();
}

// return 1, if message is a request to us
bool Chat_Command (u32 plix, const char* str)
{
	bool r;

	if (! local_role_is_server)
		return 0;
// lets take strings from chat, only if they begin with '/'
	if (*str != '/')
		return 0;
	str += 1;
	if (! *str)
	{
		CoopMod_SendMessage (plix, "no command");
		return 1;
	}
// "//" string is not for us
	if (*str == '/')
	{
// *the message is already printed into the log; there it goes unmodified
		strcpy ((char*)str - 1, str);
		return 0;
	}
	r = Cmd_Dispatch (plix, str);
	if (r)
		return 1;
//print_msg:
//	LogFile_Chat_Printf ("%s", str);

//	CoopMod_SendMessage (plix, "command \"%s\" unknown", str);
	return 1;
}

// return 1, if message is a request to us
// I wished to put a message write at the end of this function, but for better verbosity/reliability - I will better put it at the beginning
// five stars "*****" will go for very important notices
// **sometimes after level change, there is no response from dedi server on a commands like "/rn"
bool Chat_Message_In_Parse (const char* str)
{
	bool r;
	int i_r;
	const char* message;
//	char* ptr;
	player_record* pr;
	int len;

// D3 has no protection against multiple players with the same nick, so here could be added "#x " (where x is pnum) before the message
// but I will perhaps leave those strings simplier
	LogFile_Chat_Printf ("%s", str);
	Assert(DMFCBase);

// check on a faked callsign
	pr = DMFCBase->GetPlayerRecordByPnum (hook_store_plix);
	if (! pr)
// something is wrong
	{
		LogFile_Chat_Printf ("***** can't get player record for player %d", hook_store_plix);
//		goto print_msg;
		return 0;
	}
	len = strlen (pr->callsign);
	MAX_CALLSIGN_SIZE;
	i_r = strncmp (str, pr->callsign, len);
// if is does not begin from the player nick - assume it as faked
	if (i_r)
	{
		LogFile_Chat_Printf ("***** faked message from player %d", hook_store_plix);
//		goto print_msg;
		return 0;
	}
//	message += len;
	message = str + len;

// look text string 117 for the all possible options
//	message = strstr (str, " says: ");
	i_r = strncmp (message, " says: ", 7);
	if (! i_r)
		goto says_found;
	i_r = strncmp (message, " sagt: ", 7);
	if (! i_r)
		goto says_found;
	i_r = strncmp (message, " dice: ", 7);
	if (! i_r)
		goto says_found;
	i_r = strncmp (message, " dit : ", 7);
	if (! i_r)
		goto says_found;
	LogFile_Chat_Printf ("***** in message from player %d - \"says:\" was not found", hook_store_plix);
//	goto print_msg;
	return 0;

says_found:
	message += 7;
	r = Chat_Command (hook_store_plix, message);
	if (! r)
		return 0;

//	return r;
//	return 0;
	return 1;
}

//void Chat_Message_Sending (const char* str)
bool Chat_Message_Sending (const char* str)
{
	bool r;
	player_record* pr;

//	if (! chat_log_name[0])
//		return;

	Assert(DMFCBase);
	pr = DMFCBase->GetPlayerRecordByPnum (0);
	if (! pr)
// something is wrong
	{
		LogFile_Chat_Printf ("***** no player record for player 0");
		LogFile_Chat_Printf ("%s", str);
		return 0;
	}
// will be looking different, than "%s says: %s"
	LogFile_Chat_Printf ("%s: %s", pr->callsign, str);
	r = Chat_Command (0, str);
	return r;
}


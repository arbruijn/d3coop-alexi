// server commands
// *not all commans are working without a crash for usage at the client side
// crappy :  command class and also a functions to where it distributes are present here
// crappy :  command type which deals with a client (by ix, id or name) is not in the enum list

#include "commands.h"
#include "coop.h"
#include "coop_mod.h"
#include "coop_mod_cmd.h"
//#include "../osiris/osiris_import.h"
#include "error.h"
#include "nicks.h"
//#include <string>
//#include <sstream>
//using namespace std;
#include "hooks.h"
#include "exceptions.h"
#include "log_file.h"
#include "drop.h"
#include "macros.h"


enum
{
	COMMAND_TYPE__BOOL,
	COMMAND_TYPE__INT,
	COMMAND_TYPE__FLOAT,
// command, without arguments
	COMMAND_TYPE__COMMAND,
// command, with a string as argument(s)
	COMMAND_TYPE__COMMAND_ARGUMENTED,
// command, with an int as argument
	COMMAND_TYPE__COMMAND_INT,
// command, with two ints as an arguments
	COMMAND_TYPE__COMMAND_INT_INT,
// *some player's some info, pnum safe
//	COMMAND_TYPE__SHOW,
	COMMAND_TYPE__SHOW_PLAYER_INFO,
// info from some slot, slot index safe
	COMMAND_TYPE__SHOW_SLOT_INFO,
//	COMMAND_TYPE__SET_PLAYER_DATA,
// pnum safe
	COMMAND_TYPE__CLIENT,
	COMMAND_TYPE__CLIENT__STRING,
	COMMAND_TYPE__CLIENT__INT,
	COMMAND_TYPES_NUM
};

typedef struct
// *arrays for strings aren't necessary here, but I will do it fully
{
// (typing) name of the command
	char name[32];
// string for the D3 engine
//	char string[32];
// informative string, which will be shown every time when this variable will be touched
	char short_info[64];
// full description
	char description[100];
// kind of variable it handles or function it uses
	int type;
	void* variable;
	bool show_value_change_to_all;
	int default_whom;
	void* function;
} command_t;

//#define COMMANDS_MAX 100
#define COMMANDS_MAX 200
command_t commands[COMMANDS_MAX];
u32 commands_num = 0;

//const char* command_cmd = 0;
// *could also be a full cmd name, what is more understandable when reading a printed output
char command_cmd[300] = {0};
char command_args[1000] = {0};

typedef void Func_UpdateVariable_t (void);
typedef void Func_Command_t (void);
typedef void Func_CommandArgumented_t (const char* arguments);
typedef void Func_Command_Int_t (int value);
typedef void Func_Command_IntInt_t (int value0, int value1);
typedef void Func_ShowPlayerInfo_t (int whos, int whom);
typedef void Func_Client_t (player_info_t* player_info);
typedef void Func_Client_String_t (player_info_t* player_info, string str_arg);
typedef void Func_Client_Int_t (player_info_t* player_info, int i_arg);


// *the function args became unnecessary
void Commands_Command (command_t* command, char* string_arguments)
{
	bool r;
	int arg_num;
	bool value_bool;
	int value_int_0;
	int value_int_1;
	float value_float;
//	bool* var_bool = &auto_kick__team_kills;
	bool is_set = 0;
	char string_value_current[100];
	char string_value_previous[100] = {0};
	Func_Command_t* Func_Command = 0;
	Func_CommandArgumented_t* Func_CommandArgumented = 0;
	Func_Command_Int_t* Func_Command_Int = 0;
	Func_Command_IntInt_t* Func_Command_IntInt = 0;
	Func_ShowPlayerInfo_t* Func_ShowPlayerInfo = 0;
	Func_Client_t* Func_Client = 0;
	Func_Client_String_t* Func_Client_String = 0;
	Func_Client_Int_t* Func_Client_Int = 0;
//	string str_args;
	stringstream strm_args;
	stringstream strm;
	string str_arg1;
	string str_arg2;
	player_info_t* player_info;

	if (! command->variable && ! command->function)
	{
		DLLAddHUDMessage ("'Commands_Command  ()' :  no variable and no function for \"%s\" command", command->name);
		return;
	}

//	str_args = string_arguments;
	strm_args << string_arguments;
	switch (command->type)
	{
	case COMMAND_TYPE__BOOL:
		arg_num = sscanf (string_arguments, "%d", &value_int_0);

		if (arg_num > 1)
		{
			DLLAddHUDMessage ("too much arguments for \"%s\" command", command->name);
			break;
		}
		else if (arg_num == 1)
		{
			value_bool = *(bool*)command->variable;
// *0/1 is much better readable than on/off
//			sprintf (string_value_previous, " (was %s)", value_bool ? "on" : "off");
			sprintf (string_value_previous, " (was %u)", value_bool);
			value_bool = !! value_int_0;
			*(bool*)command->variable = value_bool;
			is_set = 1;
		}

//		sprintf (string_value_current, "%s", value_bool ? "on" : "off");
//		sprintf (string_value_current, "%s is %s%s%s", command->short_info, is_set ? "set to " : "", *(bool*)command->variable ? "on" : "off", string_value_previous);
		sprintf (string_value_current, "%s is %s%u%s", command->short_info, is_set ? "set to " : "", *(bool*)command->variable, string_value_previous);
		if (command->show_value_change_to_all)
			CoopMod_SendMessage (-1, "%s", string_value_current);
		else
			DLLAddHUDMessage ("%s", string_value_current);

		break;

	case COMMAND_TYPE__INT:
		arg_num = sscanf (string_arguments, "%d", &value_int_0);

		if (arg_num > 1)
		{
			DLLAddHUDMessage ("too much arguments for \"%s\" command", command->name);
			break;
		}
		else if (arg_num == 1)
		{
			sprintf (string_value_previous, " (was %d)", *(int*)command->variable);
			*(int*)command->variable = value_int_0;
			is_set = 1;
		}

		sprintf (string_value_current, "%s is %s%d%s", command->short_info, is_set ? "set to " : "", *(int*)command->variable, string_value_previous);
		if (command->show_value_change_to_all)
			CoopMod_SendMessage (-1, "%s", string_value_current);
		else
			DLLAddHUDMessage ("%s", string_value_current);

		break;

	case COMMAND_TYPE__FLOAT:
		arg_num = sscanf (string_arguments, "%f", &value_float);

		if (arg_num > 1)
		{
			DLLAddHUDMessage ("too much arguments for \"%s\" command", command->name);
			break;
		}
		else if (arg_num == 1)
		{
			sprintf (string_value_previous, " (was %.1f)", *(float*)command->variable);
			*(float*)command->variable = value_float;
			is_set = 1;
		}

		sprintf (string_value_current, "%s is %s%.1f%s", command->short_info, is_set ? "set to " : "", *(float*)command->variable, string_value_previous);
		if (command->show_value_change_to_all)
			CoopMod_SendMessage (-1, "%s", string_value_current);
		else
			DLLAddHUDMessage ("%s", string_value_current);

		break;

	case COMMAND_TYPE__COMMAND:
		Func_Command = (Func_Command_t*)command->function;
		Func_Command ();
		break;

	case COMMAND_TYPE__COMMAND_ARGUMENTED:
		Func_CommandArgumented = (Func_CommandArgumented_t*)command->function;
		Func_CommandArgumented (string_arguments);
		break;

	case COMMAND_TYPE__COMMAND_INT:
// *lazy now to do it on C++
		arg_num = sscanf (string_arguments, "%d %d", &value_int_0, &value_int_1);

		if (arg_num < 1)
		{
			DLLAddHUDMessage ("not enough arguments for \"%s\" command", command->name);
			break;
		}
		else if (arg_num > 1)
		{
			DLLAddHUDMessage ("too much arguments for \"%s\" command", command->name);
			break;
		}
		Func_Command_Int = (Func_Command_Int_t*)command->function;
		Func_Command_Int (value_int_0);
		break;

	case COMMAND_TYPE__COMMAND_INT_INT:
// *lazy now to do it on C++
		arg_num = sscanf (string_arguments, "%d %d", &value_int_0, &value_int_1);

		if (arg_num < 2)
		{
			DLLAddHUDMessage ("not enough arguments for \"%s\" command", command->name);
			break;
		}
		else if (arg_num > 2)
		{
			DLLAddHUDMessage ("too much arguments for \"%s\" command", command->name);
			break;
		}
		Func_Command_IntInt = (Func_Command_IntInt_t*)command->function;
		Func_Command_IntInt (value_int_0, value_int_1);
		break;

	case COMMAND_TYPE__SHOW_PLAYER_INFO:
	case COMMAND_TYPE__SHOW_SLOT_INFO:
		Func_ShowPlayerInfo = (Func_ShowPlayerInfo_t*)command->function;
		A(Func_ShowPlayerInfo);

		arg_num = sscanf (string_arguments, "%d %d", &value_int_0, &value_int_1);

		if (arg_num < 1)
		{
			DLLAddHUDMessage ("not enough arguments for \"%s\" command", command->name);
			break;
		}
		else if (arg_num > 2)
		{
			DLLAddHUDMessage ("too much arguments for \"%s\" command", command->name);
			break;
		}

		switch (command->type)
		{
		case COMMAND_TYPE__SHOW_PLAYER_INFO:
			if (value_int_0 < 0)
				DLLAddHUDMessage ("\"%s\" command :  arg 1 is negative", command->name);
			if (value_int_0 == 0)
				DLLAddHUDMessage ("\"%s\" command :  arg 1 is zero", command->name);
			if (value_int_0 > DLLMAX_PLAYERS)
				DLLAddHUDMessage ("\"%s\" command :  arg 2 is too high (%d is maximal player number)", command->name, DLLMAX_PLAYERS);
			break;
		case COMMAND_TYPE__SHOW_SLOT_INFO:
			if (value_int_0 < 0)
				DLLAddHUDMessage ("\"%s\" command :  arg 1 is negative", command->name);
			if (value_int_0 >= C_T_STATUSES_NUM)
				DLLAddHUDMessage ("\"%s\" command :  arg 1 is too high (%d slots exist)", command->name, C_T_STATUSES_NUM);
			break;
		default:
			DLLAddHUDMessage ("'Commands_Command  ()' :  wrong type for \"%s\" command", command->name);
		}

		if (arg_num >= 2)
		{
//			if (value_int_1 < 0)
//				DLLAddHUDMessage ("\"%s\" command :  arg 2 is negative", command->name);
			if (value_int_1 > DLLMAX_PLAYERS)
			{
				DLLAddHUDMessage ("\"%s\" command :  arg 2 is too high (%d is maximal player number)", command->name, DLLMAX_PLAYERS);
				break;
			}
		}

		Func_ShowPlayerInfo (value_int_0, (arg_num == 1) ? command->default_whom : value_int_1);

		break;

	case COMMAND_TYPE__CLIENT:
	case COMMAND_TYPE__CLIENT__STRING:
	case COMMAND_TYPE__CLIENT__INT:
// client - always as string
		ws(strm_args);
		if (strm_args.eof())
		{
			DLLAddHUDMessage ("\"%s\" command :  no arg 1", command->name);
			break;
		}
		strm_args >> str_arg1;
//		if (strm_args.fail() || ! strm_args.eof())
		if (strm_args.fail())
//		if (strm_args.eof())
		{
			DLLAddHUDMessage ("\"%s\" command :  arg 1 failed", command->name);
			break;
		}
		ws(strm_args);
#if 0
		strm_args >> str_arg2;
		if (strm_args.fail())
		{
			DLLAddHUDMessage ("\"%s\" command :  no arg 2", command->name);
			break;
		}
		ws(strm_args);
		if (! strm_args.eof())
		{
			DLLAddHUDMessage ("\"%s\" command :  too many args; need only 2", command->name);
			break;
		}
#endif
		if (command->type == COMMAND_TYPE__CLIENT)
		{
			if (! strm_args.eof())
			{
				DLLAddHUDMessage ("\"%s\" command :  need only 1 arg", command->name, string_arguments);
				break;
			}
//			goto call_client_func;
			goto get_player_info;
		}
		getline(strm_args, str_arg2);
		if (! str_arg2.length())
		{
			DLLAddHUDMessage ("\"%s\" command :  no arg 2", command->name);
			break;
		}

get_player_info:
		r = CoopMod_TeammateStatus_Get (&player_info, str_arg1, "Command");
		if (! r)
			break;

//call_client_func:
		switch (command->type)
		{
		case COMMAND_TYPE__CLIENT:
			Func_Client = (Func_Client_t*)command->function;
			Func_Client (player_info);
			break;
		case COMMAND_TYPE__CLIENT__STRING:
			Func_Client_String = (Func_Client_String_t*)command->function;
			Func_Client_String (player_info, str_arg2);
			break;
		case COMMAND_TYPE__CLIENT__INT:
			strm << str_arg2;
			strm >> value_int_0;
			if (! strm.eof())
			{
				DLLAddHUDMessage ("\"%s\" command :  failed to convert arg \"%s\" to int", command->name, str_arg2);
				break;
			}
			Func_Client_Int = (Func_Client_Int_t*)command->function;
			Func_Client_Int (player_info, value_int_0);
			break;
		default:
			DLLAddHUDMessage ("'Commands_Command ()' :  wrong command type (%d)", command->type);
		}
		break;

	default:
		DLLAddHUDMessage ("'Commands_Command ()' :  command type (%d) is unknown", command->type);
		break;
	}
}

void Commands_Dispatcher (char* full_string)
{
	command_t* command;
//	int i;
	int arg_num;
//	int value;

	A(commands_num <= COMMANDS_MAX);

	if (*full_string != '$')
	{
		DLLAddHUDMessage ("'Commands_Dispatcher ()' :  no '$' sign in string \"%s\"", command_cmd);
		return;
	}

// *can specify the size of the buffer, but only statically, like "%80s" (need 81 char buffer for that)
	arg_num = sscanf (full_string, "$%s %[^\0]s", command_cmd, command_args);

	if (arg_num < 1)
	{
		DLLAddHUDMessage ("'Commands_Dispatcher ()' :  no arguments");
		return;
	}
	if (arg_num < 2)
		*command_args = 0;

//	for (i = 0; i < commands_num; i++)
	FORI0(commands_num)
	{
		command = &commands[i];

		if (! _stricmp (command_cmd, (const char*)command->name))
		{
			Commands_Command (command, command_args);
//			break;
			return;
		}
	}

	DLLAddHUDMessage ("'Commands_Dispatcher ()' :  command \"%s\" is not found", command_cmd);

//	command_cmd = 0;
	command_cmd[0] = 0;
	command_args[0] = 0;
}


// + command register +

static void Commands_Register (command_t* command)
{
	if (commands_num >= COMMANDS_MAX)
		Error ("Commands_Register () :  'commands_num' has reached 'COMMANDS_MAX' (%u)", COMMANDS_MAX);

	commands[commands_num] = *command;
	DMFCBase->AddInputCommand (command->name, command->description, Commands_Dispatcher, true);
	commands_num++;
}

void Commands_Register_Bool (char* name, char* short_info, char* description, bool* variable, Func_UpdateVariable_t* function = 0, bool show_value_change_to_all = 0)
{
command_t command;

	command.type = COMMAND_TYPE__BOOL;
	strcpy (command.name, name);
	strcpy (command.short_info, short_info);
	strcpy (command.description, description);
	command.variable = variable;
	command.show_value_change_to_all = show_value_change_to_all;
	command.function = function;

	Commands_Register (&command);
}

void Commands_Register_Int (char* name, char* short_info, char* description, int* variable, Func_UpdateVariable_t* function = 0, bool show_value_change_to_all = 0)
{
command_t command;

	command.type = COMMAND_TYPE__INT;
	strcpy (command.name, name);
	strcpy (command.short_info, short_info);
	strcpy (command.description, description);
	command.variable = variable;
	command.show_value_change_to_all = show_value_change_to_all;
	command.function = function;

	Commands_Register (&command);
}

void Commands_Register_Float (char* name, char* short_info, char* description, float* variable, Func_UpdateVariable_t* function = 0, bool show_value_change_to_all = 0)
{
command_t command;

	command.type = COMMAND_TYPE__FLOAT;
	strcpy (command.name, name);
	strcpy (command.short_info, short_info);
	strcpy (command.description, description);
	command.variable = variable;
	command.show_value_change_to_all = show_value_change_to_all;
	command.function = function;

	Commands_Register (&command);
}

void Commands_Register_Command (char* name, char* description, Func_Command_t* function)
{
command_t command;

	command.type = COMMAND_TYPE__COMMAND;
	strcpy (command.name, name);
//	strcpy (command.short_info, short_info);
	command.short_info[0] = 0;
	strcpy (command.description, description);
	command.variable = 0;
	command.function = function;

	Commands_Register (&command);
}

void Commands_Register_CommandArgumented (char* name, char* description, Func_CommandArgumented_t* function)
{
command_t command;

	command.type = COMMAND_TYPE__COMMAND_ARGUMENTED;
	strcpy (command.name, name);
//	strcpy (command.short_info, short_info);
	command.short_info[0] = 0;
	strcpy (command.description, description);
	command.variable = 0;
	command.function = function;

	Commands_Register (&command);
}

void Commands_Register_Command_Int (char* name, char* description, Func_Command_Int_t* function)
{
command_t command;

	command.type = COMMAND_TYPE__COMMAND_INT;
	strcpy (command.name, name);
//	strcpy (command.short_info, short_info);
	command.short_info[0] = 0;
	strcpy (command.description, description);
	command.variable = 0;
	command.function = function;

	Commands_Register (&command);
}

void Commands_Register_Command_IntInt (char* name, char* description, Func_Command_IntInt_t* function)
{
command_t command;

	command.type = COMMAND_TYPE__COMMAND_INT_INT;
	strcpy (command.name, name);
//	strcpy (command.short_info, short_info);
	command.short_info[0] = 0;
	strcpy (command.description, description);
	command.variable = 0;
	command.function = function;

	Commands_Register (&command);
}

void Commands_Register_ShowPlayerInfo (char* name, char* description, Func_ShowPlayerInfo_t* function, int default_whom)
{
command_t command;

	command.type = COMMAND_TYPE__SHOW_PLAYER_INFO;
	strcpy (command.name, name);
//	strcpy (command.short_info, short_info);
	command.short_info[0] = 0;
	strcpy (command.description, description);
	command.variable = 0;
	command.default_whom = default_whom;
	command.function = function;

	Commands_Register (&command);
}

void Commands_Register_ShowSlotInfo (char* name, char* description, Func_ShowPlayerInfo_t* function, int default_whom)
{
	command_t command;

	command.type = COMMAND_TYPE__SHOW_SLOT_INFO;
	strcpy (command.name, name);
//	strcpy (command.short_info, short_info);
	command.short_info[0] = 0;
	strcpy (command.description, description);
	command.variable = 0;
	command.default_whom = default_whom;
	command.function = function;

	Commands_Register (&command);
}

void Commands_Register_Client (char* name, char* description, Func_Client_t* function)
{
	command_t command;

	command.type = COMMAND_TYPE__CLIENT;
	strcpy (command.name, name);
	command.short_info[0] = 0;
	strcpy (command.description, description);
	command.variable = 0;
	command.function = function;

	Commands_Register (&command);
}

void Commands_Register_Client_String (char* name, char* description, Func_Client_String_t* function)
{
	command_t command;

	command.type = COMMAND_TYPE__CLIENT__STRING;
	strcpy (command.name, name);
	command.short_info[0] = 0;
	strcpy (command.description, description);
	command.variable = 0;
	command.function = function;

	Commands_Register (&command);
}

void Commands_Register_Client_Int (char* name, char* description, Func_Client_Int_t* function)
{
	command_t command;

	command.type = COMMAND_TYPE__CLIENT__INT;
	strcpy (command.name, name);
	command.short_info[0] = 0;
	strcpy (command.description, description);
	command.variable = 0;
	command.function = function;

	Commands_Register (&command);
}

// - command register -


// list all registered commands
void Commands_List ()
{
	static char buffer[256];
	u32 i;
	command_t* cmd;
	u32 counter;

	buffer[0] = 0;
	counter = 0;
//	for (i = 0; i < commands_num; i++)
	i = 0;
	DLLAddHUDMessage ("Server reegistered commands list :");
//	while (1)
//	{
loop1:
	if (counter >= 4)
// collected enough for one print (current text lines volume is crappy small)
	{
		DLLAddHUDMessage ("%s", buffer);
		counter = 0;
		buffer[0] = 0;
	}
	if (counter)
		strcat (buffer, " ");
	cmd = &commands[i];
	strcat (buffer, cmd->name);
//	if (counter)
//		strcat (buffer, ",");
	i++;
	counter++;
	if (i < commands_num)
	{
		strcat (buffer, ",");
		goto loop1;
	}
	else
	{
		strcat (buffer, ".");
//		break;
	}
//	}
	if (counter)
		DLLAddHUDMessage ("%s", buffer);
}


// + main +

void Commands_TeamDamage (char* full_string)
{
char command_name[] = "TeamDamage";
int arg_num;
int value0;

	arg_num = sscanf (full_string, "%*s %d", &value0);

	if (arg_num < 1)
		CoopMod_TeamDamage (0, 0);
	else if (arg_num > 1)
		DLLAddHUDMessage ("too much arguments for \"%s\" command", command_name);
	else
		CoopMod_TeamDamage (1, !! value0);
}

// - main -


// + auxilary +

void Commands_Sleep (char* full_string)
{
char command_name[] = "Sleep";
int arg_num;
int value0;

	arg_num = sscanf (full_string, "%*s %d", &value0);

	if (arg_num < 1)
		CoopMod_SetSleep (0, 0);
	else if (arg_num > 1)
		DLLAddHUDMessage ("too much arguments for \"%s\" command", command_name);
	else
		CoopMod_SetSleep (1, value0);
}

// - auxilary -



// + players management +

void Commands_Kill (char* full_string)
{
char command_name[] = "Kill";
int arg_num;
int value;
char string[1000];

	arg_num = sscanf (full_string, "%*s %d %s", &value, string);

	if (arg_num < 1)
		DLLAddHUDMessage ("not enough arguments for \"%s\" command", command_name);
	else if (arg_num > 2)
		DLLAddHUDMessage ("too much arguments for \"%s\" command", command_name);
	else
		CoopMod_Kill (value, (arg_num == 1) ? NULL : string);
}

void Commands_Kick (char* full_string)
{
char command_name[] = "KickM";
int arg_num;
int value;
char string[1000];

//	arg_num = sscanf (full_string, "%*s %d %[ a-z^\"]s", &value, string);
	arg_num = sscanf (full_string, "%*s %d %[^\0]s", &value, string);

	if (arg_num < 1)
		DLLAddHUDMessage ("not enough arguments for \"%s\" command", command_name);
	else if (arg_num > 2)
		DLLAddHUDMessage ("too much arguments for \"%s\" command", command_name);
	else
		CoopMod_Kick (value, 1, (arg_num == 1) ? NULL : string);
}

void Commands_UnKick (char* full_string)
{
char command_name[] = "UnKick";
int arg_num;
int value0;

	arg_num = sscanf (full_string, "%*s %d", &value0);

	if (arg_num > 1)
		DLLAddHUDMessage ("too much arguments for \"%s\" command", command_name);
	else
	{
		if (arg_num)
			CoopMod_UnKick (value0);
		else
			CoopMod_UnKickAll ();
	}
}

// - players management -


// some commands have a duplicate, for fast typing
void Commands_Init ()
{
	commands_num = 0;

	Commands_Register_Command ("List", "list all registered commands", Commands_List);

// dummies
#if 0
	Commands_Register_Int (char* name, char* short_info, char* description, int* variable, Func_UpdateVariable_t* function = 0, bool show_value_change_to_all = 0);
	Commands_Register_Int ("", "", "", &);
#endif

// + main +

	DMFCBase->AddInputCommand ("TeamDamage", "team damage", Commands_TeamDamage, true);

	Commands_Register_Bool ("AutoKick_TeamKills", "autokick for team kills", "switch autokick for teamkillers on/off", &auto_kick__team_kills, 0, 1);
	Commands_Register_Bool ("AKTK", "autokick for team kills", "switch autokick for teamkillers on/off", &auto_kick__team_kills, 0, 1);

	Commands_Register_Bool ("AutoKick_TeamDamageLong", "autokick for constant team attackers", "switch autokick for constant team attackers on/off", &auto_kick__team_damage_long, 0, 1);
	Commands_Register_Bool ("AKTDL", "autokick for constant team attackers", "switch autokick for constant team attackers on/off", &auto_kick__team_damage_long, 0, 1);

// yes, it is a main thing for the D3 :/
	Commands_Register_Bool ("exceptions", "exceptions handling", "exceptions handling on/off", &exceptions_catch, 0);

// yes main, because it is used everywhere
	Commands_Register_Float ("KickDelay", "kick delay time", "kick delay time", &kick_time_delay, 0);

// also main !  it is logging !  the mostly important part of any server
// *must report about change of these vars, but I don't care
	Commands_Register_Bool ("LogConnections", "log connections", "log connections on/off", &log_connections, 0);
	Commands_Register_Bool ("LogChat", "log chat", "log chat on/off", &log_chat, 0);

	Commands_Register_Bool ("Drop", "drop", "drop on/off", &drop, 0);
	Commands_Register_Bool ("DropWeapon", "drop weapon", "drop weapon on/off", &drop_weapon, 0);

	Commands_Register_Bool ("RobotsEndLevel", "level end on robots destroy", "destroying all robots does end the level", &coop_mod_endlevel_on_0_robots, 0, 1);
	Commands_Register_Bool ("REL", "level end on robots destroy", "destroying all robots does end the level", &coop_mod_endlevel_on_0_robots, 0, 1);

// - main -


// + team kills +

//	DMFCBase->AddInputCommand ("TeamKillsMax", "coop_team_kills_max", Commands_TeamKillsMax, true);
	Commands_Register_Int ("TeamKillsMax", "coop_team_kills_max", "maximum amount of allowed teamkills", &coop_team_kills_max, 0);
	
//	DMFCBase->AddInputCommand ("TeamKillsPeriod", "coop_team_kills_period", Commands_TeamKillsPeriod, true);
	Commands_Register_Float ("TeamKillsPeriod", "coop_team_kills_period", "period for allowed teamkills", &coop_team_kills_period, 0);

// - team kills -


// + team damage long +

//	DMFCBase->AddInputCommand ("TeamDamageLongMin", "coop_teamdamage_long_min", Commands_TeamDamageLongMin, true);
//	DMFCBase->AddInputCommand ("TDLMin", "coop_teamdamage_long_min", Commands_TeamDamageLongMin, true);
	Commands_Register_Int ("TeamDamageLongMin", "coop_teamdamage_long_min", "coop_teamdamage_long_min", &coop_teamdamage_long_min);
	Commands_Register_Int ("TDLMin", "coop_teamdamage_long_min", "coop_teamdamage_long_min", &coop_teamdamage_long_min);

//	DMFCBase->AddInputCommand ("TeamDamageLongMax", "coop_teamdamage_long_max", Commands_TeamDamageLongMax, true);
//	DMFCBase->AddInputCommand ("TDLMax", "coop_teamdamage_long_max", Commands_TeamDamageLongMax, true);
	Commands_Register_Int ("TeamDamageLongMax", "coop_teamdamage_long_max", "coop_teamdamage_long_max", &coop_teamdamage_long_max, 0);
	Commands_Register_Int ("TDLMax", "coop_teamdamage_long_max", "coop_teamdamage_long_max", &coop_teamdamage_long_max, 0);

//	DMFCBase->AddInputCommand ("TeamDamageLongPeriod", "coop_teamdamage_long_period", Commands_TeamDamageLongPeriod, true);
//	DMFCBase->AddInputCommand ("TDLPeriod", "coop_teamdamage_long_period", Commands_TeamDamageLongPeriod, true);
	Commands_Register_Float ("TeamDamageLongPeriod", "coop_teamdamage_long_period", "coop_teamdamage_long_period", &coop_teamdamage_long_period, 0);
	Commands_Register_Float ("TDLPeriod", "coop_teamdamage_long_period", "coop_teamdamage_long_period", &coop_teamdamage_long_period, 0);

//	DMFCBase->AddInputCommand ("TeamDamageLong_RobotDamagePostPeriod", "coop_teamdamage_robotdamage_post_time", Commands_TeamDamageLong_RobotDamagePostPeriod, true);
//	DMFCBase->AddInputCommand ("TDL_RDPP", "coop_teamdamage_robotdamage_post_time", Commands_TeamDamageLong_RobotDamagePostPeriod, true);
	Commands_Register_Float ("TeamDamageLong_RobotDamagePostPeriod", "coop_teamdamage_robotdamage_post_time", "coop_teamdamage_robotdamage_post_time", &coop_teamdamage_robotdamage_post_time, 0);
	Commands_Register_Float ("TDL_RDPP", "coop_teamdamage_robotdamage_post_time", "coop_teamdamage_robotdamage_post_time", &coop_teamdamage_robotdamage_post_time, 0);

// - team damage long -


// + auxilary +

	Commands_Register_CommandArgumented ("SetMacro", "set macro", CoopMod_SetMacro);
	Commands_Register_CommandArgumented ("SM", "set macro", CoopMod_SetMacro);

// *not finished, cause no source to do this
//	DMFCBase->AddInputCommand ("PowerupsDeleteAll", "intended for full level objects flush", Commands_PowerupsDeleteAll, true);
//	DMFCBase->AddInputCommand ("PFlush", "intended for full level objects flush", Commands_PowerupsDeleteAll, true);
	Commands_Register_Command ("PowerupsDeleteAll", "intended for full level objects flush", CoopMod_PowerupsDeleteAll);
	Commands_Register_Command ("PFlush", "intended for full level objects flush", CoopMod_PowerupsDeleteAll);

	DMFCBase->AddInputCommand ("Sleep", "additional sleep time for every frame", Commands_Sleep, true);

// instruction, how to use it :  input "$SOI 1", touch an object, input "$SOI 0", look its handle, input this command with that handle
//	DMFCBase->AddInputCommand ("MoveObject", "move stated object to the center of the stated room", Commands_MoveObject, true);
//	DMFCBase->AddInputCommand ("MObj", "move stated object to the center of the stated room", Commands_MoveObject, true);
	Commands_Register_Command_IntInt ("MoveObject", "move stated object to the center of the stated room", CoopMod_MoveObject);
	Commands_Register_Command_IntInt ("MObj", "move stated object to the center of the stated room", CoopMod_MoveObject);
//	DMFCBase->AddInputCommand ("MovePlayer", "move stated player to the center of the stated room", Commands_MovePlayer, true);
//	DMFCBase->AddInputCommand ("MPlr", "move stated player to the center of the stated room", Commands_MovePlayer, true);
	Commands_Register_Command_IntInt ("MovePlayer", "move stated player to the center of the stated room", CoopMod_MovePlayer);
	Commands_Register_Command_IntInt ("MPlr", "move stated player to the center of the stated room", CoopMod_MovePlayer);
	Commands_Register_Command_Int ("MovePlayersAll", "move stated player to the center of the stated room", CoopMod_MovePlayersAll);
	Commands_Register_Command_Int ("MPA", "move stated player to the center of the stated room", CoopMod_MovePlayersAll);
	Commands_Register_Command_IntInt ("FindObj", "find an object", CoopMod_FindObj);
	Commands_Register_Command_IntInt ("FO", "find an object", CoopMod_FindObj);
	Commands_Register_Command_Int ("FindRobot", "find a robot", CoopMod_FindRobot);
	Commands_Register_Command_Int ("FR", "find a robot", CoopMod_FindRobot);

// *handle or ix - doesn't matter
	Commands_Register_Int ("DumpObjIx", "index of an object to dump", "index of an object to dump", &coopmod_dump_obj_handle, 0);
	Commands_Register_Int ("DOI", "index of an object to dump", "index of an object to dump", &coopmod_dump_obj_handle, 0);
	Commands_Register_Command ("DumpObj", "dump an object structure into a file", CoopMod_DumpObj);
	Commands_Register_Command ("DO", "dump an object structure into a file", CoopMod_DumpObj);
	Commands_Register_Command ("DumpObjAI", "dump an object structure AI into a file", CoopMod_DumpObjAI);
	Commands_Register_Command ("DOAI", "dump an object structure AI into a file", CoopMod_DumpObjAI);

//	DMFCBase->AddInputCommand ("AntiGuideBotMode", "index of an antiguidebot mode", Commands_AntiGuideBotMode, true);
//	DMFCBase->AddInputCommand ("AGBM", "index of an antiguidebot mode", Commands_AntiGuideBotMode, true);
// script stuff (still not helps against crashes)
	Commands_Register_Int ("AntiGuideBotMode", "coop_anti_guidebot_mode", "index of the antiguidebot mode", &coop_anti_guidebot_mode, 0);
	Commands_Register_Int ("AGBM", "coop_anti_guidebot_mode", "index of the antiguidebot mode", &coop_anti_guidebot_mode, 0);
// incoming network packets (helps against crashes produced by connected clients, but not when the nondedi server player #0 presses F4)
	Commands_Register_Bool ("GuideBotBlock", "block guidebot", "block guidebot release", &hook_block_guidebot_packet, 0, 1);
	Commands_Register_Bool ("GBB", "block guidebot", "block guidebot release", &hook_block_guidebot_packet, 0, 1);
// *note, it doesn't show all traffic, but only which is going through 559120 subr
	Commands_Register_Bool ("ShowSocketTraffic", "socket traffic", "show socket traffic", &hooks_indicate__socket_traffic, 0, 0);
	Commands_Register_Bool ("SST", "socket traffic", "show socket traffic", &hooks_indicate__socket_traffic, 0, 0);
	Commands_Register_Bool ("SendPackets5e", "send packets ourselves", "send packets 5e ourselves", &hooks_send_packets_5e, 0, 1);
	Commands_Register_Bool ("SP5e", "send packets ourselves", "send packets 5e ourselves", &hooks_send_packets_5e, 0, 1);
	Commands_Register_Bool ("PPSMinKick", "kick on PPS min", "kick on violated PPS min", &coop_pps_min_do_kick, 0, 1);

//	DMFCBase->AddInputCommand ("PPSMin", "minimal PPS allowed", Commands_PPSMin, true);
	Commands_Register_Int ("PPSMin", "coop_pps_min", "minimal PPS allowed", &coop_pps_min);

// the heck, I begin very dislike this command; it is not necessary so much anymore, after drop-shield was implemented;
// perhaps it should be limited, if there are more than one player in the game; maybe remember the old shield value, and then restore it, if some one more joins the game
//	DMFCBase->AddInputCommand ("SetShield", "set shield value for a client", Commands_SetShield, true);
//	Commands_Register_Client_String ("test", "set shield value for a client", Commands_SetShield_S);
	Commands_Register_Command_IntInt ("ObjectShield", "set shield value for an object", CoopMod_ObjectShield);
	Commands_Register_Command_IntInt ("OS", "set shield value for an object", CoopMod_ObjectShield);
// *will work not always, because it is a game spoil cheat
	Commands_Register_Client_Int ("PlayerShield", "set shield value for a client", CoopMod_PlayerShield);
	Commands_Register_Client_Int ("PS", "set shield value for a client", CoopMod_PlayerShield);
// *this command is intended to work in any conditions
	Commands_Register_Client ("PlayerShieldNormalize", "set shield value for a client", CoopMod_PlayerShieldNormalize);
	Commands_Register_Client ("PSN", "set shield value for a client", CoopMod_PlayerShieldNormalize);
	Commands_Register_Client_Int ("PlayerEnergy", "set energy value for a client", CoopMod_PlayerEnergy);
	Commands_Register_Client_Int ("PE", "set energy value for a client", CoopMod_PlayerEnergy);
	Commands_Register_Client_Int ("PlayerAmmo", "set ammo value for a client", CoopMod_PlayerAmmo);
	Commands_Register_Client_Int ("PA", "set ammo value for a client", CoopMod_PlayerAmmo);
	Commands_Register_Client_Int ("PlayerMissiles", "set missiles value for a client", CoopMod_PlayerMissiles);
	Commands_Register_Client_Int ("PM", "set missiles value for a client", CoopMod_PlayerMissiles);
	Commands_Register_Command_Int ("CoopPPS", "packets per second", CoopMod_SetPPS);
	Commands_Register_Command_Int ("CPPS", "packets per second", CoopMod_SetPPS);
	Commands_Register_Command_IntInt ("ObjectFlagClear", "flags of an object", CoopMod_ObjectFlag_Clear);
	Commands_Register_Command_IntInt ("OFC", "flags of an object", CoopMod_ObjectFlag_Clear);
	Commands_Register_Command_IntInt ("ObjectFlagSet", "flags of an object", CoopMod_ObjectFlag_Set);
	Commands_Register_Command_IntInt ("OFS", "flags of an object", CoopMod_ObjectFlag_Set);

//	Commands_Register_Command_IntBool ("ObjectAIFlags", "AI flags of an object", CoopMod_ObjectRebel);
	Commands_Register_Command_IntInt ("ObjectAIType", "AI type of an object", CoopMod_ObjectModifyAIType);
	Commands_Register_Command_IntInt ("OAIT", "AI type of an object", CoopMod_ObjectModifyAIType);
	Commands_Register_Command_IntInt ("ObjectAIFlagClear", "AI flags of an object", CoopMod_ObjectAIFlag_Clear);
	Commands_Register_Command_IntInt ("OAIFC", "AI flags of an object", CoopMod_ObjectAIFlag_Clear);
	Commands_Register_Command_IntInt ("ObjectAIFlagSet", "AI flags of an object", CoopMod_ObjectAIFlag_Set);
	Commands_Register_Command_IntInt ("OAIFS", "AI flags of an object", CoopMod_ObjectAIFlag_Set);
	Commands_Register_Command_IntInt ("ObjectAIFlagRebel", "AI flags of an object", CoopMod_ObjectRebel);
	Commands_Register_Command_IntInt ("OAIFR", "AI flags of an object", CoopMod_ObjectRebel);
	Commands_Register_Command_IntInt ("ObjectAIFlagHostile", "AI flags of an object", CoopMod_ObjectHostile);
	Commands_Register_Command_IntInt ("OAIFH", "AI flags of an object", CoopMod_ObjectHostile);
	Commands_Register_Command_IntInt ("ObjectAIFlagFire", "AI flags of an object", CoopMod_ObjectFire);
	Commands_Register_Command_IntInt ("OAIFF", "AI flags of an object", CoopMod_ObjectFire);
//	Commands_Register_Command_Int ("ObjectShowBeam", "show an object", CoopMod_ObjectShowBeam);
//	Commands_Register_Command_Int ("OSB", "show an object", CoopMod_ObjectShowBeam);

	Commands_Register_Float ("GameSpeed", "game speed distortion", "game speed distortion", &coopmod_time_distortion, 0, 1);

// - auxilary -


// + indication single +

//	Commands_Register_ShowPlayerInfo ("DamageStatusTotally", "damage status totally", "show status of the total damage made to team and to robots", CoopMod_DamageStatusTotally);
//	Commands_Register_ShowPlayerInfo ("DST", "damage status totally", "show status of the total damage made to team and to robots", CoopMod_DamageStatusTotally);
	Commands_Register_ShowPlayerInfo ("DamageStatusTotally", "show status of the total damage made to team and to robots", CoopMod_DamageStatusTotally, -1);
	Commands_Register_ShowPlayerInfo ("DST", "show status of the total damage made to team and to robots", CoopMod_DamageStatusTotally, -1);

//	Commands_Register_ShowSlotInfo ("DamageStatusTotallyIndexed", "damage status totally indexed", "show status of the total damage made to team and to robots", CoopMod_DamageStatusTotallyIndexed);
//	Commands_Register_ShowSlotInfo ("DSTI", "damage status totally indexed", "show status of the total damage made to team and to robots", CoopMod_DamageStatusTotallyIndexed);
	Commands_Register_ShowSlotInfo ("DamageStatusTotallyIndexed", "show status of the total damage made to team and to robots", CoopMod_DamageStatusTotallyIndexed, 0);
	Commands_Register_ShowSlotInfo ("DSTI", "show status of the total damage made to team and to robots", CoopMod_DamageStatusTotallyIndexed, 0);

// why I have delayed creation of this command for so long :)
	Commands_Register_ShowSlotInfo ("PlayerInfoIndexed", "show main information of a player", CoopMod_PlayerInfoIndexed, 0);
	Commands_Register_ShowSlotInfo ("PII", "show main information of a player", CoopMod_PlayerInfoIndexed, 0);

//	Commands_Register_ShowPlayerInfo ("ShowPPS", "show player's PPS", "show PPS setting of some player", CoopMod_ShowPlayerPPS);
//	Commands_Register_ShowPlayerInfo ("SPPS", "show player's PPS", "show PPS setting of some player", CoopMod_ShowPlayerPPS);
	Commands_Register_ShowPlayerInfo ("ShowPPS", "show PPS setting of some player", CoopMod_ShowPlayerPPS, 0);
	Commands_Register_ShowPlayerInfo ("SPPS", "show PPS setting of some player", CoopMod_ShowPlayerPPS, 0);

	Commands_Register_Command ("MissionInfo", "show mission info", CoopMod_ShowMissionInfo);
	Commands_Register_Command ("Mission", "show mission info", CoopMod_ShowMissionInfo);

	Commands_Register_CommandArgumented ("GeoIP", "produce geoip result for an ip", CoopMod_ShowGeoip);

// - indication single -


// + indication static +

	Commands_Register_Bool ("ShowClientIP", "show client IP", "show IP of connecting client", &show_client_IP, 0);
	Commands_Register_Bool ("SCIP", "show client IP", "show IP of connecting client", &show_client_IP, 0);

	Commands_Register_Bool ("ShowConnectionActivity", "show connection activity", "show IP of connection activating client", &show_connection_activity, 0);
	Commands_Register_Bool ("SCA", "show connection activity", "show IP of connection activating client", &show_connection_activity, 0);

	Commands_Register_Bool ("ShowConnectionActivityPacket", "show connection activity packet", "show IP of incoming packet from a connection activating client", &show_connection_activity_packet, 0);
	Commands_Register_Bool ("SCAP", "show connection activity packet", "show IP of incoming packet from a connection activating client", &show_connection_activity_packet, 0);

	Commands_Register_Bool ("ShowRoomNum", "show room number", "show room number for debug purposes", &show_room_num, 0);
	Commands_Register_Bool ("SRN", "show room number", "show room number for debug purposes", &show_room_num, 0);

	Commands_Register_Bool ("ShowObjectInfo", "show touching object info", "show touching object info for debug purposes", &show_object_info_on_collide, 0, 1);
	Commands_Register_Bool ("SOI", "show touching object info", "show touching object info for debug purposes", &show_object_info_on_collide, 0, 1);

// - indication static -

	Commands_Register_Bool ("CrashyPowerupsBlock", "block crashy powerups", "block crashy powerups picking up", &crashy_powerups_block, 0);
	Commands_Register_Bool ("CPB", "block crashy powerups", "block crashy powerups picking up", &crashy_powerups_block, 0);

	Commands_Register_Bool ("CrashyPowerupsMove", "move crashy powerups", "move crashy powerups somewhere out", &crashy_powerups_move, 0);
	Commands_Register_Bool ("CPM", "move crashy powerups", "move crashy powerups somewhere out", &crashy_powerups_move, 0);


// + players management +

	DMFCBase->AddInputCommand ("Kill", "kill some player", Commands_Kill, true);

	DMFCBase->AddInputCommand ("KickM", "kick some player", Commands_Kick, true);

	DMFCBase->AddInputCommand ("UnKick", "abort kick sequence for all players", Commands_UnKick, true);
	DMFCBase->AddInputCommand ("UK", "abort kick sequence for all players", Commands_UnKick, true);
	DMFCBase->AddInputCommand ("KA", "abort kick sequence for all players", Commands_UnKick, true);

// - players management -


// + nicks +

	Commands_Register_Bool ("NicksCheck", "nicks checking", "check nick for a connecting player", &coop_nick_check, 0, 1);
	Commands_Register_Bool ("NC", "nicks checking", "check nick for a connecting player", &coop_nick_check, 0, 1);

	Commands_Register_Int ("NicksSpacesMax", "nicks spaces max", "max spaces allowed in a connecting player nick", &nicks_spaces_max, 0, 1);
	Commands_Register_Int ("NSM", "nicks spaces max", "max spaces allowed in a connecting player nick", &nicks_spaces_max, 0, 1);

// - nicks -

// + game controls +

// this actually isn't gameplay command, it is a management command, and should not be used, when are other players on the server
//	Commands_Register_CommandArgumented ("Spawn", "spawn an object", CoopMod_Spawn);
// fast test
//	Commands_Register_CommandArgumented ("S", "spawn an object", CoopMod_Spawn);
	Commands_Register_CommandArgumented ("SpawnInFront", "spawn an object in front of you", CoopMod_SpawnInFront);
	Commands_Register_CommandArgumented ("SIF", "spawn an object in front of you", CoopMod_SpawnInFront);
// temp thing; shouldn't exist in a real game at all, because some admins will abuse it
//	Commands_Register_Command ("A", "add an object", CoopMod_AddWeapon);

// *are implemented into a normal shape - "/cmd"
#if 0
	Commands_Register_Command ("DropShield", "drop a shield ball", Drop_Shield);
	Commands_Register_Command ("DS", "drop a shield ball", Drop_Shield);
	Commands_Register_Command ("DropEnergy", "drop an energy ball", Drop_Energy);
	Commands_Register_Command ("DE", "drop an energy ball", Drop_Energy);
	Commands_Register_Command ("DropWeaponPrimary", "drop primary weapon", Drop_WeaponPrimary);
	Commands_Register_Command ("DWP", "drop primary weapon", Drop_WeaponPrimary);
	Commands_Register_Command ("DropWeaponSecondary", "drop secondary weapon", Drop_WeaponSecondary);
	Commands_Register_Command ("DWS", "drop secondary weapon", Drop_WeaponSecondary);
#endif

// - game controls -
}

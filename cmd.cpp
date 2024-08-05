// public commands for service and gameplay

#include "cmd.h"
//using namespace std;
#include "coop_mod.h"
#include "coop.h"
#include "error.h"
#include "cmd_show.h"
#include "string2.h"
#include "log_file.h"
// MSVC 6.0
#if _MSC_VER <= 1200
#pragma warning(disable:4786)
#endif
#include <vector>
// the heck, vector<> conflicts with D3; will have to type 'std::' each time
//using namespace std;


std::vector<cmd_t> cmds;


// have to wrap :/ , 'cmds.push_back({CMD_TYPE__CMD, 0, 0});' does not work
//// *have actually do a wrap for each type; haven't done atm
//void Cmd_PushBack (u32 type, const char* name, void* func_ptr)
#if 1
template <u32 _type, typename _Func_Type>
void TCmd_Add (const char* name, _Func_Type* func_ptr, const char* description)
{
	cmd_t cmd;

// atm need all params
	Assert(_type);
	Assert(name);
	Assert(func_ptr);
	cmd.type = _type;
	cmd.name = mklower(name);
	if (description)
		cmd.description = description;
	cmd.func = func_ptr;
	cmds.push_back(cmd);
}
#endif

void Cmd_Help (u32 pnum)
{
	std::vector<cmd_t>::iterator it;
	std::vector<cmd_t>::iterator it_end;
	u32 c;
	string buf;

	it = cmds.begin();
	it_end = cmds.end();
	c = 0;
	CoopMod_SendMessage (pnum, "User commands are :");
	while (it != it_end)
	{
		if (c)
		{
			buf += ',';
// 4 per line; some aren't fitting in the Shift+F9 anyway, since they are long
			if (c >= 4)
			{
				CoopMod_SendMessage (pnum, "%s", buf.c_str());
				buf.erase();
				c = 0;
			}
			else
				buf += ' ';
		}
		buf += it->name;
		c++;
		it++;
	}
	if (c)
		CoopMod_SendMessage (pnum, "%s.", buf.c_str());
	else
		CoopMod_SendMessage (pnum, "- no commands -");
}

void Cmd_Init ()
{
// template definitions
	if (0)
	{
		CMD_ADD_CMD(0, 0, 0);
	}
// due to buggy MSVC 6.0, default args will not work here
	CMD_ADD_CMD("help", Cmd_Help, "get list of all commands");
	CMD_ADD_CMD("hlp", Cmd_Help, "get list of all commands");
// "/?" is used in most applications as an argument to get help
	CMD_ADD_CMD("?", Cmd_Help, "same as \"help\"");
	CMD_ADD_CMD("version", CmdShow_Version, "show versions");
	CMD_ADD_CMD("ver", CmdShow_Version, "show versions");
	CMD_ADD_CMD("difficulty", CmdShow_Difficulty, "show difficulty");
	CMD_ADD_CMD("diff", CmdShow_Difficulty, "show difficulty");
	CMD_ADD_CMD("log", CmdShow_Logging, "show which loggings are enabled");
	CMD_ADD_CMD("myshield", CmdShow_OwnShield, "show how much shield you have");
	CMD_ADD_CMD("pps", CmdShow_PPS, "show current settings for packets per second");
	CMD_ADD_CMD("robotsnum", CmdShow_ShowRobotsNum, "show how much robot objects currently are");
	CMD_ADD_CMD("rn", CmdShow_ShowRobotsNum, "show how much robot objects currently are");
//	CMD_ADD_CMD("settings", CmdShow_Settings, "show some settings");
	CMD_ADD_CMD("showrobotnearest", CmdShow_ShowRobotNearest, "show beam to a nearest robot");
	CMD_ADD_CMD("srn", CmdShow_ShowRobotNearest, "show beam to a nearest robot");
	CMD_ADD_CMD("sleep", CmdShow_Sleep, "show sleep time");

	CMD_ADD_UINT("objectinfo", CmdShow_ObjectInfo, "show info about an object");
	CMD_ADD_UINT("oi", CmdShow_ObjectInfo, "show info about an object");
	CMD_ADD_UINT("objectshield", CmdShow_ObjectShield, "show how much shield an object has");
	CMD_ADD_UINT("os", CmdShow_ObjectShield, "show how much shield an object has");
	CMD_ADD_UINT("objectshowbeam", CmdShow_ObjectShowBeam, "show an object");
	CMD_ADD_UINT("osb", CmdShow_ObjectShowBeam, "show an object");
	CMD_ADD_UINT("playershield", CmdShow_PlayerShield, "show how much shield a player has");
	CMD_ADD_UINT("ps", CmdShow_PlayerShield, "show how much shield a player has");
}

// does not receive a messages from player #0 or dedi console, but I actually don't see a need in that
// returns 1, if ... doesn't matter
bool Cmd_Dispatch (u32 plix, const char* str)
{
//	bool r;
// *could make them all 'static', to avoid of malloc at the run time; however, it is a bit more complicated with 'vector<>'
	std::vector<cmd_t>::iterator it;
	std::vector<cmd_t>::iterator it_end;
	std::string s;
	u32 uint_0;
	stringstream ss;
	stringstream ss0;
	std::string cmd;
	std::vector<std::string> args;
	u32 args_num;
	u32 num_args_required;

// *normally should no message on the server side be actually, but since these commands goes through chat - may be possible
// nah, only while debugging; a client may play with these commands as much as and how he wish
// even hadn't helped me
//	DLLAddHUDMessage ("player #%d cmd \"%s\"", plix, str);
	it = cmds.begin();
	it_end = cmds.end();
//	s = str;
	ss << str;
	ss >> cmd;
	if (! cmd.length())
// no actual content
		return 0;
	args.clear();
	while (1)
	{
// *spaces after a word are not skipped
// *something is buggied :  "cmd " - no eof after taking the cmd, "cmd 0 " - eof after teking the first arg
// lets even exclude eof flag
#if 0
		ws(ss);
		if (ss.eof())
			break;
		ss >> s;
#else
		ss >> s;
		if (! s.length())
			break;
#endif
		args.push_back(s);
	}
	args_num = args.size();

	mklower(cmd);
	while (it != it_end)
	{
		if (! it->name.compare(cmd))
			goto dispatch_command;
		it++;
	}

	CoopMod_SendMessage (plix, "command \"%s\" is unknown", str);
	return 0;

dispatch_command:
// can be switched off; a command should answer some, and if the command is not recognized, then the command dispatcher could answer that the command isn't known
//	LogFile_Chat_Printf ("* command from player %d", hook_store_plix);
	if (! it->func)
	{
		goto finish;
	}

#define CHECK_ARGS_NUM(_required_num) do { num_args_required = _required_num; if (args_num != num_args_required) { if (num_args_required) goto wrong_args_num; else goto no_args_required; } } while (0)
//// prepare if the stringstream will be necessary again
//	ss.str("");
//	ss.clear();
	switch (it->type)
	{
	case CMD_TYPE__CMD:
//		num_args_required = 0;
//		if (! ss.eof())
//		if (args.size())
//			goto no_args_required;
		CHECK_ARGS_NUM(0);
		((cmd_funct_cmd_t*)it->func) (plix);
		break;

	case CMD_TYPE__UINT:
//		num_args_required = 1;
//		if (args.size() != num_args_required)
//			goto wrong_args_num;
		CHECK_ARGS_NUM(1);
		ss0 << args[0];
		ss0 >> uint_0;
		if (! ss0.eof())
		{
			CoopMod_SendMessage (plix, "*** command \"%s\" failed to read arg %u \"%s\" as a number", cmd.c_str(), 0, args[0].c_str());
			goto finish;
		}
		((cmd_funct_uint_t*)it->func) (plix, uint_0);
		break;

	case CMD_TYPE__UINT_UINT:
		CoopMod_SendMessage (plix, "*** command \"%s\" - u32 u32 command type is not implemented", cmd.c_str());
		break;

	default:
// into a log; it is because that is how it more probably will be discovered
		LogFile_Chat_Printf ("***** player #%d, command \"%s\" has wrong type %u", plix, it->type);
	}

finish:
	return 1;

no_args_required:
	CoopMod_SendMessage (plix, "*** command \"%s\" does not require arguments; got %u arguments", cmd.c_str(), args_num);
	return 1;

//cmd_unfinished:
//more_args:
wrong_args_num:
//	CoopMod_SendMessage (plix, "*** command \"%s\" has more arguments %u than is necessary %u", cmd.c_str(), );
//	CoopMod_SendMessage (plix, "*** command \"%s\" has more arguments than is necessary %u", cmd.c_str(), );
	CoopMod_SendMessage (plix, "*** command \"%s\" requires %u arguments, got %u arguments", cmd.c_str(), num_args_required, args_num);
	return 1;
}


// by Alexander Ilyin

#include "coop_mod_cmd.h"
#include "../osiris/osiris_import.h"
#include "coop.h"
#ifdef WIN32
//#include "image.h"
#define vsnprintf _vsnprintf
#endif
// MSVC 6.0
//#if _MSC_VER <= 1200
//#pragma warning(disable:4786)
//#endif
#define vector myvector
#include <vector>
#include <map>
#include "error.h"
#include "game_utils.h"
//#include "dallas.h"
#include "script.h"
#include "commands.h"
#include "drop.h"
#include "macros.h"
#include "screen.h"
#include "cmd.h"


std::vector<const char*> spoiling_admin_answers_sv;
std::vector<const char*> spoiling_admin_answers_cl_pub;
#undef vector
#define CMDVAR "CMDNAME"
#define VAR_QUOTED(_var) "\"%" _var "%\""


bool CoopModCmd_Init ()
{
//	bool r;
	string s0;
	string s1;

	spoiling_admin_answers_sv.push_back("will not be set while there are other players in the game");
//	spoiling_admin_answers_sv.push_back("others wouldn't have to see your naked bottom; or do they obligatory have to ?");
	spoiling_admin_answers_sv.push_back("dum-de-dum-de-dum ... no !");
	spoiling_admin_answers_sv.push_back("your parents will be informed about this action");
	spoiling_admin_answers_sv.push_back("your homework has to be done before any attempt to use this command");
	spoiling_admin_answers_sv.push_back("so you don't like to play, but like to administrate, isn't it ?");
	spoiling_admin_answers_sv.push_back("more ! mooore ! give me MORE ! ... no !");
	spoiling_admin_answers_cl_pub.push_back("administrator unsuccessfully attempted to use " VAR_QUOTED(CMDVAR) " command; note :  unsuccessfully");
	spoiling_admin_answers_cl_pub.push_back("administrator is attempting to administrate, using " VAR_QUOTED(CMDVAR) " command");
	spoiling_admin_answers_cl_pub.push_back("administrator is *&%#@#%*#@#!$^* " VAR_QUOTED(CMDVAR) " !^%$**%#@%&*");
	spoiling_admin_answers_cl_pub.push_back("administrator attempts to spoil the game using " VAR_QUOTED(CMDVAR) " command");

// test
#if 0
	Script_ClearVars ();
#if 0
	r = Script_Var_Update ("def", "<val def>");
	if (! r)
		return 0;
	r = Script_Var_Update ("abc", "<val abc>");
	if (! r)
		return 0;
	r = Script_Var_Update ("ghi", "<val ghi>");
	if (! r)
		return 0;
#endif
	r = Script_Var_Update (CMDVAR, "<test>");
	if (! r)
		return 0;
	s0 = "administrator attempts to spoil the game using " VAR_QUOTED(CMDVAR) " command";
//	s0 = "administrator " VAR_QUOTED("def") " attempts to spoil the game using " VAR_QUOTED(CMDVAR) " command";
	r = Script_String_InplaceIn (s1, s0);
	if (! r)
		return 0;
#endif

	return 1;
}

char cmd_macro[256] = {0};

// *add the index of macro, if want few macros
//void CoopMod_SetMacro1 ()
//void CoopMod_SetMacro (char* macro)
void CoopMod_SetMacro (const char* macro)
{
	int i;

	i = strlen (macro);
	if (i >= sizeof(cmd_macro))
	{
		DLLAddHUDMessage ("macro is longer than of %u characters", sizeof(cmd_macro) - 1);
		return;
	}
	strcpy (cmd_macro, macro);
	DLLAddHUDMessage ("macro set to \"%s\"", cmd_macro);
}

void CoopMod_RunMacro ()
{
	if (! *cmd_macro)
	{
		DLLAddHUDMessage ("no macro");
		return;
	}
	DLLAddHUDMessage ("macro \"%s\"", cmd_macro);
	if (*cmd_macro == '/')
	{
#if 0
		Screen_Restore ();
		__asm int 3
#endif
// oh, I already have noted, that 'EVT_CLIENT_INPUT_STRING' is for "$..." strings only
#if 0
		dllinfo di = {0};
//// *less likely it should be 'DMFCBase->TranslateEvent()'
		di.input_string = cmd_macro;
//		DLLGameCall (EVT_CLIENT_INPUT_STRING, &di);
//		DMFCBase->CallClientEvent (EVT_CLIENT_INPUT_STRING, -1, -1, 0);
		DMFCBase->TranslateEvent (EVT_CLIENT_INPUT_STRING, &di);
#endif
		if (local_role_is_server)
			Cmd_Dispatch (0, &cmd_macro[1]);
		else
// uhoh, long time to think about it; no big need to do it now ...
//			CoopMod_SendMessage_ (pnum, GR_RGB(128,96,160), sound, format, args);
//			CoopMod_SendMessage (0, "%s says: %s", cmd_macro);
			DLLAddHUDMessage ("client->server macros are not supported");
	}
	else
		DMFCBase->OnInputString (cmd_macro);
}

bool CoopMod_Kill (int pnum, char* cause)
{
	bool r;
	object* playerobj;
	player_info_t* player_info;

	r = CoopMod_TeammateStatus_Get (&player_info, pnum, "Kill ()");
	if (! r)
		return 0;

	playerobj = player_info->object_ptr;

//	DLLApplyDamageToPlayer (int playerobj, int killerobj, int damage_type, playerobj->shields,int server_says=0,int weapon_id=255,bool playsound=true);
//	DLLApplyDamageToPlayer (playerobj, playerobj, 0, playerobj->shields,int server_says=0,int weapon_id=255,bool playsound=true);

	msafe_struct mstruct;
	mstruct.objhandle = playerobj->handle;
	mstruct.killer_handle = playerobj->handle;
	mstruct.damage_type = 0;
	mstruct.amount = playerobj->shields + 0.1;
	MSafe_CallFunction (MSAFE_OBJECT_DAMAGE_OBJECT, &mstruct);

// crashes
//	DMFCBase->DoDamageToPlayer (pnum, 0, playerobj->shields + 0.1, false);

	if (cause)
		CoopMod_SendMessage (-1, "Kill for player \"%s\", cause :  %s", player_info->callsign, cause);
	else
		CoopMod_SendMessage (-1, "Kill for player \"%s\"", player_info->callsign);

	return 1;
}

// say the cause of the kicking, and start kick timer
void CoopMod_Kick (int pnum, bool delayed, char* format, ...)
{
	bool r;
	va_list args;
	player_info_t* player_info;
	char message[1000];
	float time = DMFCBase->GetGametime ();
//	float delay = delayed ? kick_time_delay : 0.0;
	float delay = delayed ? kick_time_delay : -0.1;

	va_start(args, format);
	vsnprintf (message, sizeof(message) - 1, format, args);
// if message too long, vsnprintf () won't terminate
	message[sizeof(message) - 1] = 0;

// do not kick main player (-Server- in case of dedicated server)
	if (! pnum)
	{
		DLLAddHUDMessage ("player 0 will not be kicked");
		return;
	}
	r = CoopMod_TeammateStatus_Get (&player_info, pnum, "Kick ()");
	if (! r)
		return;

// switch accelerator trigger
	coop_accel_kick = 1;
	if (! player_info->time.kick)
	{
		player_info->time.kick = time + delay;

		if (! delayed)
		{
// do not wait 'OnInterval ()' - the reason to kick was serious
// first kick, then message
			CoopMod_OnInterval_Kick ();
			CoopMod_SendMessage (-1, "immediate kick for %s, cause :  %s", player_info->callsign, message);
		}
		else
//			if (format)
//			CoopMod_SendMessage (-1, "kick for %s, after %f seconds", callsign, delay);
			CoopMod_SendMessage (-1, "kick for %s, after %.0f seconds, cause :  %s", player_info->callsign, delay, message);
	}
//	else
//		return;
}

void CoopMod_UnKick (int pnum)
{
	bool r;
	player_info_t* player_info;

	r = CoopMod_TeammateStatus_Get (&player_info, pnum, "UnKick ()");
	if (! r)
		return;

	if (player_info->time.kick)
	{
		player_info->time.kick = 0.0;
		CoopMod_SendMessage (-1, "kick sequence for %s aborted", player_info->callsign);
	}
	else
		CoopMod_SendMessage (-1, "player %s isn't kicked", player_info->callsign);
}

void CoopMod_UnKickAll ()
{
int i;

	for (i = 0; i < DLLMAX_PLAYERS; i++)
	{
		if (player_info_ptrs[i])
		{
			if (player_info_ptrs[i]->time.kick)
				CoopMod_UnKick (i);
			else
				DLLAddHUDMessage ("player #%d kick wasn't started", i);
		}
	}
}

void CoopMod_SetSleep (bool change, int value)
{
	if (change)
	{
// limit when server becomes unhandlable
		if (value > 1000)
			value = 1000;
		CoopMod_SendMessage (-1, "sleep time is set to %d ms (was %d ms)", value, coop_sleep);
		coop_sleep = value;
	}
	else
		CoopMod_SendMessage (-1, "sleep time is %d ms", coop_sleep);
}

void CoopMod_MoveObject (int objhandle, int room_num, bool echo)
{
	room* rooms;
	msafe_struct mstruct;
	u32 ix;

// get handle by ... handle (to let to use indices, and not only the handles)
	if (objhandle < MAX_OBJECTS)
	{
		object* obj;
		ix = objhandle & HANDLE_OBJNUM_MASK;
		obj = &objects[ix];
		objhandle = obj->handle;
	}
	mstruct.objhandle = objhandle;
	rooms = DMFCBase->GetRooms ();
//	DLLComputeRoomCenter (&mstruct.pos, &rooms[room_num]);
// position change function also does set orient; so get current
	MSafe_GetValue (MSAFE_OBJECT_ORIENT, &mstruct);
//	Room_Value (room_num, VF_GET, RMSV_V_PATH_PNT, &mstruct.pos, 0);
	Room_Value (room_num, VF_GET, RMSV_V_PORTAL_PATH_PNT, &mstruct.pos, 0);
//	Room_Value (room_num, VF_GET, RMSV_V_FACE_CENTER_PNT, &mstruct.pos, 0);
	if (echo)
//		DLLAddHUDMessage ("object %u, new position is :  room %u, x %f, y %f, z %f", objhandle, room_num, mstruct.pos.x, mstruct.pos.y, mstruct.pos.z);
		CoopMod_SendMessage (-1, "object handle %u, new position is :  room %u, x %f, y %f, z %f", objhandle, room_num, mstruct.pos.x, mstruct.pos.y, mstruct.pos.z);
	mstruct.roomnum = room_num;
//	MSafe_CallFunction (MSAFE_OBJECT_POS, &mstruct);
//	MSafe_CallFunction (MSAFE_OBJECT_ROOMNUM, &mstruct);
//	MSafe_CallFunction (MSAFE_OBJECT_POS, &mstruct);
	MSafe_CallFunction (MSAFE_OBJECT_WORLD_POSITION, &mstruct);
}

void CoopMod_MoveObject (int objhandle, int room_num)
{
	CoopMod_MoveObject (objhandle, room_num, 1);
}

void CoopMod_MovePlayer (int pnum, int room_num, bool echo)
{
	bool r;
	msafe_struct mstruct;
	player_info_t* player_info;

	ubyte room_type = Room_IsValid (room_num);
	if (! room_type)
	{
		CoopMod_SendMessage (-1, "room %d isn't valid for a player disposition", room_num);
		return;
	}
//	else if (room_type == 1)
//	{
//		CoopMod_SendMessage (-1, "the room %d is in a terrain's cell", room_num);
//		return;
//	}

	if (DMFCBase->IsPlayerDedicatedServer (pnum))
	{
		CoopMod_SendMessage (-1, "can't move dedicated server main player");
		return;
	}
	r = CoopMod_TeammateStatus_Get (&player_info, pnum, "MovePlayer ()");
	if (! r)
		return;

	mstruct.slot = pnum;
	MSafe_GetValue (MSAFE_OBJECT_PLAYER_HANDLE, &mstruct);
//	DLLAddHUDMessage ("objhandle %d %d", mstruct.objhandle, player_info->object_index);

	CoopMod_MoveObject (mstruct.objhandle, room_num, 0);
	if (echo)
//		CoopMod_SendMessage (-1, "player %s is moved to room's %d center", player_info->callsign, room_num);
		CoopMod_SendMessage (-1, "player %s is moved to room's %d portal", player_info->callsign, room_num);

	mstruct.state = 1;
	mstruct.lifetime = 3.0;
	MSafe_CallFunction (MSAFE_OBJECT_INVULNERABLE, &mstruct);
}

void CoopMod_MovePlayer (int pnum, int room_num)
{
	CoopMod_MovePlayer (pnum, room_num, 1);
}

void CoopMod_MovePlayersAll (int room_num)
{
	u32 i;
	u32 num;

	num = 0;
	for (i = 0; i < DLLMAX_PLAYERS; i++)
//	for (i = local_role_is_server; i < DLLMAX_PLAYERS; i++)
	{
		if (DMFCBase->IsPlayerDedicatedServer (i))
			continue;
		if (player_info_ptrs[i])
		{
			CoopMod_MovePlayer (i, room_num, 0);
			num++;
		}
	}

	CoopMod_SendMessage (-1, "all players (%u) were moved to room %d", num, room_num);
}

void CoopMod_ShowObjInfo (bool show_id, u32 ix)
{
	object* obj;

	obj = &objects[ix];
	if (show_id)
		DLLAddHUDMessage ("id %04x, object ix %u, handle %u", obj->id, ix, obj->handle);
	else
		DLLAddHUDMessage ("object ix %u, handle %u", ix, obj->handle);
}

//template<bool do_sort> void T_FindObj (int obj_type, int obj_id)
void CoopMod_FindAndShowObjects (bool sort_by_distance, int obj_type, int obj_id)
{
	u32 i;
	object* obj;
	u32 num;
	bool use_id;
	typedef std::multimap<float, u32> mm_f_i_t;
	static mm_f_i_t mm;
// 'vector' structure redefinition does not help; the structure then cannot be copied
#if 1
	vector own_pos;
	//vec own_pos;
#else
// *'vector' conflicts with 'std::vector'
struct vec
{
	float x, y, z;
};
	vec own_pos;
#endif

	if ((u32)obj_type >= MAX_OBJECT_TYPES)
	{
		DLLAddHUDMessage ("object type %u is higher than MAX_OBJECT_TYPES %u", obj_type, MAX_OBJECT_TYPES);
		return;
	}
	use_id = obj_id != -1;
	if (use_id)
	{
		if ((u32)obj_id >= MAX_OBJECT_IDS)
		{
			DLLAddHUDMessage ("object id %u is higher than MAX_OBJECT_IDS %u", obj_id, MAX_OBJECT_IDS);
			return;
		}
		DLLAddHUDMessage ("object type %u, id %u", obj_type, obj_id);
	}
	else
	{
		DLLAddHUDMessage ("object type %u, id is ignored", obj_type);
	}

	if (sort_by_distance)
	{
		u32 pnum;
		u32 obj_ix;

		mm.clear();
// get own position
		pnum = DMFCBase->GetPlayerNum ();
		Assert(pnum < DLLMAX_PLAYERS);
		obj_ix = players[pnum].objnum;
		Assert(obj_ix < MAX_OBJECTS);
		own_pos = objects[obj_ix].pos;
	}
	num = 0;
	for (i = 0; i < MAX_OBJECTS; i++)
	{
		obj = &objects[i];
		if (obj->type != obj_type)
			continue;
		if (use_id)
			if (obj->id != obj_id)
				continue;
#if 0
		if (use_id)
			DLLAddHUDMessage ("object ix %u, handle %u", i, obj->handle);
		else
			DLLAddHUDMessage ("id %04x, object ix %u, handle %u", obj->id, i, obj->handle);
#else
		if (sort_by_distance)
		{
			float dist;
//// *bad, should use the command using player instead
//			dist = DLLvm_GetMagnitude(obj->pos - own_pos);
			dist = DLLvm_VectorDistance(&obj->pos, &own_pos);
//			mm.insert(dist, i);
//			mm.insert(pair<float, u32> (dist, i));
			mm.insert(std::pair<float, u32> (dist, i));
		}
		else
			CoopMod_ShowObjInfo (! use_id, i);
#endif
		num++;
	}
	if (sort_by_distance)
	{
		mm_f_i_t::reverse_iterator it;
// begin from the farthest
		for (it = mm.rbegin(); it != mm.rend(); it++)
		{
			CoopMod_ShowObjInfo (! use_id, it->second);
		}
	}
	DLLAddHUDMessage ("total objects found %u", num);
}

// unsorted find of an objects
void CoopMod_FindObj (int obj_type, int obj_id)
{
//	T_FindObj<0> (obj_type, obj_id);
	CoopMod_FindAndShowObjects (0, obj_type, obj_id);
}

// sorted search for a/the robots
void CoopMod_FindRobot (int obj_id)
{
//	T_FindObj<1> (OBJ_ROBOT, obj_id);
	CoopMod_FindAndShowObjects (1, OBJ_ROBOT, obj_id);
}

void CoopMod_DumpMem (const char* filename, void* mem, u32 len)
{
	FILE* f;
	u32 i;

	f = fopen (filename, "wb");
	if (! f)
	{
		DLLAddHUDMessage ("failed to open file for memory dump");
		return;
	}
	i = fwrite (mem, len, 1, f);
//	if (i != len)
	if (i != 1)
	{
		DLLAddHUDMessage ("failed to write %u bytes of memory dump into the file; returned result is %u", len, i);
//		return;
		goto finish;
	}
	DLLAddHUDMessage ("object structure has been dumped");
finish:
	fclose (f);
}

void CoopMod_DumpObj ()
{
	object* obj;
	u32 ix;

// test
//	obj->ctype.laser_info.parent_type = 0;
//	int i = sizeof(t_ai_info);
// what about object 0 ?  it isn't the player #0 ?
	if (! coopmod_dump_obj_handle)
	{
		DLLAddHUDMessage ("no object is set to dump");
		return;
	}
// *unsafe
	ix = coopmod_dump_obj_handle & HANDLE_OBJNUM_MASK;
	obj = &objects[ix];
// test
//	obj->mtype.phys_info.rotvel = obj->mtype.phys_info.rotvel;
	CoopMod_DumpMem ("dump_obj.bin", obj, sizeof(*obj));
}

// can crash, if 'ai_info' will point not at 't_ai_info'
void CoopMod_DumpObjAI ()
{
	object* obj;
	u32 ix;

// what about object 0 ?  it isn't the player #0 ?
	if (! coopmod_dump_obj_handle)
	{
		DLLAddHUDMessage ("no object is set to dump");
		return;
	}
// *unsafe
	ix = coopmod_dump_obj_handle & HANDLE_OBJNUM_MASK;
	obj = &objects[ix];
	if (! obj->ai_info)
	{
		DLLAddHUDMessage ("object %u has no ai info", ix);
		return;
	}
// normal D3 SDK is not giving structure for this; use the corrected (more complete) SDK, to compile this
	CoopMod_DumpMem ("dump_objai.bin", obj->ai_info, sizeof(*obj->ai_info));
// nah, it obviously isn't 't_ai_info'
//	CoopMod_DumpMem ("dump_objai.bin", obj->ai_info, sizeof(t_ai_info));
//	CoopMod_DumpMem ("dump_objai.bin", obj->ai_info, sizeof(ai_frame));
}

//// *'cmd_name' isn't currently in use
// *but actually this protection is extra
// I suspect it will not disturb me less than prevent other admins to spoil the game with these cheats
void CoopMod_ReportLameAdmin (const char* cmd_name)
//void CoopMod_ReportLameAdmin ()
{
	bool r;
//	string s;
	string s0;
	string s1;
//	const char* msg;
	u32 ix;

// the arrays must be filled
	A(spoiling_admin_answers_sv.size());
	A(spoiling_admin_answers_cl_pub.size());

	r = Script_Var_Update (CMDVAR, cmd_name);
	if (! r)
		return;

// answer locally
	ix = rand() % spoiling_admin_answers_sv.size();
//	msg = spoiling_admin_answers_sv[ix];
	s0 = spoiling_admin_answers_sv[ix];
	r = Script_String_InplaceIn (s1, s0);
	if (! r)
		return;
//	DLLAddHUDMessage ("%s", msg);
	DLLAddHUDMessage ("%s", s1.c_str());
// answer publically
	ix = rand() % spoiling_admin_answers_cl_pub.size();
//	msg = spoiling_admin_answers_cl_pub[ix];
	s0 = spoiling_admin_answers_cl_pub[ix];
	r = Script_String_InplaceIn (s1, s0);
	if (! r)
		return;
//	CoopMod_SendMessage (-1, "%s", msg);
	CoopMod_SendMessage (-1, "%s", s1.c_str());
}

void CoopMod_ObjectShield (int ix, int value)
{
	bool r;
	object* obj;
	msafe_struct mstruct;

//	ix &= HANDLE_OBJNUM_MASK;
	if (ix >= MAX_OBJECTS)
	{
		DLLAddHUDMessage ("object #%u is out of range %u - %u", ix, 0, MAX_OBJECTS);
		return;
	}
	obj = &objects[ix];
// players are checked before being modified
	if (obj->type == OBJ_PLAYER)
	{
		player_info_t* plinfo;
		r = CoopMod_TeammateStatus_Get (&plinfo, obj->id, "ObjectShield");
		if (! r)
			return;
		CoopMod_PlayerShield (plinfo, value);
		return;
	}
	CoopMod_SendMessage (-1, "object %u shield is set to %d (was %f)", ix, value, obj->shields);
// have to send that info through network, so don't modify directly, but call msafe
#if 0
	obj->shields = value;
#else
	mstruct.objhandle = obj->handle;
	mstruct.shields = value;

	MSafe_CallFunction (MSAFE_OBJECT_SHIELDS, &mstruct);
#endif
}

// don't do the shield auto-restore on a second player join, because the player could at that moment be intensively attacked
// hmmm, don't compare here the new value with the old one, to avoid about admin action;
// instead, make a separate shield normalizing function
//void CoopMod_Player_SetShield (player_info_t* player_info, int value)
void CoopMod_PlayerShield (player_info_t* player_info, int value)
{
//	bool r;
//	player_info_t* player_info;
	msafe_struct mstruct;
	u32 num;
//	u32 i;

// count players number
	num = GameUtils_CountPlayers ();

// *there is no 'net_game'
//	if (netgame->curr_num_players)
	if (num > 1)
	{
		CoopMod_ReportLameAdmin ("PlayerShield");
// mmm bad, clients will not understand what "PS" is
//		CoopMod_ReportLameAdmin (command_cmd);
		return;
	}

//	r = CoopMod_TeammateStatus_Get (&player_info, pnum, "SetShield ()");
//	if (! r)
//		return;

	mstruct.objhandle = player_info->object_ptr->handle;
	mstruct.shields = value;

	MSafe_CallFunction (MSAFE_OBJECT_SHIELDS, &mstruct);
// no public echo, ok ?
	DLLAddHUDMessage ("player \"%s\" shield is set to %d", player_info->callsign, value);
// also say to him, ok ?
	CoopMod_SendMessage (player_info->pnum, "shield is set to %d", value);
}

void CoopMod_PlayerShieldNormalize (player_info_t* player_info)
{
	float shields;
	msafe_struct mstruct;

#define SHIELD_NORMAL_LEVEL 200
	Assert(player_info);
	shields = player_info->object_ptr->shields;
	if (shields <= SHIELD_NORMAL_LEVEL)
	{
		DLLAddHUDMessage ("player \"%s\" shield %f already is inside of the range", player_info->callsign, shields);
		return;
	}
	shields = SHIELD_NORMAL_LEVEL;

	mstruct.objhandle = player_info->object_ptr->handle;
	mstruct.shields = shields;

	MSafe_CallFunction (MSAFE_OBJECT_SHIELDS, &mstruct);
// no public echo, ok ?
	DLLAddHUDMessage ("player \"%s\" shield is normalized to %f", player_info->callsign, shields);
// also say to him, ok ?
	CoopMod_SendMessage (player_info->pnum, "shield is normalized to %f", shields);
}

void CoopMod_PlayerEnergy (player_info_t* player_info, int value)
{
	msafe_struct mstruct;
	u32 num;

// count players number
	num = GameUtils_CountPlayers ();

	if (num > 1)
	{
		CoopMod_ReportLameAdmin ("PlayerEnergy");
		return;
	}

	mstruct.objhandle = player_info->object_ptr->handle;
	mstruct.energy = value;

	MSafe_CallFunction (MSAFE_OBJECT_ENERGY, &mstruct);
	DLLAddHUDMessage ("player \"%s\" energy is set to %d", player_info->callsign, value);
	CoopMod_SendMessage (player_info->pnum, "energy is set to %d", value);
}

// *simply sets ammo for the current weapon; I don't want to do a complete solution, where could choose for which weapon to set the ammo
void CoopMod_PlayerAmmo (player_info_t* player_info, int value)
{
//	msafe_struct mstruct;
	u32 num;
//	int val;

// count players number
	num = GameUtils_CountPlayers ();

	if (num > 1)
	{
		CoopMod_ReportLameAdmin ("PlayerAmmo");
		return;
	}

// there is no normal interface to do that
#if 0
//	mstruct.objhandle = player_info->object_ptr->handle;
//	mstruct.shields = value;

//**
//	MSafe_CallFunction (MSAFE_OBJECT_ENERGY, &mstruct);
// *look 'PLYV_I_STRIP_WEAPONS' in 'aStripWeaponsEnergy()' on how to use it
//	val = 0;
	val = 1234;
// hmmm, 'PLYSV_I_WEAPON_AMMO' expects only 'VF_GET' (look 004a814e)
//	Player_Value (player_info->object_ptr->handle, VF_SET, PLYSV_I_WEAPON_AMMO, &val, 1);
	Player_Value (player_info->object_ptr->handle, VF_GET, PLYSV_I_WEAPON_AMMO, &val, 1);
	DLLAddHUDMessage ("player \"%s\" ammo is set to %d", player_info->callsign, value);
	CoopMod_SendMessage (player_info->pnum, "ammo is set to %d", value);
#endif
#if 1
	player* plr;
	player_weapon* plr_weapon;
//	short amount_ammo;
	u32 weapon_ix;
//	u32 powerup_id;
//	u32 pnum;

#if 0
	pnum = player_info->pnum;
	if (pnum >= MAX_PLAYERS)
		return;
	plr = &players[pnum];
#else
	plr = player_info->plr;
#endif
	plr_weapon = &plr->weapon[PW_PRIMARY];
	weapon_ix = plr_weapon->index;
	if (weapon_ix >= 10)
		return;
	plr->weapon_ammo[weapon_ix] = value;

	Drop_SetAmmo (player_info->pnum, weapon_ix, value);
#endif
//	DLLAddHUDMessage ("unsupported");
}

void CoopMod_PlayerMissiles (player_info_t* player_info, int value)
{
//	msafe_struct mstruct;
	u32 num;

// count players number
	num = GameUtils_CountPlayers ();

	if (num > 1)
	{
		CoopMod_ReportLameAdmin ("PlayerMissiles");
		return;
	}

#if 0
	mstruct.objhandle = player_info->object_ptr->handle;
	mstruct.shields = value;

//**
	MSafe_CallFunction (MSAFE_OBJECT_ENERGY, &mstruct);
	DLLAddHUDMessage ("player \"%s\" missiles is set to %d", player_info->callsign, value);
	CoopMod_SendMessage (player_info->pnum, "missiles is set to %d", value);
#endif
#if 1
	player* plr;
	player_weapon* plr_weapon;
	u32 weapon_ix;

	plr = player_info->plr;
	plr_weapon = &plr->weapon[PW_SECONDARY];
	weapon_ix = plr_weapon->index;
	if (weapon_ix < 10)
		return;
	if (weapon_ix >= 20)
		return;
	plr->weapon_ammo[weapon_ix] = value;

	Drop_SetAmmo (player_info->pnum, weapon_ix, value);
#endif
//	DLLAddHUDMessage ("unsupported");
}

//void CoopMod_Spawn (const char* arguments)
void CoopMod_SpawnInFront (const char* arguments)
{
	msafe_struct mstruct;
	int objnum;
	object* obj;
	int pnum;
	int objtype;
	int objid;
	stringstream strm;

// test; result is 895300
//	DLLAddHUDMessage ("object infos :  %08x", objects_info);

	FORI0(MAX_OBJECT_IDS)
	{
		int ii;
		object_info* oi;
		oi = &objects_info[i];
// test
#if 0
		if (oi->name[0] == 't')
			oi = oi;
		if (oi->name[0] == 'T')
			oi = oi;
#endif
// is it checked this way ?
// no
//		if (! oi->name[0])
//			break;
// look 4a649e
		if (oi->type == OBJ_NONE)
// nope, stops soon (on 0x69)
//			break;
			continue;
// D3 object names are case insensitive; look above 43276c, it calls 'stricmp()' which is on 579eb0
		ii = stricmp (arguments, oi->name);
		if (! ii)
		{
			objid = i;
//			goto found_name;
			goto create_object;
		}
	}

// oh yes, this is convenient, since D3 can't repeat commands, and have to type them anew
#if 1
	strm << arguments;
// don't know now how better to do its positioning; fast solution
//	strm >> pnum;
//	strm >> objtype;
	strm >> objid;
#if 0
	if (! strm.eof())
	{
		DLLAddHUDMessage ("soiled string");
		return;
	}
#else
	if (strm.eof())
	{
		goto create_object;
	}
#endif
#endif

	DLLAddHUDMessage ("object \"%s\" unknown", arguments);
	return;

//found_name:
create_object:
	pnum = 0;
// a bit abnormal, however it does work for a powerups also
	objtype = OBJ_ROBOT;

	mstruct.slot = pnum;
	MSafe_GetValue (MSAFE_OBJECT_PLAYER_HANDLE, &mstruct);

// get roomnum + pos + orient
	MSafe_GetValue (MSAFE_OBJECT_WORLD_POSITION, &mstruct);
	MSafe_GetValue (MSAFE_OBJECT_VELOCITY, &mstruct);

	mstruct.pos += mstruct.orient.fvec * 20.0;

	objnum = DLLObjCreate (objtype, objid, mstruct.roomnum, &mstruct.pos, &mstruct.orient, -1);
	if (objnum == -1)
		return;
// should not happen
	if (objnum < 0)
		return;
	if (objnum >= MAX_OBJECTS)
		return;

	obj = &objects[objnum];
// throw it forward
	obj->mtype.phys_info.velocity = mstruct.orient.fvec * 40.0;
// add ship velocity
	obj->mtype.phys_info.velocity += mstruct.velocity;

	DLLMultiSendObject (obj, 0, 1);

#if 0
	if (objid == 284)
	{
		obj->ai_info->ai_type = 2;
		DLLAddHUDMessage ("ai type was corrected");
		DLLAddHUDMessage ("ai class is %u", obj->ai_info->ai_class);
	}
#endif
	DLLInitObjectScripts (obj, 1);
//	DLLAddHUDMessage ("ai class is %u", obj->ai_info->ai_class);

// say to all, that the admin has modified the mine
	Assert(objects_info);
	CoopMod_SendMessage (-1, "spawned object %u :  id %u, name \"%s\", %s %u", objnum, objid, objects_info[objid].name, ROOMNUM_OUTSIDE(mstruct.roomnum) ? "cell" : "room", CELLNUM(mstruct.roomnum));
}

// I am in doubts :  it is necessary only for debug; but also is good, if want to skip rapidly some big part of a level; in other cases it is a crap-game-making function
void CoopMod_AddWeapon ()
{
	u32 num;

	num = GameUtils_CountPlayers ();

	if (num > 1)
	{
		CoopMod_ReportLameAdmin ("AddWeapon");
		return;
	}

#if 0
// *no description and any example of that
	msafe_struct mstruct;
//	mstruct.objhandle = objects[i].handle;
//	mstruct.playsound = 0;
	Screen_Restore ();
// *'MSafe_CallFunction ()' is constantly requested with -1 and 0xab, but isn't requested for a weapon take (tested on smart missiles and napalm gun and gunboys)
	MSafe_CallFunction (MSAFE_OBJECT_ADD_WEAPON, &mstruct);
#endif

// the hud name is "GunBoy"
#if 0
	bool r;
	player* plr;
//	plr = &(DMFCBase->GetPlayers())[1];
	plr = &(DMFCBase->GetPlayers())[0];
//// nothing appears
//"adds a type/id item to the inventory (returns true on success)"
// (Inventory *inven,int type,int id,object *parent,int aux_type,int aux_id,int flags,char *description);
// shown in hud, but nothing does, when using
//	r = Inven_Add (&plr->counter_measures, OBJ_POWERUP, POWERUP_COUNTERMEASURE_GUNBOY, 0, 0, 0, 0, "-none-");
// shown in hud, shots an electric shot to the back side, when using
//	r = Inven_Add (&plr->counter_measures, OBJ_WEAPON, POWERUP_COUNTERMEASURE_GUNBOY, 0, 0, 0, 0, "-none-");
//"adds a type/id item to the inventory (returns true on success)"
// (Inventory *inven,int id,int aux_type,int aux_id,int flags,char *description);
// shown in hud, shots an electric shot to the back side, when using
//	r = Inven_AddCounterMeasure (&plr->counter_measures, POWERUP_COUNTERMEASURE_GUNBOY, 0, 0, 0, "GunBoy");
// 44 - yellow laser, 45 - electric homing blue
// approx. 0-8 - primary shots
// approx. 8-16 - secondary shots (missiles)
//// ~95 - seeker mine (one more)
// 96 - gunboy
// 97 - mine
// 98 - seeker mine
	int i1;
#if 0
	Screen_Restore ();
	i1 = 12 * 8;
// ha ! don't write a description, to let the engine describe it itself
//	r = Inven_AddCounterMeasure (&plr->counter_measures, i1 + 0, 0, 0, 0, "GunBoy");
	r = Inven_AddCounterMeasure (&plr->counter_measures, i1 + 0, 0, 0, 0, 0);
	r = Inven_AddCounterMeasure (&plr->counter_measures, i1 + 1, 0, 0, 0, 0);
	r = Inven_AddCounterMeasure (&plr->counter_measures, i1 + 2, 0, 0, 0, 0);
	r = Inven_AddCounterMeasure (&plr->counter_measures, i1 + 3, 0, 0, 0, 0);
	r = Inven_AddCounterMeasure (&plr->counter_measures, i1 + 4, 0, 0, 0, 0);
	r = Inven_AddCounterMeasure (&plr->counter_measures, i1 + 5, 0, 0, 0, 0);
	r = Inven_AddCounterMeasure (&plr->counter_measures, i1 + 6, 0, 0, 0, 0);
	r = Inven_AddCounterMeasure (&plr->counter_measures, i1 + 7, 0, 0, 0, 0);
#endif
// add 16 gunboys
	for (i1 = 0; i1 < 16; i1++)
		r = Inven_AddCounterMeasure (&plr->counter_measures, 96, 0, 0, 0, 0);
//	Screen_Restore ();
	r = r;
#endif
}

void CoopMod_ObjectModifyFlag (int ix, u32 flag, bool on)
{
//	bool r;
//	u32 ix;
	object* obj;
	u32 old;

//	ix &= HANDLE_OBJNUM_MASK;
	if (ix >= MAX_OBJECTS)
	{
		DLLAddHUDMessage ("object #%u is out of range %u - %u", ix, 0, MAX_OBJECTS);
		return;
	}
	obj = &objects[ix];
	old = obj->flags;
// *possibly no need to call the engine; don't know; team flags are working with just this direct change
#if 1
	if (on)
		obj->flags |= flag;
	else
		obj->flags &= ~flag;
#else
#endif
	CoopMod_SendMessage (-1, "object %u flags were modified from %08x to %08x", ix, old, obj->flags);
}

void CoopMod_ObjectFlag_Clear (int ix, int flag_ix)
{
	CoopMod_ObjectModifyFlag (ix, 1 << flag_ix, 0);
}

void CoopMod_ObjectFlag_Set (int ix, int flag_ix)
{
	CoopMod_ObjectModifyFlag (ix, 1 << flag_ix, 1);
}

void CoopMod_ObjectModifyAIType (int ix, int type)
{
	object* obj;
	u32 old;

//	ix &= HANDLE_OBJNUM_MASK;
	if (ix >= MAX_OBJECTS)
	{
		DLLAddHUDMessage ("object #%u is out of range %u - %u", ix, 0, MAX_OBJECTS);
		return;
	}
	obj = &objects[ix];
	if (! obj->ai_info)
	{
		DLLAddHUDMessage ("object %u has no ai info", ix);
		return;
	}
// *'AI_SetType()' left it unmodified
	old = obj->ai_info->ai_type;
// hmmm, for AI type, cannot just set - it will not act; have to call the engine; while most or all flags do not need that
#if 0
	obj->ai_info->ai_type = type;
	CoopMod_SendMessage (-1, "object %u AI type was modified from %02x to %02x", ix, old, obj->ai_info->ai_type);
#else
	char c;
	old = obj->ai_info->ai_type;
	AI_PowerSwitch (obj->handle, 0);
//	old = 0;
// nope, not this one
// and hmmm, if the type is 'char' 5, then there are no 5 'char' anymore in the 'ai_info' struct
// looks like there is no 'AI_xxx()' function to get the type
// lets leave it so for now ...
//	AI_Value (obj->handle, VF_GET, AIV_C_MOVEMENT_TYPE, &c);
//	old = c;
	AI_SetType (obj->handle, type);
//	AI_Value (obj->handle, VF_GET, AIV_C_MOVEMENT_TYPE, &c);
	c = obj->ai_info->ai_type;
	AI_PowerSwitch (obj->handle, 1);
	CoopMod_SendMessage (-1, "object %u AI type was modified from %02x to %02x", ix, old, c);
#endif
}

// AIF_TEAM_PTMC, none :  normal enemy
// AIF_TEAM_REBEL, rebel :  attacks enemies, but enemy robots do not detect him, if no damage
// AIF_TEAM_HOSTILE, hostile :  attacks whoever; fights any, incl. the same; not detectable, if no damage
// AIF_TEAM_NEUTRAL, rebel + hostile :  until got damage, is passive

// *look 'aAIFlags()' also
//void CoopMod_ObjectModifyAIFlag (int ix, bool on)
void CoopMod_ObjectModifyAIFlag (int ix, u32 flag, bool on)
{
//	bool r;
//	u32 ix;
	object* obj;
	u32 old;

//	ix &= HANDLE_OBJNUM_MASK;
	if (ix >= MAX_OBJECTS)
	{
		DLLAddHUDMessage ("object #%u is out of range %u - %u", ix, 0, MAX_OBJECTS);
		return;
	}
	obj = &objects[ix];
	if (! obj->ai_info)
	{
		DLLAddHUDMessage ("object %u has no ai info", ix);
		return;
	}
//	r = !! (obj->ai_info->flags & AIF_TEAM_REBEL);
	old = obj->ai_info->flags;
// *possibly no need to call the engine; don't know; team flags are working with just this direct change
#if 1
	if (on)
		obj->ai_info->flags |= flag;
	else
		obj->ai_info->flags &= ~flag;
#else
	AI_PowerSwitch (obj->handle, 0);
	AI_Value (obj->handle, on ? VF_SET_FLAGS : VF_CLEAR_FLAGS, AIV_I_FLAGS, &flag);
	AI_PowerSwitch (obj->handle, 1);
#endif
	CoopMod_SendMessage (-1, "object %u AI flags were modified from %08x to %08x", ix, old, obj->ai_info->flags);
}

void CoopMod_ObjectAIFlag_Clear (int ix, int flag_ix)
{
	CoopMod_ObjectModifyAIFlag (ix, 1 << flag_ix, 0);
}

void CoopMod_ObjectAIFlag_Set (int ix, int flag_ix)
{
	CoopMod_ObjectModifyAIFlag (ix, 1 << flag_ix, 1);
}

// "Team rebel" flag
// ally, fights for players (like "Miniendboss")
//void CoopMod_ObjectRebel (int ix, bool on)
void CoopMod_ObjectRebel (int ix, int on)
{
//	bool r;

//	CoopMod_ObjectModifyAIFlag (ix, AIF_TEAM_REBEL, on);
	CoopMod_ObjectModifyAIFlag (ix, AIF_TEAM_REBEL, !! on);
}

// "Team hostile" flag
//// detectable as enemy of robots ?
//void CoopMod_ObjectHostile (int ix, bool on)
void CoopMod_ObjectHostile (int ix, int on)
{
//	bool r;

//	CoopMod_ObjectModifyAIFlag (ix, AIF_TEAM_HOSTILE, on);
	CoopMod_ObjectModifyAIFlag (ix, AIF_TEAM_HOSTILE, !! on);
}

void CoopMod_ObjectFire (int ix, int on)
{
//	bool r;

	CoopMod_ObjectModifyAIFlag (ix, AIF_FIRE, !! on);
}

void CoopMod_TeamDamage (bool change, bool value)
{
	if (change)
	{
		team_damage = value;
		Assert(net_game);
// fine, everything in this game works, except the 1st thing - the shutdown :)
// *shutting the dedi server down with 'NF_EXIT_NOW', crashes it; nondedi exits to server selection
// 'exit(0)' does no gamma palette restore
// no public exports for 'GetFunctionMode()'/'SetFunctionMode()' (I would try just set 'QUIT_MODE' there) (the analog of 'ned_SetupSearchPaths()' probably is on 508070 or less probably on 508b50, but I don't see any similar short subr near)
		if (team_damage)
			net_game->flags |= NF_DAMAGE_FRIENDLY;
		else
			net_game->flags &= ~NF_DAMAGE_FRIENDLY;
//			net_game->flags |= NF_EXIT_NOW;
//			exit (0);
	}

	CoopMod_SendMessage (-1, "team damage is %s", team_damage ? "on" : "off");
}

//// hmmm; will be done in client cmd
// is done in client cmd
void CoopMod_ObjectShowBeam (int ix)
{
//	Dallas_LightningCreate (int objhandle1, int objhandle2, float lifetime, float thickness, int numtiles, int texture_id, float slidetime, int timesdrawn, int red, int green, int blue, bool autotile);
}

void CoopMod_SetPPS (int pps)
{
//	u32 i;
	netplayer* netplayers = 0;
	u32 cnt;

	if (pps < 2)
		pps = 2;
// even though 'ubyte netplayer::pps' can have values up to 255, lets limit it to 127 ... though no
	if (pps > 255)
		pps = 255;
	coop_pps = pps;
// change pps to each connected player
	netplayers = DMFCBase->GetNetPlayers ();
	cnt = 0;
	FORI0(DLLMAX_PLAYERS)
	{
//		player_info_t* pi;
//		pi = player_info_ptrs[i];
		if (netplayers[i].flags & NPF_CONNECTED)
		{
			if (! (netplayers[i].flags & NPF_SERVER))
			{
				netplayers[i].pps = coop_pps;
				cnt++;
			}
		}
	}
//	CoopMod_SendMessage (-1, "PPS has been changed to %u", coop_pps);
	CoopMod_SendMessage (-1, "PPS has been changed to %u, %u netplayers have been updated", coop_pps, cnt);
}


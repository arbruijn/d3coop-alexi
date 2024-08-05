// functions separated from the base, to leave original file unchanged maximally how is possible

#include "coop_mod.h"
//#include "idmfc.h"
#include "coop.h"
// here this array has to be initialized, so undef the define
#undef __OSIRIS_IMPORT_H_
#include "../osiris/osiris_import.h"
//#include "../osiris/osiris_vector.h"
#include "vector.h"
#include "commands.h"
#include "error.h"
#include "missions.h"
#include "rooms.h"
#include "powerups.h"
#include "nicks.h"
#include "log_file.h"
#include "geoip.h"
// Linux does not know 'byte'; Windows somehow has already been defined
#include "types.h"
#include "exceptions.h"
#include "utils.h"
#include "hooks.h"
#include <imagehlp.h>
#include "screen.h"
#include "packet.h"
#include "cmd.h"
#include "drop.h"
#include "coop_mod_cmd.h"
#include "game_utils.h"

#ifndef WIN32
// *for 'select ()'
#include <sys/select.h>
#endif
#ifdef WIN32
#include "image.h"
#define vsnprintf _vsnprintf
#endif


#define MM_TIMER_PERIOD 1

// client or server
int local_role = 0;
bool local_role_is_server = 0;
// make a suspicious name for if will be undefined :)
const char* local_role_name = "-uninitialized_role_name-";

player_info_t player_infos[C_T_STATUSES_NUM];

// links to 'player_infos'
// *don't use it instead of 'player_info->used' or 'player_info->connected',
// because 'OnServerPlayerDisconnect ()' may being not always called, to clean up these variables
player_info_t* player_info_ptrs[];

// +++ configuration +++
bool auto_kick__team_kills = 1;
bool auto_kick__team_damage_long = 1;

// *for 5 seconds is not always possible at least to start typing :)
//float kick_time_delay = 5.0;
float kick_time_delay = 10.0;
// auxilary values for kicked client
float coop_kicked_divide_damage = 10.0;
// don't let kicked player to easy kill someone
float coop_kicked_damage_max = 1.0;

// 5 min for 3 teamkills is good, otherwise - very active players will be kicked too
// if someone makes many damage to teammates instead of doing damage to robots -
// teamdamage system must be used against his actions instead of using teamkill limit system
int coop_team_kills_max = 3;
float coop_team_kills_period = 600.0;

// calculate maximal teamdamage count value from the coeficient, but limit with these upper and lower values
int coop_teamdamage_long_min = 15;
int coop_teamdamage_long_max = 40;
// same, but filtered
// *not calculated at this moment, just static value
int coop_teamdamage_long_filtered = 4;
// (minimal) time period for abusive teamdamage detection
float coop_teamdamage_long_period = 4.0;
// division coefficient for total teamdamage statistics for teamdamage autokick
float coop_teamdamage_long_coeficient = 10.0;
// decrement teamdamage_long counter after this time
float coop_teamdamage_calmdown_time = 40.0;
float coop_teamdamage_calmdown_filtered_time = 120.0;
// a time after robotdamage, when teamdamage is not registering
float coop_teamdamage_robotdamage_post_time = 1.0;
// a time after playerdamage, when damage to player is not registering for filtered values
// *must be oriented on napalm missile damage, since it is mostly problematic, due to no possibility to define kind of damage
float coop_teamdamage_filter_time = 1.4f;

int coop_sleep = 1;
bool show_client_IP = 0;
bool show_connection_activity = 1;
bool show_connection_activity_packet = 1;
// due to often lacks in some scripts, I will turn it on by default
bool show_room_num = 1;
// *when touching
bool show_object_info_on_collide = 0;
// *must be used before those powerups will be taken, because indexes changes
bool crashy_powerups_block = 0;
bool crashy_powerups_move = 1;
bool coop_nick_check = 1;

// mode against GB events
// 0 - nothing, 1 - ignore event, 2 - kick with notify, 3 - immediate kick
int coop_anti_guidebot_mode = 1;
//int coop_anti_guidebot_mode = 0;
// block of GB relative packets (will not work for player #0)
//bool coop_guidebot_block = 1;
// added by Derek's request/report, for p2p game
// when one low pps client joins, all other also have (or maybe just got from him only ?) that pps
// and now I will use this too; anyone who has not configured his network settings must know, that his settings are very incorrect
int coop_pps_min = 12;
bool coop_pps_min_do_kick = 0;
//u32 coop_pps = 61;
u32 coop_pps = 255;
float coopmod_time_distortion = 1.0;
int coopmod_dump_obj_handle = 0;
// +++ only for value keeping +++
bool team_damage = 1;
// --- only for value keeping ---
bool coop_mod_endlevel_on_0_robots = 0;
// --- configuration ---

// *these handles not helps
int gb_released_handle = -1;
object* gb_released_obj = 0;

// *accelerator for kicks processing
bool coop_accel_kick = 0;
GeoIP* geoip_instance = 0;

// + game data +
netgame_info* net_game;

// *'object_info' structure size is 0x1d0
object_info* objects_info = 0;

// *'object' structure size is 0x328
// *notable is, that in most cases, zeroes are blocks :  11d - 160, 17c - 2b8
// 000 type
// 001 dummy_type
// 002 id
// 004 flags
// 008 name
// 00c handle
// 010 next
// 012 prev
// 014 control_type
// 015 movement_type
// 016 render_type
// 017 lighting_render_type
// 018 roomnum
// 01c pos
// 028 orient
// 04c last_pos
// 058 renderframe
// 05c wall_sphere_offset
// 068 anim_sphere_offset
// 074 size
// 078 shields
//
// 080 creation_time
// 084 lifeleft
// 088 lifetime
// 08c parent_handle
// 090 attach_ultimate_handle
// 094 attach_parent_handle
// 098 attach_children
// 09c weapon_fire_flags
// 09d attach_type
// 09e lowest_attached_vis
// 0a0 attach_dist / attach_index
// 0a4 mtype
//   0a4 mtype.phys_info.velocity
//   0bc mtype.phys_info.rotvel
//   0ec mtype.phys_info.rotdrag
//   11c mtype.phys_info.flags
// 160 min_xyz
// 16c max_xyz
// 178 dynamic_wb
// 17c impact_size
// 180
// 184
// 188
// 18c impact_force
// 190 change_flags
// 194
// 198
// 19c lm_object
// 258 ctype
//   258 ctype.laser_info.parent_type
// 2b8 ai_info (is 'void*', but could be 't_ai_info*')
// 2bc rtype
//   2bc rtype...
// 310 effect_info
// 314 lighting_info
// 318 position_counter
// 31c osiris_script
// 320 custom_default_script_name
// 324 custom_default_module_name
object* objects = 0;

player* players = 0;

// 0x00a207b0
// 'ship' structure size is 0x1724
// bc static_wb[]
// e8 static_wb[].gp_fire_wait[]
ship* ships = 0;

// *'msafe_struct' structure size is 0x398
// *     look better in "osiris_common.h"
// *'objhandle' is at 0x2c
// *'type' is at 0x44 (MSAFE_INVEN_GET_TYPE_ID)
// *'id' is at 0x48 (MSAFE_INVEN_GET_TYPE_ID)
// *'is_real' is at 0xe4
// *'index' is at 0x108
// *'state' is at 0x114 (MSAFE_INVEN_CHECK, MSAFE_INVEN_CHECK_OBJECT)
// *'slot' is at 0x115
// *'size' is at 0x244 (MSAFE_INVEN_SIZE)
// *'count' is at 0x24c (MSAFE_INVEN_COUNT, MSAFE_INVEN_GET_TYPE_ID)
// - game data -

// will not work properly
/*
typedef struct object_respawn_info_s
{
	bool is_powerup;
// will except bugs, but not on 100%
	ushort id;
	byte algn;
} object_respawn_info_t;

object_respawn_info_t object_respawn_infos[MAX_OBJECTS];
*/

// beautiful would be explain everything what I am doing
void* malloc06100000 = 0;

// *debug stuff is placed higher than this (static, because of 'player_info_t') function
//player_info_t* CoopMod_GetTeammateStatus (int pnum, bool warn_if_not_indexed, char* caller);


// check the client info - is it filled and can be used or not
bool CoopMod_TeammateStatus_Check (player_info_t* player_info, bool warn_if_not_indexed, bool must_be_connected, char* caller)
{
//	Assert(player_info);
#if 0
// I will not subdivide it, to show more detailed info - I don't see big sense,
//// '! player_info' is a protection only, and does not say anything
	if (warn_if_not_indexed)
	{
		if (! player_info || ! player_info->connected)
		{
// *means - not connected, but lets say exactly what we see
			DLLAddHUDMessage ("%s :  player %d is not registered", caller, pnum);
			return 0;
		}
	}
#endif
	if (! player_info)
	{
		if (warn_if_not_indexed)
//			DLLAddHUDMessage ("%s :  no player info ptr", caller);
			DLLAddHUDMessage ("%s :  no player info ptr (possibly is not connected)", caller);
		return 0;
	}
	if (must_be_connected)
	{
		if (! player_info->connected)
		{
			DLLAddHUDMessage ("%s :  player %d is not registered", caller, player_info);
			return 0;
		}
	}

	return 1;
}

// define a client by pnum (ix)
//bool CoopMod_TeammateStatus_Get (player_info_t** player_info, int pnum, bool warn_if_not_indexed, char* caller)
bool CoopMod_TeammateStatus_Get (player_info_t** player_info_out, int pnum, char* caller, bool warn_if_not_indexed)
{
	bool r;
	player_info_t* player_info;
	static char buffer[1024] = {0};

//	Assert(player_info_out);
	if (pnum < 0 || pnum >= DLLMAX_PLAYERS)
	{
		DLLAddHUDMessage ("'%s' :  player number %d is out of range", caller, pnum);
		return 0;
	}
	player_info = player_info_ptrs[pnum];
//	r = CoopMod_TeammateStatus_Check (player_info, warn_if_not_indexed, 1, caller);
//	r = CoopMod_TeammateStatus_Check (player_info, 1, 1, "CoopMod_TeammateStatus_Get (pnum)");
	_snprintf (buffer, sizeof(buffer) - 1, "%s, CoopMod_TeammateStatus_Get (%d)", caller, pnum);
//	r = CoopMod_TeammateStatus_Check (player_info, 1, 1, buffer);
	r = CoopMod_TeammateStatus_Check (player_info, warn_if_not_indexed, 1, buffer);
	if (player_info_out)
		*player_info_out = player_info;
	return r;
}

// define a client by an automatically parsed identificator (string), which may be :  ix/id/name
//bool CoopMod_TeammateStatus_Get (player_info_t** player_info, string player, bool warn_if_not_indexed, char* caller)
bool CoopMod_TeammateStatus_Get (player_info_t** player_info_out, string player, char* caller)
{
	bool r;
	player_info_t* player_info;
	stringstream strm;
	bool must_be_connected = 0;
	int pnum;

	Assert(player_info_out);
	strm << player;
// parsing is simplified atm; only pnum
	must_be_connected = 1;
	strm >> pnum;
	if (strm.fail())
	{
		DLLAddHUDMessage ("'%s' :  cannot convert string \"%s\" to player number", caller, player.c_str());
		return 0;
	}
	ws(strm);
	if (! strm.eof())
	{
		DLLAddHUDMessage ("'%s' :  string \"%s\" is soiled", caller, player.c_str());
		return 0;
	}

	r = CoopMod_TeammateStatus_Get (&player_info, pnum, caller);
	if (! r)
		return 0;
	r = CoopMod_TeammateStatus_Check (player_info, 1, must_be_connected, "CoopMod_TeammateStatus_Get (string)");

	*player_info_out = player_info;

	return 1;
}

// + debug +

// *only for this player
// *goes before command dispatcher
void CoopMod_OnInputString (char* input_string)
{
	DLLAddHUDMessage ("OnInputString () req");
//	DLLAddHUDMessage ("input_string");

#if 0
	{
	FILE* f;

		f = fopen ("dbg2.txt", "ab");

		fprintf (f, input_string);
//		fwrite (input_string, 16, 1, f);
		fprintf (f, "---------------\r\n");

		fclose (f);
	}

//	DMFCBase->OnInputString (input_string);
#endif
}

void CoopMod_OnGetTokenString (char* src, char* dest, int dest_size)
{
	DLLAddHUDMessage ("OnGetTokenString () req");
//	DLLAddHUDMessage ("input_string");

	{
	FILE* f;

		f = fopen ("dbg3.txt", "ab");

		fprintf (f, src);
//		fwrite (input_string, 16, 1, f);
		fprintf (f, "---------------\r\n");

		fclose (f);
	}

	DMFCBase->OnGetTokenString (src, dest, dest_size);
}

OSIRISEXTERN Obj_Create_fp Obj_Create_Org;
//OSIRISEXTERN Obj_Create_fp CoopMod_Obj_Create;

//int CoopMod_Obj_Create (ubyte type,ushort id,int roomnum,vector *pos,const matrix *orient = 0,int parent_handle = OBJECT_HANDLE_NONE,vector *initial_velocity=0)
int CoopMod_Obj_Create (ubyte type,ushort id,int roomnum,vector *pos,const matrix *orient,int parent_handle,vector *initial_velocity)
{
FILE* f;

	f = fopen ("dbg5.txt", "ab");

	fprintf (f, "Obj_Create %02x, %04x", type, id);
	fprintf (f, "\r\n");

	fclose (f);

	return Obj_Create_Org (type, id, roomnum, pos, orient, parent_handle, initial_velocity);
}

OSIRISEXTERN Obj_CallEvent_fp Obj_CallEvent_Org;
//OSIRISEXTERN Obj_CallEvent_fp CoopMod_Obj_CallEvent;

bool CoopMod_Obj_CallEvent (int objnum,int event,tOSIRISEventInfo *ei)
{
	if (0)
	{
	FILE* f;

		f = fopen ("dbg6.txt", "ab");

		fprintf (f, "Obj_CallEvent %08x, %08x", objnum, event);
		fprintf (f, "\r\n");

		fclose (f);
	}

	return Obj_CallEvent_Org (objnum, event, ei);
}

OSIRISEXTERN MSafe_CallFunction_fp MSafe_CallFunction_Org;
//OSIRISEXTERN MSafe_CallFunction_fp CoopMod_MSafe_CallFunction;

void CoopMod_MSafe_CallFunction (int type, msafe_struct* mstruct)
{
FILE* f;

	f = fopen ("dbg7.txt", "ab");

	fprintf (f, "MSafe_CallFunction %08x", type);
	fprintf (f, "\r\n");

	fclose (f);

	MSafe_CallFunction_Org (type, mstruct);
}

void CoopMod_ConnectionActivity (int pnum)
{
	int net_address;
	byte*addr;

	if (! show_connection_activity)
		return;
	Assert(pnum >= 0 && pnum < DLLMAX_PLAYERS);

// negative
//	DMFCBase->CheckPlayerNum (pnum);

// *not works
#if 0
	bool r;
	player_info_t* player_info;
	r = CoopMod_TeammateStatus_Get (player_info, pnum, "ConnectionActivity ()");
	if (! r)
		return;
#endif

// *not works
	if (0)
	{
	player_record* pr;

		pr = DMFCBase->GetPlayerRecordByPnum(pnum);

		if (! pr)
		{
			DLLAddHUDMessage ("'CoopMod_ConnectionActivity ()' :  no player record for player #%d", pnum);
			return;
		}

		net_address = *(int*)pr->net_addr.address;

		DLLAddHUDMessage ("-player (#%d) %s connection activity-", pnum, pr->callsign);
	}

// *not works
	if (0)
	{
	player_record* pr = DMFCBase->GetPlayerRecord (0);
//	player_record* pr = DMFCBase->GetPlayerRecord (pnum);
	int i;

		if (! pr)
		{
			DLLAddHUDMessage ("-'CoopMod_ConnectionActivity ()' :  no player records-");
			return;
		}

		for (i = 0; i < MAX_PLAYER_RECORDS; i++)
		{
			if (pr->pnum == pnum)
				break;
			pr++;
		}

		if (i >= MAX_PLAYER_RECORDS)
		{
			DLLAddHUDMessage ("-was unable to find a player (#%d) in the player records array-", pnum);
			return;
		}

		net_address = *(int*)pr->net_addr.address;
		addr = (byte*)&net_address;
		int network_id = *(int*)pr->net_addr.net_id;

//		DLLAddHUDMessage ("-player (#%d) %s connection activity, state %d, IP %d.%d.%d.%d-", pnum, pr->callsign, pr->state, addr[0], addr[1], addr[2], addr[3]);
		DLLAddHUDMessage ("-player (#%d) connection activity, state %d, network_id %d, IP %d.%d.%d.%d-", pnum, pr->state, network_id, addr[0], addr[1], addr[2], addr[3]);
	}

	netplayer* netplayers = 0;
	network_address net_addr;
	char ip_str[128];
	ulong ip;
	int country_ix = 0;
	const char* country_name = 0;

	netplayers = DMFCBase->GetNetPlayers ();

// *I hope 'netplayer' is indexed the same way as 'player'
	net_addr = netplayers[pnum].addr;
	addr = (byte*)&net_addr;
// IP is ok, but 'flags' are wrong
//	DLLAddHUDMessage ("-flags %d, reliable_socket %d, sequence %d-", netplayers->flags, netplayers->reliable_socket, netplayers->sequence);
	sprintf (ip_str, "%u.%u.%u.%u:%u", addr[0], addr[1], addr[2], addr[3], net_addr.port);
	if (geoip_instance)
	{
		ip = addr[0] << 24;
		ip |= addr[1] << 16;
		ip |= addr[2] << 8;
		ip |= addr[3];
		country_ix = GeoIP_id_by_ipnum (geoip_instance, ip);
	}
	Assert(country_ix >= 0);
	Assert(country_ix < 253);
	country_name = GeoIP_country_name[country_ix];

//	DLLAddHUDMessage ("-player (#%d) connection activity, IP %s-", pnum, ip_str);
//	LogFile_Connections_Printf ("#%u, \"%s\"", pnum, ip_str);
// yes, two such printing places, I know; both of them are necessary for a while
	if (geoip_instance)
	{
		DLLAddHUDMessage ("- #%d, connection activity, %s [%s] -", pnum, ip_str, country_name);
//		LogFile_Connections_Printf ("#%u, %s [%s]", pnum, ip_str, country_name);
//		LogFile_Chat_Printf ("* pending #%u, %s [%s]", pnum, ip_str, country_name);
// comma after ip seems rises readability a bit
		LogFile_Connections_Printf ("#%u, %s, [%s]", pnum, ip_str, country_name);
		LogFile_Chat_Printf ("* pending #%u, %s, [%s]", pnum, ip_str, country_name);
	}
	else
	{
		DLLAddHUDMessage ("- #%d, connection activity, %s -", pnum, ip_str);
		LogFile_Connections_Printf ("#%u, %s", pnum, ip_str);
		LogFile_Chat_Printf ("* pending #%u, %s", pnum, ip_str);
	}
}

void CoopMod_ConnectionPacket (ulong ip, ushort port)
{
	int country_ix = 0;
	const char* country_name = 0;
	char ip_str[128];
	byte* addr;

	if (! show_connection_activity_packet)
		return;

	if (geoip_instance)
	{
		country_ix = GeoIP_id_by_ipnum (geoip_instance, ip);
	}
	Assert(country_ix >= 0);
	Assert(country_ix < 253);
	country_name = GeoIP_country_name[country_ix];

	addr = (byte*)&ip;
	sprintf (ip_str, "%u.%u.%u.%u:%u", addr[3], addr[2], addr[1], addr[0], port);
	if (geoip_instance)
	{
		DLLAddHUDMessage ("- connection packet, %s [%s] -", ip_str, country_name);
//		LogFile_Connections_Printf ("%s [%s]", ip_str, country_name);
//		LogFile_Chat_Printf ("* connecting %s [%s]", ip_str, country_name);
// comma after ip seems rises readability a bit
		LogFile_Connections_Printf ("%s, [%s]", ip_str, country_name);
		LogFile_Chat_Printf ("* connecting %s, [%s]", ip_str, country_name);
	}
	else
	{
		DLLAddHUDMessage ("- connection packet, %s -", ip_str);
		LogFile_Connections_Printf ("%s", ip_str);
		LogFile_Chat_Printf ("* connecting %s", ip_str);
	}
}

// - debug -


// + abnormal +
//// all abnormal stuff, like "rooms", is placed inside of here

void CoopMod_PowerupsDeleteAll ()
{
char command_name[] = "PowerupsDeleteAll";
int i;

	DLLAddHUDMessage ("removing of all powerups in the mine");

	object* objects = DMFCBase->GetObjects ();

	for (i = 0; i < MAX_OBJECTS; i++)
	{
		if (objects[i].type == OBJ_POWERUP)
		{
			if (objects[i].id >= 0x40)
				continue;

//			objects[i].type = OBJ_NONE;
			msafe_struct mstruct;
			mstruct.objhandle = objects[i].handle;
			mstruct.playsound = 0;
			MSafe_CallFunction (MSAFE_OBJECT_REMOVE, &mstruct);
		}
	}
}

void CoopMod_GB_Release_Frame ()
{
	object* obj;
	int id_old;
	int handle;

//	if (gb_released_handle == -1)
		return;

	msafe_struct mstruct;
	int room_num = 0;

//	DLLObjGet (gb_released_handle, &obj);
	obj = gb_released_obj;

	if (obj)
	{
		id_old = obj->id;
		obj->id = 13;
		handle = obj->handle;
	}
	else
	{
		DLLAddHUDMessage ("no object");
		return;
	}

//	mstruct.objhandle = gb_released_handle;
	mstruct.objhandle = handle;
	DLLAddHUDMessage ("handles :  old %08x, new %08x; types :  old %08x", gb_released_handle, handle, obj->type);

	mstruct.id = 4;
	MSafe_CallFunction (MSAFE_OBJECT_ID, &mstruct);
	mstruct.id = 15;
//	mstruct.playsound = 0;
//	MSafe_CallFunction (MSAFE_OBJECT_REMOVE, &mstruct);

//	MSafe_GetValue (MSAFE_OBJECT_ORIENT, &mstruct);
//	Room_Value (room_num, VF_GET, RMSV_V_PORTAL_PATH_PNT, &mstruct.pos, 0);
//	DLLAddHUDMessage ("new position is :  %f, %f, %f", mstruct.pos.x, mstruct.pos.y, mstruct.pos.z);
//	mstruct.roomnum = room_num;
//	MSafe_CallFunction (MSAFE_OBJECT_WORLD_POSITION, &mstruct);
//	DLLAddHUDMessage ("GB move");
	MSafe_GetValue (MSAFE_OBJECT_ID, &mstruct);

	DLLAddHUDMessage ("object id :  old %d, obj_id %d, new %d", id_old, obj->id, mstruct.id);

	gb_released_handle = -1;
	gb_released_obj = 0;
}

// cannot be handled, no event
void CoopMod_OnGB_Release (object* obj)
{
	Assert(obj);

	switch (coop_anti_guidebot_mode)
	{
	case 2:
	case 3:
		{
			object* parent_obj;

			DLLObjGet (obj->parent_handle, &parent_obj);

			if (! parent_obj)
			{
				DLLAddHUDMessage ("-Guidebot has no parent object-");
				break;
			}

			if (parent_obj->type == OBJ_PLAYER || parent_obj->type == OBJ_GHOST || parent_obj->type == OBJ_OBSERVER)
			{
// *on disconnect, a player is spewing gb too (OBJ_GHOST)
				if (DMFCBase->CheckPlayerNum (parent_obj->id))
					CoopMod_Kick (parent_obj->id, coop_anti_guidebot_mode == 2, "Guidebot release");
//				else
//					DLLAddHUDMessage ("-client %d is already disconnected-", parent_obj->id);
			}
			else
				DLLAddHUDMessage ("-Guidebot object was released not by a player, but by type %d, id %d object-", parent_obj->type, parent_obj->id);
		}
		break;
	}
}

// - abnormal -


void CoopMod_OnServerLevelStart (void)
{
	DMFCBase->OnServerLevelStart ();

	LogFile_Chat_Printf ("* level start");
	Missions_Recognize ();
	Powerups_Move ();
}

void CoopMod_OnServerLevelEnd (void)
{
	LogFile_Chat_Printf ("* level end");
	DMFCBase->OnServerLevelEnd ();
}

// *only important clean here, for preparing it for possible usage
void CoopMod_ClearStatusSlot (int i)
{
	player_info_t* player_info = &player_infos[i];

	player_info->used = 0;
//	player_info->connected = 0;

//	*(int*)player_info->net_address = 0;

//	player_info->time.connection = 0;
//	player_info->time.disconnection = 0;
//	player_info->time.play = 0;
//	player_info->time.kick = 0;
}

//// duplicate of the patcher definer, but I need it :/
void CoopMod_DefineVersion_Main (int& version_main)
{
	DWORD timestamp_main;

	timestamp_main = GetTimestampForLoadedLibrary ((HMODULE)0x00400000);
	switch (timestamp_main)
	{
// *this is German version, so perhaps is EU patched
	case 0x391b3158:
		version_main = VERSION_MAIN_14_XX;
		break;
	case 0x3919f324:
		version_main = VERSION_MAIN_14_NOCD;
		break;
	case 0x3bd44cf7:
		version_main = VERSION_MAIN_15_BETA;
		break;
	default:
		version_main = VERSION_MAIN_UNDEFINED;
	}
}

void CoopMod_Init ()
{
	bool r;
	int i;

	DMFCBase->Set_OnServerPlayerDisconnect (CoopMod_OnServerPlayerDisconnect);

	DMFCBase->Set_OnServerPlayerEntersGame (CoopMod_OnServerPlayerEntersGame);

	DMFCBase->Set_OnServerPlayerKilled (CoopMod_OnServerPlayerKilled);

	DMFCBase->Set_OnServerObjectShieldsChanged (CoopMod_OnServerObjectShieldsChanged);

//	DMFCBase->Set_OnControlMessage (CoopMod_OnControlMessage);

//	DMFCBase->Set_OnSpecialPacket (CoopMod_OnSpecialPacket);

// only for this player
//	DMFCBase->Set_OnInputString (CoopMod_OnInputString);

//	DMFCBase->Set_OnGetTokenString (CoopMod_OnGetTokenString);

	DMFCBase->Set_OnServerLevelStart (CoopMod_OnServerLevelStart);
	DMFCBase->Set_OnServerLevelEnd (CoopMod_OnServerLevelEnd);

	Commands_Init ();
	Cmd_Init ();
	r = CoopModCmd_Init ();
	A(r);
	Drop_Init ();

	DMFCBase->AddHUDItemCallback (HI_TEXT, DisplayHUDRoomnum);

	local_role = DMFCBase->GetLocalRole ();
	local_role_is_server = local_role == LR_SERVER;
	local_role_name = local_role_is_server ? "server" : "client";

// *got from "assault" mod
	tOSIRISModuleInit* OSInit = DMFCBase->GetOsirisModuleData ();
// initialize it once; if will need to include "../osiris/osiris_import.h", define '__OSIRIS_IMPORT_H_' before then
// (although actually it already is initialized)
	osicommon_Initialize (OSInit);
//	Obj_Create_Org = Obj_Create;
//	OSInit->fp[56] = (int*)CoopMod_Obj_Create;
//	Obj_CallEvent_Org = Obj_CallEvent;
//	OSInit->fp[3] = (int*)CoopMod_Obj_CallEvent;
//	MSafe_CallFunction_Org = MSafe_CallFunction;
//	OSInit->fp[1] = (int*)CoopMod_MSafe_CallFunction;

// yup, 0 should work
	objects_info = DMFCBase->GetObjectInfo (0);
	objects = DMFCBase->GetObjects ();
	players = DMFCBase->GetPlayers ();
	net_game = DMFCBase->GetNetgameInfo ();
// I don't see a function, with which I could get it in a client
// could be 'OnClientGameCreated ()', but the description of it is not clear
//	int own_pnum;
//	own_pnum = DMFCBase->GetPlayerNum ();
	ships = DMFCBase->GetShips ();

// no, this one does not contain address; have to get 'network_game' somehow
//	LogFile_DefineLogNames (net_game->server_address.port);
// wow, 'network_game' does not exist in memory at all; tested by 'name' and 'mission_name'
// the engine keeps the port number under top secret :)
// use pattern :  (8b 5c 24) 3c 56, 68 (hmmm, works on 1.4 NoCD and 1.5, but not works on EU version)
// and a bit below will be :  mov [005a5d04],ax
	ulong address_port;
	int version_main;
//// 'version_main' is undefined yet, so have to define self
// 'version_main' was not defined yet
	CoopMod_DefineVersion_Main (version_main);
	switch (version_main)
	{
	case VERSION_MAIN_14_XX:
		address_port = 0x005abd04;
		break;
	case VERSION_MAIN_14_NOCD:
		address_port = 0x005a5d04;
		break;
	case VERSION_MAIN_15_BETA:
		address_port = 0x005a6d40;
		break;
	default:
		Error ("CoopMod_Init () :  undefined \"main.exe\" version");
	}
	LogFile_DefineLogNames (*(ushort*)address_port);
// *in the current structure it will not be printed, because the "autoexec.dmfc" was not read yet and the log chatting is not enabled by default
// (not a big loss actually)
// *can be solved by use of own config
	LogFile_Chat_Printf ("* %s start, co-op mod version %s", local_role_name, COOPMOD_VERSION);

// clean up teammates data
// *only important initialization here, all other must be in 'OnServerPlayerEntersGame ()'
	for (i = 0; i < C_T_STATUSES_NUM; i++)
		CoopMod_ClearStatusSlot (i);

	for (i = 0; i < DLLMAX_PLAYERS; i++)
		player_info_ptrs[i] = 0;

//	DLLAddHUDMessage ("DLL Init. PPS is %d", net_game->packets_per_second);

// ok. this seems helps against Mercenary level 5 big door in room 3 bug; but I don't think this is a proper way
#if 0
#ifdef WIN32
// I often see accesses at this area, after the game has a crash; so lets hack it; in 0x06000000-0x07000000 area I think nothing is allocated
//// examples :  0x061cc5c8
//// but moves to 0x0675c5c8
// examples :  0x061cc5c8 + 0x0675c5c8
// *request after those 'DMFCBase->... ()', because they also do allocate some memory, so their memory then appears after this block
#define MEM_AREA_1 0x06100000
//#define MEM_AREA_SIZE_1 0x00100000
#define MEM_AREA_SIZE_1 0x00700000
	malloc06100000 = VirtualAlloc ((void*)MEM_AREA_1, MEM_AREA_SIZE_1, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (malloc06100000)
	{
		if ((ulong)malloc06100000 != MEM_AREA_1)
			DLLAddHUDMessage ("VirtualAlloc (%08x) unsucceed; returned another address %08x", MEM_AREA_1, malloc06100000);
		else
			DLLAddHUDMessage ("VirtualAlloc (%08x) succeed", MEM_AREA_1);
	}
	else
		DLLAddHUDMessage ("VirtualAlloc (%08x) has failed", MEM_AREA_1);
#endif
#endif

// *moved closer at the entry point
//	Exceptions_Init ();
// exceptions test
#if 0
// hmmm
//	__asm mov al,byte ptr [0x06123456]
	__asm mov eax,0x06123456
	__asm mov eax,0x061fffff
	__asm mov al,[eax]
#endif
// test
// ordinar crap :  dll code duplicates exe
// or perhaps not; engine gets the colour through the event EVT_CLIENT_GETCOLOREDNAME 0x0623
//	DMFCBase->GetTeamColor (0);

	Rooms_GameStart ();
//	Hooks_Init ();
	geoip_instance = GeoIP_new (GEOIP_STANDARD | GEOIP_MEMORY_CACHE);
	if (geoip_instance)
		DLLAddHUDMessage ("GeoIP initialized");
	else
		DLLAddHUDMessage ("- GeoIP init fail -");

// clock on my WinXP is running 2-2.5 times faster, due to modification to multimedia timer; fix it :  set the period now, so that it wouldn't be changed each time between frames
// (there still be timeBeginPeriod()/timeEndPeriod() requests, but they will do nothing)
#ifdef WIN32
	MMRESULT mmresult;
	mmresult = timeBeginPeriod (MM_TIMER_PERIOD);
#endif

// *there is no unregister
	DMFCBase->RegisterPacketReceiver (PACKTYPE_CMDTOSERVER, Packet_Receiver_Cmd_ToServer);
	DMFCBase->RegisterPacketReceiver (PACKTYPE_CMDTOCLIENT, Packet_Receiver_Cmd_ToClient);
	DMFCBase->RegisterPacketReceiver (PACKTYPE_DROPWEAPON_SETAMMO_TOCLIENT, Packet_Receiver_DropWeapon_SetAmmo_ToClient);
}

void CoopMod_Close ()
{
#ifdef WIN32
//	BOOL B_r;
	MMRESULT mmresult;

	mmresult = timeEndPeriod (MM_TIMER_PERIOD);
#endif

	Rooms_GameFinish ();
	GeoIP_delete (geoip_instance);

#if 0
#ifdef WIN32
	if (malloc06100000)
	{
// 'VirtualFree ()' is promised to fail on MEM_RELEASE with non zero size arg; and with zero - seems fails MEM_DECOMMIT
//		B_r = VirtualFree (malloc06100000, MEM_AREA_SIZE_1, MEM_DECOMMIT | MEM_RELEASE);
		B_r = VirtualFree (malloc06100000, MEM_AREA_SIZE_1, MEM_DECOMMIT);
		B_r = VirtualFree (malloc06100000, 0, MEM_RELEASE);
	}
#endif
#endif

//	Exceptions_Quit ();
}


// +++ indication +++

void CoopMod_SendMessage_ (int pnum, int color, char* sound, const char* format, va_list args)
{
	msafe_struct mstruct;

	vsnprintf(mstruct.message, sizeof(mstruct.message) - 1, format, args);
// if message too long, vsnprintf () won't terminate
	mstruct.message[sizeof(mstruct.message) - 1] = 0;

	if (pnum >= DLLMAX_PLAYERS)
		return;

	mstruct.color = color;
	if (pnum >= 0)
	{
// *enough only objnum for this case
#if 1
		int objnum;
		int objhandle;

// does someone know, what have I done here ?
		objnum = players[pnum].objnum;
		objhandle = objects[objnum].handle;

		mstruct.objhandle = objhandle;
// hmmm, it doesn't work from here
#else
		player_info_t* player_info;

		player_info = player_info_ptrs[pnum];
		mstruct.objhandle = player_info->object_ptr->handle;
#endif

// means specific player
		mstruct.state = 1;
	}
	else
	{
// means all players
		mstruct.state = 0;
	}

//	while (&mstruct == &mstruct);
//	DLLAddHUDMessage ("-a probe-");

	MSafe_CallFunction (MSAFE_MISC_HUD_MESSAGE, &mstruct);

	if (sound)
	{
// to all players
//		mstruct.state = 0;
		mstruct.index = Scrpt_FindSoundName (sound);
		mstruct.volume = 1.0;

		MSafe_CallFunction (MSAFE_SOUND_2D, &mstruct);
	}

// msg to pnum 0 is not being printed in dedi console; fix it
	if (local_role_is_server)
		if (! pnum)
//		if (DMFCBase->IsPlayerDedicatedServer (pnum))
			DLLAddHUDMessage ("%s", mstruct.message);
}

void CoopMod_SendMessage_Colored (int pnum, int color, char* format, ...)
{
	va_list args;

	va_start(args, format);
	CoopMod_SendMessage_ (pnum, color, 0, format, args);
	va_end(args);
}

void CoopMod_SendMessage (int pnum, char* format, ...)
{
//	const char* sound;
	char* sound;
	va_list args;

	va_start(args, format);
	sound = 0;
// no
//	sound = "droplet2";
// no
//	sound = "droplet2.wav";
// yes
//	sound = "RainDrop";
// no
//	sound = "briefingtype1";
// no
//	sound = "briefingtype1.wav";
// yes (looped sound)
//	sound = "briefingtype";
// yes
//	sound = "Siren";
	if (pnum != -1)
		CoopMod_SendMessage_ (pnum, GR_RGB(128,96,160), sound, format, args);
	else
		CoopMod_SendMessage_ (pnum, GR_RGB(128,128,128), sound, format, args);
	va_end(args);
}


// + indication commands +

#define STRING_BUF_SIZE 1024
char string_buf[STRING_BUF_SIZE];

void CoopMod_ShowDamageStatusTotally (player_info_t* player_info, int whom)
{
	bool r;
	float coefficient;
	char temp_buf[300];

	Assert(player_info);
	if (whom != -1)
		r = CoopMod_TeammateStatus_Get (0, whom, "DamageStatusTotally (_whom_)");
		if (! r)
		{
			DLLAddHUDMessage ("'CoopMod_ShowDamageStatusTotally ()' :  invalid receptor (%d) index", whom);
			return;
		}

	*string_buf = 0;
	sprintf (temp_buf, "Total damage status for %s is :  ", player_info->callsign);
	strcat (string_buf, temp_buf);
	sprintf (temp_buf, "to robots %.0f, ", player_info->damage.to_robots_total);
	strcat (string_buf, temp_buf);
	sprintf (temp_buf, "to teammates %.0f, ", player_info->damage.to_team_total);
	strcat (string_buf, temp_buf);

// except division by zero
	if (player_info->damage.to_team_total >= 10.0)
	{
		coefficient = player_info->damage.to_robots_total / player_info->damage.to_team_total;

		sprintf (temp_buf, "coefficient is %.1f, ", coefficient);
	}
	else
		sprintf (temp_buf, "coefficient ---, ");

	strcat (string_buf, temp_buf);

	sprintf (temp_buf, "continuous hits %d, ", player_info->teamdamage_long.count);
	strcat (string_buf, temp_buf);
	sprintf (temp_buf, "filtered %d", player_info->teamdamage_long.count_filtered);
	strcat (string_buf, temp_buf);

	CoopMod_SendMessage (whom, string_buf);
}

const char* CoopMod_PrintIPPort (player_info_t* player_info)
{
	static char buf[100];
//	ulong net_address;
	byte* addr;
//	player_record* pr;

	Assert(player_info);
	buf[0] = 0;
#if 0
	pr = DMFCBase->GetPlayerRecordByPnum (player_num);
	net_address = *(ulong*)pr->net_addr.address;
	addr = (byte*)&net_address;
	sprintf (buf, "%d.%d.%d.%d:%d", addr[0], addr[1], addr[2], addr[3], pr->net_addr.port);
#endif
	addr = player_info->net_address.ip_b;
	sprintf (buf, "%d.%d.%d.%d:%d", addr[0], addr[1], addr[2], addr[3], player_info->net_address.port);
	return buf;
}

const char* CoopMod_GetCountry (player_info_t* player_info)
{
//	net_address = *(int*)pr->net_addr.address;
	byte* addr;
//	char address_str[64];
	const char* country_name = 0;
	int country_ix = 0;
	ulong ip;

	if (! geoip_instance)
		return 0;

// convert bytes order
	addr = player_info->net_address.ip_b;
	ip = addr[0] << 24;
	ip |= addr[1] << 16;
	ip |= addr[2] << 8;
	ip |= addr[3];
//	country_ix = GeoIP_id_by_ipnum (geoip_instance, *(ulong*)player_info->net_address.ip);
	country_ix = GeoIP_id_by_ipnum (geoip_instance, ip);
	Assert(country_ix >= 0);
	Assert(country_ix < 253);
	country_name = GeoIP_country_name[country_ix];
	return country_name;
}

void CoopMod_ShowPlayerInfo (player_info_t* player_info, int whom)
{
	bool r;
//	float coefficient;
//	char temp_buf[300];
	const char* address;
	const char* country_name = 0;

	Assert(player_info);
	if (whom != -1)
		r = CoopMod_TeammateStatus_Get (0, whom, "ShowPlayerInfo (_whom_)");
		if (! r)
		{
			DLLAddHUDMessage ("'CoopMod_ShowPlayerInfo ()' :  invalid receptor (%d) index", whom);
			return;
		}

	*string_buf = 0;
	address = CoopMod_PrintIPPort (player_info);
// eeehh, backward calculation of the slot ix :/
	if (geoip_instance)
	{
		country_name = CoopMod_GetCountry (player_info);
		sprintf (string_buf, "%d, #%d, %s [%s], \"%s\"", player_info - player_infos, player_info->pnum, address, country_name, player_info->callsign);
	}
	else
		sprintf (string_buf, "%d, #%d, %s, \"%s\"", player_info - player_infos, player_info->pnum, address, player_info->callsign);

	CoopMod_SendMessage (whom, string_buf);
}

void CoopMod_DamageStatusTotally (int whos, int whom)
{
	bool r;
	player_info_t* player_info;

	r = CoopMod_TeammateStatus_Get (&player_info, whos, "DamageStatusTotally (_whos_)");
	if (! r)
		return;

	CoopMod_ShowDamageStatusTotally (player_info, whom);
}

// *connection check not added, because wasn't required yet
bool CoopMod_CheckStatusIndex (int index, int whom)
{
	player_info_t* player_info = &player_infos[index];

// check - does this client has ever been connected
	if (! player_info->used)
	{
		if (whom > 0)
//			DLLAddHUDMessage ("'CoopMod_DamageStatusTotallyIndexed ()' :  clent with that (%d) index has never been connected", index);
			DLLAddHUDMessage ("CoopMod_CheckStatusIndex () :  slot with index %d wasn't in use", index);
// I see no need to inform what command it was
//		CoopMod_SendMessage (whom, "'CoopMod_DamageStatusTotallyIndexed ()' :  clent with that (%d) index has never been connected", index);
		CoopMod_SendMessage (whom, "slot with index %d wasn't in use", index);
		return 0;
	}

	return 1;
}

void CoopMod_DamageStatusTotallyIndexed (int whos_index, int whom)
{
	player_info_t* player_info;

// check - does this client had ever connected
	if (! CoopMod_CheckStatusIndex (whos_index, whom))
		return;

	player_info = &player_infos[whos_index];

	CoopMod_ShowDamageStatusTotally (player_info, whom);
}

void CoopMod_PlayerInfoIndexed (int whos_index, int whom)
{
	player_info_t* player_info;

// check - does this client had ever connected
	if (! CoopMod_CheckStatusIndex (whos_index, whom))
		return;

	player_info = &player_infos[whos_index];

	CoopMod_ShowPlayerInfo (player_info, whom);
}

void CoopMod_ShowPlayerPPS (int player_num, int whom)
{
	bool r;
	netplayer* netplayers = 0;
	u32 pps;
	u32 pps_original;
	player_info_t* player_info;

	r = CoopMod_TeammateStatus_Get (&player_info, player_num, "ShowPlayerPPS ()");
	if (! r)
		return;

	netplayers = DMFCBase->GetNetPlayers ();

// *I hope 'netplayer' is indexed the same way as 'player'
	pps = netplayers[player_num].pps;
	pps_original = player_info->pps_original;

//	CoopMod_SendMessage (whom, "player \"%s\" (#%d) has %d PPS (%d is min allowed)", player_info ? player_info->callsign : "unknown", player_num, pps, coop_pps_min);
	CoopMod_SendMessage (whom, "player \"%s\" (#%d) has %d PPS in settings (%d is min allowed), server output to him is %d PPS", player_info ? player_info->callsign : "<unknown>", player_num, pps_original, coop_pps_min, pps);
}

void CoopMod_ShowMissionInfo ()
{
	Assert(mission_name);
	DLLAddHUDMessage ("mission name is \"%s\", index in table is %d%s, level %d", mission_name, mission_index, mission_index == MISSION_NONE ? " (none)" : (mission_index == MISSION_UNKNOWN ? " (unknown)" : ""), level_num);
}

bool CoopMod_ReadIp (ulong& ip_out, const char* str)
{
	stringstream strm;
	u32 i;
	byte ip[4];
// cannot be byte, '>>' understands it as a character
//	byte ip_byte;
	u32 ip_byte;
	char c;

	if (! *str)
	{
		DLLAddHUDMessage ("no IP is given");
		return 0;
	}
	strm << str;
	for (i = 0; i < 4; i++)
	{
		strm >> ip_byte;
// *'stringstream' flags are messed a bit
// eof appears at a last number read also
#if 0
		if (strm.eof())
//		if (i < 3 && strm.eof())
		{
			if (i >= 3 && ! strm.fail())
				break;
			DLLAddHUDMessage ("string \"%s\" as IP, ended on number %d", str, i);
			return 0;
		}
#else
// 'fail' will not appear without 'eof' as I understand
		if (strm.eof() && strm.fail())
// no number
		{
			if (i >= 3 && ! strm.fail())
				break;
			DLLAddHUDMessage ("string \"%s\" as IP, no number %d", str, i);
			return 0;
		}
#endif
// if eof, then fail is also set
		if (strm.fail())
		{
			DLLAddHUDMessage ("string \"%s\" as IP, number %d cannot be read", str, i);
			return 0;
		}
		ip[3 - i] = (byte)ip_byte;
		if (i >= 3)
			break;
// this does not produce eof, if it is the last symbol
		strm >> c;
//		if (i >= 3)
//			break;
		if (strm.eof())
		{
//			DLLAddHUDMessage ("string \"%s\" as IP, ended before number %d", str, i + 1);
			DLLAddHUDMessage ("string \"%s\" as IP, no dot after number %d", str, i);
			return 0;
		}
		if (strm.fail())
		{
			DLLAddHUDMessage ("string \"%s\" as IP, fail after number %d", str, i);
			return 0;
		}
		if (c != '.')
		{
			DLLAddHUDMessage ("string \"%s\" as IP, separator after number %d isn't dot", str, i);
			return 0;
		}
	}
	if (! strm.eof())
	{
		DLLAddHUDMessage ("string \"%s\" as IP, more characters after IP end", str);
		return 0;
	}
	ip_out = *(ulong*)ip;
	return 1;
}

void CoopMod_ShowGeoip (const char* str)
{
	bool r;
//	byte ip[4];
	ulong ip;
	const char* country_name = 0;
	int country_ix = 0;

	if (! geoip_instance)
	{
		DLLAddHUDMessage ("GeoIP instance isn't initialized");
		return;
	}

//	DLLAddHUDMessage ("string \"%s\"", str);
	r = CoopMod_ReadIp (ip, str);
	if (! r)
	{
//		DLLAddHUDMessage ("string \"%s\" cannot be read as IP", str);
		return;
	}

	country_ix = GeoIP_id_by_ipnum (geoip_instance, ip);
	Assert(country_ix >= 0);
	Assert(country_ix < 253);
	country_name = GeoIP_country_name[country_ix];
	DLLAddHUDMessage ("%s [%s]", str, country_name);
}

// - indication commands -


void DisplayHUDRoomnum (struct tHUDItem* hitem)
{
	if (! show_room_num)
		return;

	int pnum = DMFCBase->GetPlayerNum ();
// *'player_info' cannot be used here if it is a client
//	player_info_t* player_info = player_info_ptrs[pnum];
//	assert(player_info);
	int objnum = players[pnum].objnum;
	DLLgrtext_SetColor (GR_GREEN);
	DLLgrtext_SetAlpha (0x80);
//	DLLgrtext_Printf (10, 1, "room :  %d", player_info->object_ptr->roomnum);
// *don't know about the mask for sure
	DLLgrtext_Printf (10, 35, "room :  %s %d", objects[objnum].roomnum & 0x80000000 ? "O" : "I", CELLNUM(objects[objnum].roomnum) & 0x00000fff);
}

void CoopMod_OnGB_Release_Notify (int pnum)
{
	bool r;
	player_info_t* player_info;

	r = CoopMod_TeammateStatus_Get (&player_info, pnum, "OnGB_Release_Notify ()");
	if (r)
// not for the guidebot only, actually
		CoopMod_SendMessage (-1, "blocking AI scripts usage for %s", player_info->callsign);
}

// --- indication ---


// team damage continuously
void CoopMod_TeamDamageLong (player_info_t* player_info, object* obj_victim, float amount)
{
// we need only player attack info
	Assert(player_info);
	Assert(obj_victim->type == OBJ_PLAYER || obj_victim->type == OBJ_ROBOT);
	Assert(player_info->connected);

	float time = DMFCBase->GetGametime ();

	if (obj_victim->type == OBJ_ROBOT)
	{
//		DLLAddHUDMessage ("%d attacked robot", obj_it->id);
// reset the counter
		player_info->teamdamage_long.count = 0;
		player_info->teamdamage_long.count_filtered = 0;
		player_info->teamdamage_long.last_robot_damage_time = time;
//		player_info->teamdamage_long.first_teamdamage_time = 0.0;
//		player_info->teamdamage_long.last_teamdamage_time = time;
	}
	else if (obj_victim->type == OBJ_PLAYER)
	{
	int teamdamage_long_counts_limit;
	int teamdamage_long_counts_filtered_limit;

// division by zero
		Assert(player_info->damage.to_team_total);

		if (time < player_info->teamdamage_long.last_robot_damage_time + coop_teamdamage_robotdamage_post_time)
// block all countings, for some time
			return;

// + get counts limits +
// *constant values makes kick for players who shot napalm missile, and teammate gone into it's flames,
// not depending - are they playing fair or not, so lets better calculate count limit from their statistics
// *vauss may produce occasional kick, and few other cases exist too
		teamdamage_long_counts_limit = player_info->damage.to_robots_total / player_info->damage.to_team_total / coop_teamdamage_long_coeficient;
		teamdamage_long_counts_filtered_limit = coop_teamdamage_long_filtered;

// bring the limit inside the range
		if (teamdamage_long_counts_limit < coop_teamdamage_long_min)
			teamdamage_long_counts_limit = coop_teamdamage_long_min;
		if (teamdamage_long_counts_limit > coop_teamdamage_long_max)
			teamdamage_long_counts_limit = coop_teamdamage_long_max;
// - get counts limits -

// + filtered +
		if (time >= player_info->teamdamage_long.last_teamdamage_time + coop_teamdamage_filter_time)
		{
			player_info->teamdamage_long.count_filtered++;
			player_info->teamdamage_long.last_teamdamage_time = time;
		}
// - filtered -

// + not filtered +
		if (! player_info->teamdamage_long.count)
// first damage, start timer from here
			player_info->teamdamage_long.first_teamdamage_time = time;

		player_info->teamdamage_long.count++;
//		DLLAddHUDMessage ("player %d attacked player %d times", obj_it->id, player_info->teamdamage_long.count);
// - not filtered -

// reset calmdown timer
		player_info->teamdamage_long.last_calmdown_time = time;

		if (auto_kick__team_damage_long)
		{
			if (time - player_info->teamdamage_long.first_teamdamage_time >= coop_teamdamage_long_period)
// not in a small amount of time
			{
				if (player_info->teamdamage_long.count >= teamdamage_long_counts_limit)
// **wrong pnum, but needs to be debugged
// event :  autokick happened to a wrong one player.
// test 1 :  mzero'es dedi; "i" has shown two players on the server, I've joined, got slot #2, while mzero got slot #1 and Ivan got slot #3;
// after I have flared mzero few times, got proper autokick.
					CoopMod_Kick (player_info->object_ptr->id, 1, "teammate attacking %d times (%d is max)", player_info->teamdamage_long.count, teamdamage_long_counts_limit);

				if (player_info->teamdamage_long.count_filtered >= teamdamage_long_counts_filtered_limit)
					CoopMod_Kick (player_info->object_ptr->id, 1, "teammate attacking %d filtered times (%d is max)", player_info->teamdamage_long.count_filtered, teamdamage_long_counts_filtered_limit);
			}
		}
	}
}

// team damage statistically and currently
// *must work only if there are more active and fighting players
void CoopMod_DamageToObject (player_info_t* player_info, object* obj_victim, float amount)
{
// we need only player attack info
	Assert(player_info);
	Assert(obj_victim->type == OBJ_PLAYER || obj_victim->type == OBJ_ROBOT);

	if (obj_victim->type == OBJ_ROBOT)
	{
		player_info->damage.to_robots_total += amount;
	}
	else if (obj_victim->type == OBJ_PLAYER)
	{
//	float time = DMFCBase->GetGametime ();

		player_info->damage.to_team_total += amount;
	}
}

// returns 0 if client is kicked
bool CoopMod_ObjectShieldsChange (object* obj, float amount)
{
	bool r;
	player_info_t* player_info;

//	DMFCBase->OnServerObjectShieldsChanged (obj, amount);

// just bug protection
	if (! obj)
		return 1;

	if (! amount)
// if someone burning is touching another, and give him damage by it, the one gives zero damage back too
		return 1;

	if (obj->type != OBJ_PLAYER && obj->type != OBJ_ROBOT)
		return 1;

	dllinfo* Data = DMFCBase->GetDLLInfoCallData ();
	object* obj_it;

	if (obj->handle == Data->it_handle)
// hit itself
		return 1;

	DLLObjGet (Data->it_handle, &obj_it);

// *sometimes it is 0
	if (! obj_it)
		return 1;

	if (obj_it->type != OBJ_PLAYER)
// we need only player attack info
		return 1;

	if (obj_it->id < 0)
	{
		DLLAddHUDMessage ("Error :  'obj_it->id < 0' (%d) is in 'CoopMod_ObjectShieldsChange ()'", obj_it->id);
		return 1;
	}
	else if (obj_it->id >= DLLMAX_PLAYERS)
	{
		DLLAddHUDMessage ("Error :  'obj_it->id >= DLLMAX_PLAYERS' (%d) is in 'CoopMod_ObjectShieldsChange ()'", obj_it->id);
		return 1;
	}

	r = CoopMod_TeammateStatus_Get (&player_info, obj_it->id, "ObjectShieldsChange (_attacker_)");
	if (! r)
		return 1;

	bool is_kicked = !! player_info->time.kick;

	CoopMod_DamageToObject (player_info, obj, amount);
// 'DamageToObject ()' must be before, to prevent division by zero
	CoopMod_TeamDamageLong (player_info, obj, amount);

	return ! is_kicked;
}

#define DAMAGE_AMOUNT_MAX 10000.0

void CoopMod_OnServerObjectShieldsChanged (object* obj, float amount)
{
	if (amount > DAMAGE_AMOUNT_MAX)
	{
// it is normal; often appears with my fusion cannon; don't know what it means
#if 0
//		DLLAddHUDMessage ("Warning 'CoopMod_OnServerObjectShieldsChanged ()' :  damage amount is %01f (more than %01f), stopping event execution", amount, DAMAGE_AMOUNT_MAX);
// that will be much funny :)
// but too long string, since it happens
//		CoopMod_SendMessage (-1, "Warning 'CoopMod_OnServerObjectShieldsChanged ()' :  damage amount is %01f (more than %01f), stopping event execution", amount, DAMAGE_AMOUNT_MAX);
		CoopMod_SendMessage (-1, "Warning, CoopMod :  damage amount is %01f (more than %01f), stopping event execution", amount, DAMAGE_AMOUNT_MAX);
		return;
#else
		goto finish;
#endif
	}

// again, - for debug, not for the gameplay :)
// why not works ?
//	amount *= 10.0;

	if (! CoopMod_ObjectShieldsChange (obj, amount))
// damage from kicked client
	{
		amount /= coop_kicked_divide_damage;
		if (amount > coop_kicked_damage_max)
			amount = coop_kicked_damage_max;
	}

finish:
	DMFCBase->OnServerObjectShieldsChanged (obj, amount);
}

void CoopMod_OnInterval_Players (void)
{
	int i;
	float time = DMFCBase->GetGametime ();

/*
	static int j = 0;

	j++;
	if (! (j % 10))
	{
		index = player_info_indexes[1];
		if (index >= 0)
		{
		FILE* f;
		int k;
		object_info* obj_info = DMFCBase->GetObjectInfo (player_infos[index].object_ptr->id);

			f = fopen ("dbg4.txt", "ab");

//			if (objects_ != objects)
//				fprintf (f, "object");
//			if (players_ != players)
//				fprintf (f, "player");
//			if (objnum != player_infos[index].object_index)
//				fprintf (f, "objnum");
//			fprintf (f, "-----\r\n");

			fwrite (player_infos[index].object_ptr, sizeof(object), 1, f);
			for (k = 0; k < (0x400 - sizeof(object)) / 8; k++)
				fprintf (f, "--------");
//			fwrite (obj_info, sizeof(object_info), 1, f);
//			for (k = 0; k < (0x200 - sizeof(object_info)) / 8; k++)
//				fprintf (f, "--------");

			fclose (f);
		}
	}
*/

	for (i = 0; i < DLLMAX_PLAYERS; i++)
	{
		player_info_t* player_info = player_info_ptrs[i];
		if (! player_info)
			continue;
		(players[i].flags & PLAYER_FLAGS_THRUSTED);
//		float dist = vm_VectorDistanceQuick (&player_infos[index].object_ptr->pos, &player_infos[index].object_ptr->last_pos);
// *speed for Pyro is about 62.0
//		float speed = vm_GetMagnitudeFast (&player_infos[index].object_ptr->mtype.phys_info.velocity);
//		float thrust = vm_GetMagnitudeFast (&player_infos[index].object_ptr->mtype.phys_info.thrust);
//		float last_still_time = player_infos[index].object_ptr->mtype.phys_info.last_still_time;
//		float movement_scalar = players[i].movement_scalar;
//		int thruster_sound_state = players[i].thruster_sound_state;
//		int some_flag = player_infos[index].object_ptr->flags & OF_MOVED_THIS_FRAME;
//		CoopMod_SendMessage (i, "speed %f     thrust %f     last_still_time %f", speed, thrust * 1000000000000, last_still_time * 1000000000000);
//		CoopMod_SendMessage (i, "%s movement_scalar %f", players[i].callsign, movement_scalar);
//		if (thrust > 0.0)
//			CoopMod_SendMessage (i, "non zero thrust");
//		if (last_still_time > 0.0)
//			CoopMod_SendMessage (i, "non zero last_still_time");
//		CoopMod_SendMessage (i, "%s thruster_sound_state %f", players[i].callsign, thruster_sound_state);
//		if (some_flag)
//			CoopMod_SendMessage (i, "flag is set");
//		CoopMod_SendMessage (i, "flags :  %08x", player_infos[index].object_ptr->flags);
//		CoopMod_SendMessage (i, "flags :  %08x", players[1].flags);

		if (player_info->teamdamage_long.count)
// teamdamage_long calmdown
			if (player_info->teamdamage_long.last_calmdown_time + coop_teamdamage_calmdown_time <= time)
			{
				player_info->teamdamage_long.last_calmdown_time = time;
				player_info->teamdamage_long.count--;
			}

		if (player_info->teamdamage_long.count_filtered)
// teamdamage_long filtered calmdown
			if (player_info->teamdamage_long.last_calmdown_filtered_time + coop_teamdamage_calmdown_filtered_time <= time)
			{
				player_info->teamdamage_long.last_calmdown_filtered_time = time;
				player_info->teamdamage_long.count_filtered--;
			}
	}
}

void CoopMod_OnInterval_Objects (void)
{
int i;
int j = 0;
int k = 0;

	for (i = 0; i < MAX_OBJECTS; i++)
	{
// *keys are 'OBJ_POWERUP'
		if (objects[i].type == OBJ_POWERUP)
//		if (objects[i].type == OBJ_POWERUP || objects[i].type == OBJ_CLUTTER)
//		if (objects[i].type == OBJ_POWERUP || objects[i].type == OBJ_CLUTTER || objects[i].type == OBJ_MISC)
//		if (objects[i].type == OBJ_NONE)
		{
			objects[i].type = OBJ_NONE;
//			j++;
//			objects[i].creation_time = 1.0;
//			objects[i].lifeleft = 2.0;
//			objects[i].lifetime = 4.0;
//			msafe_struct mstruct;
//			mstruct.objhandle = objects[i].handle;
//			MSafe_CallFunction (MSAFE_OBJECT_GHOST, &mstruct);
//			MSafe_CallFunction (MSAFE_OBJECT_UNGHOST, &mstruct);
/*
	{
	FILE* f;
//	object_info* obj_info = DMFCBase->GetObjectInfo (objects[i].handle);
	object_info* obj_info = DMFCBase->GetObjectInfo (objects[i].id);

		f = fopen ("dbg2.txt", "ab");

//		if (obj_info->type == OBJ_POWERUP)
			fprintf (f, "%04x %f\r\n", obj_info->type, obj_info->respawn_scalar);
//		else
//			fprintf (f, "not powerup\r\n");

		fclose (f);
	}
*/
		}
	}

//loop:
//	goto loop;

//	for (i = 0; i < MAX_OBJECTS; i++)
//		if (objects[i].type == OBJ_GHOST)
//			k++;

//	DLLAddHUDMessage ("objects :  %d, %d", j, k);
}

void CoopMod_OnInterval_Kick (void)
{
int i;
// *for the next time
int teamkillers_found = 0;
player_info_t* player_info;

	float time = DMFCBase->GetGametime ();

// find teamkillers in teammates data
	for (i = 0; i < DLLMAX_PLAYERS; i++)
	{
		player_info = player_info_ptrs[i];
		if (! player_info)
			continue;

		if (! player_info->connected)
			continue;

		if (! player_info->time.kick)
			continue;
// needs to be kicked

		if (time >= player_info->time.kick)
// time to kick
		{
#if 0
			char str[20];

			sprintf (str, "$kick %d", i);
#endif
// kick
//			DMFCBase->CallClientEvent (EVT_CLIENT_GAMEPLAYERDISCONNECT, player_info->object_index, player_info->object_index, i);
			DLLMultiDisconnectPlayer (i);
			DLLAddHUDMessage ("%s has been successfully kicked :)", player_info->callsign);

// **is the 'CoopMod_OnServerPlayerDisconnect ()' requested later ?
// finish punishment
			player_info->time.kick = 0.0;
		}
		else
			teamkillers_found = 1;
	}

	if (! teamkillers_found)
// stop teamkillers sequence
		coop_accel_kick = 0;
}

// mzero has diminished her server to Rookie difficulty;
// lets try to compensate it, trying to make robots think/move/shoot faster :)
void CoopMod_DistortFrame ()
{
	static float gametime_old = 0.0;
	float gametime;
	float* gametime_ptr;
	float frametime;
	float* frametime_ptr;
//	float* f;
// impossible to check :/
//	static bool lockout = 0;
	float delta;

//	if (lockout)
// do not enter, an improper version was detected earlier
//		return;
// 'class IDMFC' source is incomplete; it has access at the frametime variable, but have to check it first, that it still is on the same position
// though right after "NEW INTERFACE STUFF" in "idmfc.h", there goes those variables, commented.
//	frametime = DMFCBase->GetFrametime ();
//	gametime = DMFCBase->GetGametime ();
	frametime_ptr = *(float**)&((byte*)DMFCBase)[0x28];
	gametime_ptr = *(float**)&((byte*)DMFCBase)[0x2c];
#if 0
	frametime /= 2.0;
	*frametime_ptr = frametime;
	*gametime_ptr -= frametime;
#endif

	gametime = *gametime_ptr;
	frametime = *frametime_ptr;
	delta = gametime - gametime_old;
// limit it
	if (delta < 0.0)
		goto finish;
	if (delta > 1.0)
		goto finish;
	if (frametime < 0.0)
		goto finish;
	if (frametime > 1.0)
		goto finish;
// *and do not distort, if the server lags (actually matters, only if rising the speed)
//	if (delta > 0.05)
//		delta = 0.05;
	if (delta > 0.05 && coopmod_time_distortion > 1.0)
		goto finish;
	delta *= coopmod_time_distortion;
	gametime = gametime_old + delta;
	frametime *= coopmod_time_distortion;
	*gametime_ptr = gametime;
	*frametime_ptr = frametime;
finish:
	gametime_old = gametime;
}

// *yup, much test trash is here at the beginning
void CoopMod_OnInterval (void)
{
// test
//	Screen_Restore ();
//	DLLfvi_FindIntersection (0, 0, 0);
//	DMFCBase->GetNumTeams ();
//	DMFCBase->GetShips ();
//	AI_IsObjEnemy (0, 0);
#if 0
	u32 pnum;
	object* obj;
	angvec av;
	float p, h, b;
	pnum = players[0].objnum;
	obj = &objects[pnum];
//	DLLvm_AnglesToMatrix (&m, 0, 0, 0);
	DLLvm_ExtractAnglesFromMatrix (&av, &obj->orient);
	p = (s16)av.p / 65536.0;
	h = (s16)av.h / 65536.0;
	b = (s16)av.b / 65536.0;
	DLLAddHUDMessage ("%f, %f, %f, %f, %f, %f", obj->orient.fvec.x, obj->orient.fvec.y, obj->orient.fvec.z, p, h, b);
#endif
#if 0
	netgame_info* netgame;
	ushort port;
	netgame = DMFCBase->GetNetgameInfo ();
	port = netgame->server_address.port;
#endif
// dump whole memory block to a file
#if 0
	FILE* f;
	DWORD id;
	HANDLE process_handle;
	DWORD num_bytes = -1;
	BOOL B_r;
// hmmm, 4096 as a page size is not defined anywhere in a windows includes
	static char buffer[4096];
	u32 address;

	id = GetCurrentProcessId ();
	process_handle = OpenProcess (PROCESS_ALL_ACCESS, 0, id);
	f = fopen ("dump.bin", "wb");
	for (address = 0; address < 0x04000000; address += 4096)
	{
		memset (buffer, 0, 4096);
		B_r = ReadProcessMemory (process_handle, (void*)address, buffer, 4096, &num_bytes);
		fwrite (buffer, 1, 4096, f);
	}
	fclose (f);
	CloseHandle (process_handle);
#endif
//
#if 0
	game_controls game_ctrls;
//	Screen_Restore ();
	game_ctrls = DMFCBase->GetLastGameControls ();
#endif
// hud message render test
#if 0
	DLLRenderHUDText (GR_RGB(255, 192, 128), 0x80, 0, 0x123, 0, "%s test message", "some1");
	DLLRenderHUDText (GR_RGB(255, 192, 128), 0xff, 0, 0x234, 0, "%s test message", "some2");
#endif
// watch 'player' structure values
#if 1
	float f;
//	f = players[0].weapon_recharge_scalar;
	f = players[0].last_fire_weapon_time;
//	if (f >= 0.5)
	if (f)
	{
//		Screen_Restore ();
	}
// calculated in the dmfc.dll
	f = DMFCBase->GetRealGametime ();
	f = DMFCBase->GetGametime ();
#endif

	Exceptions_Frame ();
// do not enable it for a just client - players, and the server admin will not tolerate it for long (a way for a player to get into a global ban list)
	if (local_role_is_server)
		if (coopmod_time_distortion != 1.0)
			CoopMod_DistortFrame ();
	CoopMod_OnInterval_Players ();
	if (coop_accel_kick)
		CoopMod_OnInterval_Kick ();
//	CoopMod_OnInterval_Objects ();
	CoopMod_GB_Release_Frame ();
	Rooms_OnInterval ();
//	Hooks_Frame ();

	if (coop_sleep)
#ifdef _WIN32
	{
		MMRESULT mmresult;

		mmresult = timeBeginPeriod (MM_TIMER_PERIOD);
//		if (mmresult != TIMERR_NOERROR)
		Sleep (coop_sleep);
		mmresult = timeEndPeriod (MM_TIMER_PERIOD);
//		GetSystemTimeAsUint64 (&time_cur);
	}
#else
	{
//		struct timeval tv = { num_seconds, num_microseconds };
		struct timeval tv = {0, coop_sleep * 1000};
		select (0, 0, 0, 0, &tv);
	}
#endif
}

void CoopMod_CheckPlayerPPS (int pnum)
{
	bool r;
	netplayer* netplayers = 0;
	int pps;
	player_info_t* player_info;

	if (! coop_pps_min)
		return;
	if (! pnum)
		return;

	r = CoopMod_TeammateStatus_Get (&player_info, pnum, "CheckPlayerPPS ()");
	if (! r)
		return;

	netplayers = DMFCBase->GetNetPlayers ();

// *I hope 'netplayer' is indexed the same way as 'player'
	pps = netplayers[pnum].pps;
	player_info->pps_original = pps;
// **not configurable
// ok, this works; so now maybe all pps relative injections/patches became unnecessary
	netplayers[pnum].pps = coop_pps;

	if (pps >= coop_pps_min)
// pps are acceptable
		return;

//	CoopMod_SendMessage (pnum, "- Please, use higher settings in D3Launcher->Setup->Network -");
//	CoopMod_SendMessage (pnum, "Your PPS (%u) setting is too low.", pps);
//	CoopMod_SendMessage (pnum, "== \"Descent 3.exe\" -> Setup -> Network -> LAN, T1 ==");
	CoopMod_SendMessage_Colored (pnum, GR_RGB(255, 64, 64), "Your PPS (%u) setting is too low.", pps);
	CoopMod_SendMessage_Colored (pnum, GR_RGB(255, 64, 64), "== \"Descent 3.exe\" -> Setup -> Network -> LAN, T1 ==");

	if (coop_pps_min_do_kick)
	{
//		CoopMod_Kick (pnum, 1, "player \"%s\" (#%d) has %d PPS (%d is min allowed)", player_info ? player_info->callsign : "<unknown>", pnum, pps, coop_pps_min);
		CoopMod_Kick (pnum, 1, "player \"%s\" (#%d) has %d PPS (%d is min allowed)", player_info->callsign, pnum, pps, coop_pps_min);
	}
}

void CoopMod_ClearClientStatus (int i)
{
	int j;
	player_info_t* player_info = &player_infos[i];

//	player_info->time.connection = 0;
//	player_info->time.disconnection = 0;
	player_info->time.play = 0;
//	player_info->time.kick = 0;

// teamkills
	for (j = 0; j < COOP_TEAM_KILLS_MAX; j++)
// *game time begins from 0
		player_info->teamkill.times[j] = -1000000.0;

// teamdamage_long
	player_info->teamdamage_long.count = 0;
//	DLLAddHUDMessage ("resetting 'player_info->teamdamage_long.count' for new client");
// teamdamage
	player_info->damage.to_robots_total = 0.0;
	player_info->damage.to_team_total = 0.0;
	player_info->damage.to_robots = 0.0;
	player_info->damage.to_team = 0.0;
}

// clear/prepare some info, and zero pointer to it
void CoopMod_Slot_Disconnect (int pnum)
{
	bool r;
	float time = DMFCBase->GetGametime ();
	player_info_t* player_info;

// *do not warn about it, requests about already disconnected players happens from time to time
	r = CoopMod_TeammateStatus_Get (&player_info, pnum, "Slot_Disconnect ()", 0);
	if (! r)
		return;

	player_info->connected = 0;
	player_info->time.disconnection = time;
	player_info->time.play += time - player_info->time.connection;
// kicking - off
	player_info->time.kick = 0.0;
	player_info_ptrs[pnum] = 0;
}

void CoopMod_OnServerPlayerEntersGame (int pnum)
{
	u32 i;
	player_record* pr;
	int net_address;
	player_info_t* player_info = 0;
	float slot_oldest_time;
	int slot_index = -1;
	char string[200] = "_1_";

	Assert(DMFCBase);

//	Screen_Restore ();
//	__asm int 3

// *no 'player_record' before this call
// *requests 'OnClientPlayerEntersGame ()'
	DMFCBase->OnServerPlayerEntersGame (pnum);

	pr = DMFCBase->GetPlayerRecordByPnum (pnum);

	if (! pr)
	{
		DLLAddHUDMessage ("Error :  no player record in 'CoopMod_OnServerPlayerEntersGame ()'");
		return;
	}

	Assert(pr->callsign);

// *not mandatory to move down; the client ip already was shown at 'ConnectionActivity ()'
	if (coop_nick_check && ! Nicks_Check (string, pr->callsign))
	{
//		CoopMod_SendMessage (-1, "player with invalid nick has been attempted to join");
		CoopMod_SendMessage (-1, "%s", string);
		DLLMultiDisconnectPlayer (pnum);
		return;
	}

//	if (pnum != DMFCBase->GetPlayerNum ())
//	if (team_damage)
// *show to entered player
//		CoopMod_SendMessage (pnum, "- Team Damage is on, be fair on teammates, have a nice game -");
	Assert(net_game);
// lets test socket buffers
#if 0
	u8 sock_buf[0xc28];
#define SOCK_BUF_IX 1
#define SOCK_BUF_STRUCT_SIZE 0xc28
#define SOCK_BUF_STRUCT (0x00fe9ed8 + SOCK_BUF_STRUCT_SIZE * SOCK_BUF_IX)
	memcpy (sock_buf, (void*)SOCK_BUF_STRUCT, SOCK_BUF_STRUCT_SIZE);
#endif
	static const char* difficulty_names[5] = 
	{
		"trainee",
		"rookie",
		"hotshot",
		"ace",
		"insane"
	};
// looks like 'COOPMOD_VERSION' now can be omited
//	CoopMod_SendMessage (pnum, "- Co-op Mod v %s, Team Damage is %s, Difficulty %u -", COOPMOD_VERSION, team_damage ? "on" : "off", net_game->difficulty);
//	CoopMod_SendMessage (pnum, "== Team Damage is (%u) %s, Difficulty - (%u) %s ==", team_damage, team_damage ? "on" : "off", net_game->difficulty, difficulty_names[net_game->difficulty]);
//	CoopMod_SendMessage (pnum, "== Team Damage is %s (%u), Difficulty is %s (%u) ==", team_damage ? "on" : "off", team_damage, difficulty_names[net_game->difficulty], net_game->difficulty);
	CoopMod_SendMessage (pnum, "== Team Damage is %s (%u), Difficulty is \"%s\" (%u) ==", team_damage ? "on" : "off", team_damage, difficulty_names[net_game->difficulty], net_game->difficulty);
#if 0
	for (i = 0; i < SOCK_BUF_STRUCT_SIZE; i++)
	{
		if (sock_buf[i] == ((u8*)SOCK_BUF_STRUCT)[i] + 1)
//		if (sock_buf[i] == ((u8*)SOCK_BUF_STRUCT)[i] + 2)
			DLLAddHUDMessage ("socket's possible stack position is at %04x", i);
	}
#endif

// 1) try to find the client in the old data
// 2) try to find a never used slot for the new (not seen before) client
// 3) try to find an oldest used slot for the new (not seen before) client

	net_address = *(u32*)pr->net_addr.address;
	byte* addr = (byte*)&net_address;
	char address_str[64];
	const char* country_name = 0;
	int country_ix = 0;
	ulong ip;

	sprintf (address_str, "%d.%d.%d.%d:%d", addr[0], addr[1], addr[2], addr[3], pr->net_addr.port);
	if (show_client_IP)
		DLLAddHUDMessage ("connected player (#%d) IP is :  %s", pnum, address_str);
// log it
	if (geoip_instance)
	{
		ip = addr[0] << 24;
		ip |= addr[1] << 16;
		ip |= addr[2] << 8;
		ip |= addr[3];
		country_ix = GeoIP_id_by_ipnum (geoip_instance, ip);
		Assert(country_ix >= 0);
		Assert(country_ix < 253);
		country_name = GeoIP_country_name[country_ix];
		LogFile_Chat_Printf ("* enters #%u, %s, \"%s\", [%s]", pnum, address_str, pr->callsign, country_name);
	}
	else
	{
		LogFile_Chat_Printf ("* enters #%u, %s, \"%s\"", pnum, address_str, pr->callsign);
	}

	float time = DMFCBase->GetGametime ();

// (step 0) check out, maybe already has been connected, and still pnum is in use (happens !)
	if (player_info_ptrs[pnum])
	{
// normally happens, when level changes
//		DLLAddHUDMessage ("'OnServerPlayerEntersGame ()' :  player #%d was not disconnected", pnum);
// register disconnect event
		CoopMod_Slot_Disconnect (pnum);
	}

// (step 1) try to find client in old teammates data
	for (i = 0; i < C_T_STATUSES_NUM; i++)
	{
		if (player_infos[i].used && ! player_infos[i].connected)
			if (player_infos[i].net_address.ip_d == net_address)
// same IP
			{
				player_info = &player_infos[i];
//				break;
// -- initialization for earlier known client --
				player_info->teamdamage_long.last_calmdown_time = time;
				goto got_player_info;
			}
	}

// not found in old data
	slot_oldest_time = time;

// (step 2 and 3 - unified) *if think, that the game time is always positive, then 'used' isn't seriously needed here
	for (i = 0; i < C_T_STATUSES_NUM; i++)
	{
		if (! player_infos[i].used)
// not used slot
		{
			slot_index = i;
			break;
		}
		if (player_infos[i].connected)
// slot is used, and client is connected
			continue;
		if (slot_oldest_time > player_infos[i].time.disconnection)
// oldest slot
		{
			slot_oldest_time = player_infos[i].time.disconnection;
			slot_index = i;
// continue
		}
	}

//	DLLAddHUDMessage ("Slot %d will be used in 'CoopMod_OnServerPlayerEntersGame ()' for new client", slot_index);

	if (slot_index < 0)
	{
		DLLAddHUDMessage ("Error :  all slots are used in 'CoopMod_OnServerPlayerEntersGame ()'");
		player_info_ptrs[pnum] = 0;
		return;
	}
	else
// add new (unknown till this moment) client to teammates data
	{
//		player_info_t* player_info = &player_infos[slot_index];
		player_info = &player_infos[slot_index];

// -- initialization for not known earlier client --
		player_info->used = 1;
//		CoopMod_ClearStatusSlot (slot_index);
		CoopMod_ClearClientStatus (slot_index);

		player_info->net_address.ip_d = net_address;
		player_info->net_address.port = pr->net_addr.port;
	}
//// else - found in old data

got_player_info:
	player_info_ptrs[pnum] = player_info;
// -- initialization for any client --
	player_info->connected = 1;
	player_info->pnum = pnum;
	strcpy (player_info->callsign, pr->callsign);
// *does not work maybe ?
	player_info->object_index = players[pnum].objnum;
	A(objects);
	player_info->object_ptr = &objects[player_info->object_index];
	A(players);
	player_info->plr = &players[player_info->pnum];
	player_info->time.connection = time;
	player_info->time.disconnection = 0.0;
// no kicking
	player_info->time.kick = 0.0;

	if (0)
	{
		msafe_struct mstruct;
		FILE* f;
		static char temp_buf[1000];

		f = fopen ("dbg5.txt", "ab");

//		mstruct.type = 3;
//		mstruct.is_real = 4;
		mstruct.index = 0;
//		mstruct.slot = 0;
//		mstruct.count = 0;

		fwrite (&mstruct, sizeof(mstruct), 1, f);
		fwrite (temp_buf, 0x400-sizeof(mstruct), 1, f);
//		fprintf (f, "\r\n");

		mstruct.objhandle = player_info->object_ptr->handle;
// *returns 1.0, :/
//		MSafe_GetValue (MSAFE_INVEN_SIZE, &mstruct);
// *returns 0, :/
//		MSafe_GetValue (MSAFE_INVEN_COUNT, &mstruct);
// *returns properly number of 'count' along with id
		MSafe_GetValue (MSAFE_INVEN_GET_TYPE_ID, &mstruct);
// not helps against guidebot, cause still is possible to release by F4
		MSafe_CallFunction (MSAFE_INVEN_REMOVE, &mstruct);

//		mstruct.count;
		DLLAddHUDMessage ("count is %d", mstruct.count);

		fwrite (&mstruct, sizeof(mstruct), 1, f);
		fwrite (temp_buf, 0x400-sizeof(mstruct), 1, f);
//		fprintf (f, "\r\n");

		fclose (f);
	}

	CoopMod_CheckPlayerPPS (pnum);

	return;
}

// *requected from time to time for long time ago disconnected players
// *seems this function is not requested, if a level is changing
// *seems is not always requested for every 'OnServerPlayerEntersGame ()', perhaps some patroling subroutine necessary
void CoopMod_OnServerPlayerDisconnect (int pnum)
{
	player_record* pr;

	Assert(DMFCBase);
	pr = DMFCBase->GetPlayerRecordByPnum (pnum);
//	LogFile_Chat_Printf ("* disconnect #%u, %s, \"%s\"", pnum, address_str, pr->callsign);
	if (pr)
		LogFile_Chat_Printf ("* disconnect #%u, \"%s\"", pnum, pr->callsign);
	else
		LogFile_Chat_Printf ("* disconnect #%u", pnum);
// hmmm, I think this is bad order
#if 0
	DMFCBase->OnServerPlayerDisconnect (pnum);

	CoopMod_Slot_Disconnect (pnum);
#else
	CoopMod_Slot_Disconnect (pnum);

	DMFCBase->OnServerPlayerDisconnect (pnum);
#endif
}

void CoopMod_OnServerPlayerKilled (object* killer_obj, int victim_pnum)
{
	// First call the default handler to do the real processesing
	DMFCBase->OnServerPlayerKilled (killer_obj, victim_pnum);

	player_record* kpr;
	int kpnum;
	int teamkill = 0;

// *kills by a robot was intended to use, although can't
	if (killer_obj)
	{
		if ((killer_obj->type == OBJ_PLAYER) || (killer_obj->type == OBJ_GHOST))
		{
			kpnum = killer_obj->id;

// debug :  it lets you to make teamkill, by killing yourself
#if 0
			if (killer_obj->type == OBJ_PLAYER)
#else
// ***one of these things isn't pnum
			if (killer_obj->type == OBJ_PLAYER && victim_pnum != kpnum)
#endif
				teamkill = 1;
		}
		else if (killer_obj->type == OBJ_ROBOT || killer_obj->type == OBJ_BUILDING)
		{
			//countermeasure kill
			kpnum = DMFCBase->GetCounterMeasureOwner (killer_obj);
		}
		else
		{
			kpnum = -1;
		}
	}
	else
	{
		kpnum = -1;
	}

	kpr = DMFCBase->GetPlayerRecordByPnum (kpnum);

	if(kpr)
	{
		if (teamkill)
		{
			CoopMod_TeamKill (kpnum, victim_pnum);
		}
	}
}

void CoopMod_TeamKill (int killer_pnum, int victim_pnum)
{
	bool r;
//	player_record *kpr;
	player_info_t* player_info;
	int i;

//	kpr = DMFCBase->GetPlayerRecordByPnum (killer_pnum);

//	if (! kpr)
//		return;

	r = CoopMod_TeammateStatus_Get (&player_info, killer_pnum, "TeamKill (_killer_pnum_)");
	if (! r)
		return;

//	if (! player_info)
//	{
//		DLLAddHUDMessage ("Error in 'CoopMod_TeamKill ()' :  player %d is not in 'player_infos[]' table", killer_pnum);
//		return;
//	}

	float time = DMFCBase->GetGametime ();
	int oldest_teamkill_inside_period = COOP_TEAM_KILLS_MAX - 1;

// scroll teamkill times, and find which is inside the time period defined with 'coop_team_kills_period'
	for (i = COOP_TEAM_KILLS_MAX - 2; i >= 0; i--)
	{
		float teamkill_time;
		teamkill_time = player_info->teamkill.times[i];
		player_info->teamkill.times[i + 1] = teamkill_time;
		if (time - teamkill_time > coop_team_kills_period)
			oldest_teamkill_inside_period = i;
	}
	player_info->teamkill.times[0] = time;
// *'oldest_teamkill_inside_period' is equal to index now
	oldest_teamkill_inside_period++;

// show the number of teamkills
	CoopMod_SendMessage (-1, "%s has %d teamkills in %.1f minutes period (kick on %d)", player_info->callsign, oldest_teamkill_inside_period, coop_team_kills_period / 60.0, coop_team_kills_max);

// too much teamkills inside time period - make a kick
	if (auto_kick__team_kills)
		if (oldest_teamkill_inside_period >= coop_team_kills_max)
			CoopMod_Kick (killer_pnum, 1, "too many teamkills");
}

int CoopMod_GetEfficiency (int pnum)
{
	player_info_t* player_info = player_info_ptrs[pnum];

	if (! player_info)
		return -1;

	if (! player_info->damage.to_team_total)
		return -1;

	return player_info->damage.to_robots_total / player_info->damage.to_team_total;
}


// +++ rooms +++

void CoopMod_PlayerImpulse (int pnum, int from_direction)
{
	msafe_struct mstruct;

	if (DMFCBase->IsPlayerDedicatedServer (pnum))
	{
		CoopMod_SendMessage (-1, "can't impulse dedicated server main player");
		return;
	}
//	bool r;
//	r = CoopMod_TeammateStatus_Get (&player_info, pnum, "PlayerImpulse ()");
//	if (! r)
//		return;

	mstruct.slot = pnum;
	MSafe_GetValue (MSAFE_OBJECT_PLAYER_HANDLE, &mstruct);
//	DLLAddHUDMessage ("objhandle %d %d", mstruct.objhandle, player_info->object_index);

//	room* rooms = DMFCBase->GetRooms ();
////	DLLComputeRoomCenter (&mstruct.pos, &rooms[room_num]);
//	MSafe_GetValue (MSAFE_OBJECT_ORIENT, &mstruct);
////	Room_Value (room_num, VF_GET, RMSV_V_PATH_PNT, &mstruct.pos, 0);
//	Room_Value (room_num, VF_GET, RMSV_V_PORTAL_PATH_PNT, &mstruct.pos, 0);
////	Room_Value (room_num, VF_GET, RMSV_V_FACE_CENTER_PNT, &mstruct.pos, 0);
//	DLLAddHUDMessage ("new position is :  %f, %f, %f", mstruct.pos.x, mstruct.pos.y, mstruct.pos.z);
	mstruct.velocity.x = 0.0;
//	mstruct.velocity.y = 10.0;
	mstruct.velocity.y = 1000.0;
	mstruct.velocity.z = 0.0;
	MSafe_CallFunction (MSAFE_OBJECT_VELOCITY, &mstruct);
}

// not helps much
void CoopMod_PlayerInProhibitedRoom (int pnum, int room_num, int from_direction, float damage_amount, float damage_period)
{
msafe_struct mstruct;

	if (DMFCBase->IsPlayerAlive (pnum))
	{
		CoopMod_SendMessage (pnum, "this room (%d) isn't allowed", room_num);

		mstruct.slot = pnum;
		MSafe_GetValue (MSAFE_OBJECT_PLAYER_HANDLE, &mstruct);

		mstruct.killer_handle = mstruct.objhandle;
// with omega sound
		mstruct.damage_type = GD_ELECTRIC;
		mstruct.amount = damage_amount;
// crash at respawn time, if is applied on dead player
		MSafe_CallFunction (MSAFE_OBJECT_DAMAGE_OBJECT, &mstruct);

		CoopMod_PlayerImpulse (pnum, from_direction);
	}
}

// --- rooms ---

// +++ events +++

// blocking not helps
bool CoopMod_EvtCollide (int me_handle, int it_handle)
{
	object* obj_me;
	object* obj_it;
	char string[200];
	bool r;

	DLLObjGet (me_handle, &obj_me);
	DLLObjGet (it_handle, &obj_it);

	if (! obj_me || ! obj_it)
// block it, ok ?
//		return 0;
		return 1;

// *hmmm, Guidebot is little bit harder to implement here
	if (obj_it->type != OBJ_PLAYER && obj_it->type != OBJ_OBSERVER)
		return 1;

// object handle, object index, room num, type, id
// *room num will be necessary !, because all indexes changes
//	sprintf (string, "%d, %d, %d, %d, %d", me_handle, obj_me - objects, obj_me->roomnum, obj_me->type, obj_me->id);
	sprintf (string, "%d, %d; %d, %d, %d (%.0f, %.0f, %.0f)", me_handle, obj_me - objects, obj_me->roomnum, obj_me->type, obj_me->id, obj_me->pos.x, obj_me->pos.y, obj_me->pos.z);
	Rooms_LogString (string);

//	DLLAddHUDMessage ("collide with: handle %d, index %d, type %d, id %d", me_handle, obj_me - objects, obj_me->type, obj_me->id);
	if (show_object_info_on_collide)
// public
//		CoopMod_SendMessage (-1, "collide with: handle %d, index %d, type %d, id %d", me_handle, obj_me - objects, obj_me->type, obj_me->id);
		CoopMod_SendMessage (-1, "collide: handle %d, index %d, type %d, id %d, pos (%.0f, %.0f, %.0f)", me_handle, obj_me - objects, obj_me->type, obj_me->id, obj_me->pos.x, obj_me->pos.y, obj_me->pos.z);

	if (crashy_powerups_block)
		r = Powerups_Check (me_handle);
	else
		r = 1;

	if (! r)
		CoopMod_SendMessage (obj_it->id, "this item pickup is not allowed");

	return r;
}

// object destroyed
//// *not checked that 'it_handle' has a sense at all
// *return value is not used
//bool CoopMod_EvtDestroy (int me_handle, int it_handle)
bool CoopMod_EvtDestroy (int me_handle, byte is_dying)
//bool CoopMod_EvtDestroy_Post (int me_handle, byte is_dying)
{
	u32 num;
	object* obj;

	if (! is_dying)
// level end; do not process it
		return 1;
// *wouldn't need the obj type here, but have to count to 1 and not to 0 :/
	DLLObjGet (me_handle, &obj);
	if (! obj)
		return 1;
	if (obj->type != OBJ_ROBOT)
		return 1;
//	CoopMod_SendMessage (-1, "an object has been destroyed");
	if (! coop_mod_endlevel_on_0_robots)
// level end; do not process it
		return 1;
//	CoopMod_SendMessage (-1, "endlevel is allowed");
	num = GameUtils_CountObjects_Type (OBJ_ROBOT);
//	CoopMod_SendMessage (-1, "remained robots num is %u", num);
// silly hack, but nothing can do - the object will be deleted only after call to us
//	if (! num)
	if (num < 2)
	{
		CoopMod_SendMessage (-1, "coop mod :  all robots were destroyed, changing level");
//		DLLMultiEndLevel ();
		DMFCBase->EndLevel ();
	}
	return 1;
}

// --- events ---


// -- Alex :  added for debug --

// We are getting a control message from someone, process the
// control messages that we want to process.  We must make sure
// we also call the default handler (very important).
// Control messages with value 0xE0 or higher are reserved for DMFC
/*
void CoopMod_OnControlMessage (ubyte msg, int from_pnum)
{
	DLLAddHUDMessage("received ControlMessage %d from %d", (int)msg, from_pnum);

	DMFCBase->OnControlMessage(msg,from_pnum);
}

void CoopMod_OnSpecialPacket (void)
{
	DLLAddHUDMessage("received SpecialPacket");

	DMFCBase->OnSpecialPacket();
}
*/

#ifdef WIN32
HINSTANCE gh_module = 0;
//HANDLE gh_process = 0;

// entry point
// although this function is optiona., I need it to get module address, so I could be able to check the dll, that it is up to date
BOOL APIENTRY DllMain (HINSTANCE h_module, DWORD reason, PVOID reserved)
{
	bool r;
	BOOL B_r;
//	DWORD id;
//	const char* filename;
	static char filename[4096];
//	byte* image;
//	u32 len;

	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
//		Screen_Restore ();
//		__asm int 3
		Assert(h_module);
		gh_module = h_module;
#if 0
		id = GetCurrentProcessId ();
		Assert(id);
		gh_process = OpenProcess (PROCESS_ALL_ACCESS, 0, id);
		Assert(gh_process);
#if 1
		B_r = CloseHandle (gh_process);
		Assert(B_r);
		gh_process = 0;
#endif
#endif
		B_r = GetModuleFileName (gh_module, filename, sizeof(filename));
		Assert(B_r);
// *give any length
		r = Image_CheckTimestamp ((byte*)gh_module, 0x1000, filename);
		break;

	case DLL_PROCESS_DETACH:
#if 0
		Assert(gh_process);
		B_r = CloseHandle (gh_process);
		Assert(B_r);
		gh_process = 0;
#endif
		break;
	}

	return 1;
}
#endif


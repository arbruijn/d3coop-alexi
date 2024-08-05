#pragma once

#include "types.h"
#include "idmfc.h"
//#include <string>
// MSVC 6.0
// *like isn't necessary here, but later, warnings are thrown on <vector>, <map>
#if _MSC_VER <= 1200
#pragma warning(disable:4786)
#endif
#include <sstream>
//// *'vector' does conflict
using namespace std;


#define COOPMOD_VERSION "= b2"
//#define C_T_STATUSES_NUM 256
#define C_T_STATUSES_NUM 40

#define COOP_TEAM_KILLS_MAX 10

// *one info for one IP
typedef struct player_info_s
{
	bool used;
	bool connected;
	int pnum;
//	ubyte net_address[4];
	struct
	{
// *in net order; I know, it isn't proper
		union
		{
			u8 ip_b[4];
			u32 ip_d;
		};
// *in host order
		u16 port;
	} net_address;
// *needed for banning or punish disconnected client
// *may be done by other way, probably (look in the list for IP, and then act by it)
	char callsign[MAX_CALLSIGN_SIZE]; // Player's callsign
// *must not be index, if possible*
	int object_index;
// warning: 'obj->id' (seems) isn't the same as 'pnum'; but will not be easy to find out what is what
	object* object_ptr;
	player* plr;
	u32 pps_original;
// it would look a bit strange, but lets pack it into a structure
	struct
	{
		float connection;
		float disconnection;
		float play;
// kick
		float kick;
	} time;
// *expected for kicking teamkillers
	struct
	{
//		bool warned;
		int times[COOP_TEAM_KILLS_MAX];
	} teamkill;
// uninterruptable teamdamage
	struct
	{
//		bool warned;
		float last_robot_damage_time;
// for kick
		float first_teamdamage_time;
// for filtering
		float last_teamdamage_time;
		int count;
		int count_filtered;
		float last_calmdown_time;
		float last_calmdown_filtered_time;
	} teamdamage_long;
// damage statistics tracking
	struct
	{
//		bool warned;
		float to_team_total;
		float to_robots_total;
// -- not in use currently --
// *was expected for filtered by time ("current") values
// damage done to the teammates
		float to_team;
// damage done to the robots
		float to_robots;
	} damage;
// -- not in use currently --
	struct
	{
		float punished_teamdamage_time;
		float punishment_begin_time;
		float punishment_end_time;
	} teamdamage;
} player_info_t;


//extern int local_role;
extern bool local_role_is_server;
extern const char* local_role_name;

extern player_info_t* player_info_ptrs[DLLMAX_PLAYERS];

// +++ configuration +++
extern bool auto_kick__team_kills;
extern bool auto_kick__team_damage_long;

extern float kick_time_delay;

extern int coop_team_kills_max;
extern float coop_team_kills_period;

extern int coop_teamdamage_long_min;
extern int coop_teamdamage_long_max;
extern float coop_teamdamage_long_period;
extern float coop_teamdamage_long_coeficient;
extern float coop_teamdamage_robotdamage_post_time;

extern int coop_sleep;
extern bool show_client_IP;
extern bool show_connection_activity;
extern bool show_connection_activity_packet;
extern bool show_room_num;
extern bool show_object_info_on_collide;
extern bool crashy_powerups_block;
extern bool crashy_powerups_move;
extern bool coop_nick_check;

extern int coop_anti_guidebot_mode;
//extern bool coop_guidebot_block;
extern int coop_pps_min;
extern bool coop_pps_min_do_kick;
extern u32 coop_pps;
extern float coopmod_time_distortion;

extern int coopmod_dump_obj_handle;
extern bool team_damage;
extern bool coop_mod_endlevel_on_0_robots;
// --- configuration ---

//extern int gb_released_handle;
//extern object* gb_released_obj;

extern bool coop_accel_kick;

// + game data +
extern netgame_info* net_game;
extern object_info* objects_info;
extern object* objects;
extern player* players;
extern ship* ships;
// - game data -


// *need pointers; 'player_info_out' can be 0
bool CoopMod_TeammateStatus_Get (player_info_t** player_info_out, int pnum, char* caller, bool warn_if_not_indexed = 1);
bool CoopMod_TeammateStatus_Get (player_info_t** player_info_out, string player, char* caller);

// + debug +
//void CoopMod_OnInputString (char* input_string);
void CoopMod_OnGB_Release (object* obj);
// - debug -

void CoopMod_Init (void);
void CoopMod_Close (void);
// + indication +
void CoopMod_SendMessage (int player, char* format, ...);
void CoopMod_DamageStatusTotally (int whos, int whom);
void CoopMod_DamageStatusTotallyIndexed (int whos_index, int whom);
void CoopMod_PlayerInfoIndexed (int whos_index, int whom);
void CoopMod_ShowPlayerPPS (int player_num, int whom);
void CoopMod_ShowMissionInfo ();
void CoopMod_ShowGeoip (const char* str);
void DisplayHUDRoomnum (struct tHUDItem* hitem);
void CoopMod_OnGB_Release_Notify (int pnum);
void CoopMod_ConnectionActivity (int pnum);
void CoopMod_ConnectionPacket (ulong ip, ushort port);
// test
void CoopMod_PowerupsDeleteAll ();
// - indication -
void CoopMod_OnServerObjectShieldsChanged (object* obj, float amount);
void CoopMod_OnInterval_Kick (void);
void CoopMod_OnInterval (void);
void CoopMod_OnServerPlayerEntersGame (int player_num);
void CoopMod_OnServerPlayerDisconnect (int player_num);
void CoopMod_OnServerPlayerKilled (object* killer_obj, int victim_pnum);
void CoopMod_TeamKill (int killer_pnum, int victim_pnum);
int CoopMod_GetEfficiency (int pnum);
void CoopMod_PlayerInProhibitedRoom (int pnum, int room_num, int from_direction, float damage_amount, float damage_period);
// + events +
bool CoopMod_EvtCollide (int me_handle, int it_handle);
bool CoopMod_EvtDestroy (int me_handle, byte is_dying);
//bool CoopMod_EvtDestroy_Post (int me_handle, byte is_dying);
// - events -

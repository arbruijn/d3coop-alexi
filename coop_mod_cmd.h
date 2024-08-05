#pragma once

//#include "idmfc.h"
#include "coop_mod.h"


bool CoopModCmd_Init ();
//void CoopMod_SetMacro (char* macro);
void CoopMod_SetMacro (const char* macro);
void CoopMod_RunMacro ();
bool CoopMod_Kill (int pnum, char* cause);
void CoopMod_Kick (int pnum, bool delayed, char* format, ...);
void CoopMod_UnKick (int pnum);
void CoopMod_UnKickAll (void);
void CoopMod_SetSleep (bool change, int value);
//void CoopMod_MoveObject (int objhandle, int room_num, bool echo);
void CoopMod_MoveObject (int objhandle, int room_num);
//void CoopMod_MovePlayer (int pnum, int room_num, bool echo);
void CoopMod_MovePlayer (int pnum, int room_num);
void CoopMod_MovePlayersAll (int room_num);
void CoopMod_FindObj (int obj_type, int obj_id);
void CoopMod_FindRobot (int obj_id);
void CoopMod_DumpObj ();
void CoopMod_DumpObjAI ();
void CoopMod_ObjectShield (int ix, int value);
void CoopMod_PlayerShield (player_info_t* player_info, int value);
void CoopMod_PlayerShieldNormalize (player_info_t* player_info);
void CoopMod_PlayerEnergy (player_info_t* player_info, int value);
void CoopMod_PlayerAmmo (player_info_t* player_info, int value);
void CoopMod_PlayerMissiles (player_info_t* player_info, int value);
//void CoopMod_Spawn (const char* arguments);
void CoopMod_SpawnInFront (const char* arguments);
void CoopMod_AddWeapon ();
void CoopMod_ObjectModifyFlag (int ix, u32 flag, bool on);
void CoopMod_ObjectFlag_Clear (int ix, int flag_ix);
void CoopMod_ObjectFlag_Set (int ix, int flag_ix);
void CoopMod_ObjectModifyAIType (int ix, int type);
void CoopMod_ObjectModifyAIFlag (int ix, u32 flag, bool on);
void CoopMod_ObjectAIFlag_Clear (int ix, int flag_ix);
void CoopMod_ObjectAIFlag_Set (int ix, int flag_ix);
//void CoopMod_ObjectRebel (int ix, bool on);
void CoopMod_ObjectRebel (int ix, int on);
//void CoopMod_ObjectHostile (int ix, bool on);
void CoopMod_ObjectHostile (int ix, int on);
void CoopMod_ObjectFire (int ix, int on);
void CoopMod_TeamDamage (bool change, bool value);
void CoopMod_ObjectShowBeam (int ix);
void CoopMod_SetPPS (int pps);


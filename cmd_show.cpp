// commands to show a value

#include "cmd_show.h"
#include "coop_mod.h"
#include "coop.h"
#include "hooks.h"
#include "error.h"
#include "log_file.h"
//#define __OSIRIS_IMPORT_H_
#include "../osiris/osiris_import.h"
#include "game_utils.h"
#include "dallas.h"


void CmdShow_Version (u32 pnum)
{
//	CoopMod_SendMessage (pnum, "coop mod version is %s", COOPMOD_VERSION);
	CoopMod_SendMessage (pnum, "server versions :  main.exe %s, dmfc.dll %s, coop mod %s", version_main_name, version_dmfc_name, COOPMOD_VERSION);
}

void CmdShow_Sleep (u32 pnum)
{
	CoopMod_SendMessage (pnum, "sleep time is %d", coop_sleep);
}

void CmdShow_Difficulty (u32 pnum)
{
	netgame_info* ng_info;
	ng_info = DMFCBase->GetNetgameInfo ();
	Assert(ng_info);
	CoopMod_SendMessage (pnum, "difficulty is %d", ng_info->difficulty);
}

//void CmdShow_Settings (u32 pnum)
void CmdShow_Logging (u32 pnum)
{
	stringstream ss;

//	CoopMod_SendMessage (pnum, "connections logging is %d, chat logging is %d", log_connections, log_chat);
	ss << "connections logging is ";
#if LOG_CONNECTIONS
	ss << log_connections;
#else
	ss << "<not installed>";
#endif
	ss << ", chat logging is ";
#if LOG_CHAT
	ss << log_chat;
#else
	ss << "<not installed>";
#endif
	CoopMod_SendMessage (pnum, "%s", ss.str());
}

void CmdShow_ObjectInfo (u32 pnum, u32 obj_ix)
{
	object* obj;

// *let to enter an object handle also
	obj_ix &= HANDLE_OBJNUM_MASK;
	if (obj_ix >= MAX_OBJECTS)
	{
		CoopMod_SendMessage (pnum, "object number #%u is out of range %u - %u", obj_ix, 0, MAX_OBJECTS);
		return;
	}
	obj = &objects[obj_ix];
	CoopMod_SendMessage (pnum, "object %u :  type %u, id %u, handle %u, shield is %f", obj_ix, obj->type, obj->id, obj->handle, obj->shields);
}

void CmdShow_ObjectShield (u32 pnum, u32 obj_ix)
{
//	bool r;
	object* obj;

	obj_ix &= HANDLE_OBJNUM_MASK;
	if (obj_ix >= MAX_OBJECTS)
	{
		CoopMod_SendMessage (pnum, "object number #%u is out of range %u - %u", obj_ix, 0, MAX_OBJECTS);
		return;
	}
	obj = &objects[obj_ix];
	CoopMod_SendMessage (pnum, "object %u shield is %f", obj_ix, obj->shields);
}

// *also have to do an informer about others shield, but can't now, because now there are only unargumented commands; argumented aren't implemented
// *could just take the object 'shields', instead of using 'mstruct'
//void CmdShow_Shield (u32 pnum, u32 pnum_he)
void CmdShow_PlayerShield (u32 pnum, u32 pnum_he)
{
	bool r;
	msafe_struct mstruct;
	float value;
	const char* connected;

	if (pnum_he >= DLLMAX_PLAYERS)
	{
		CoopMod_SendMessage (pnum, "player number #%u is out of range %u - %u", pnum_he, 0, DLLMAX_PLAYERS);
		return;
	}
	mstruct.slot = pnum_he;
	MSafe_GetValue (MSAFE_OBJECT_PLAYER_HANDLE, &mstruct);
	MSafe_GetValue (MSAFE_OBJECT_SHIELDS, &mstruct);
	value = mstruct.shields;
// *also can use 'NPF_CONNECTED' in 'netplayer'
	r = DMFCBase->CheckPlayerNum (pnum_he);
	if (r)
		connected = "";
	else
		connected = " (not connected)";
#if 0
	if (pnum == pnum_he)
		CoopMod_SendMessage (pnum, "your shield is %f", value);
	else
#endif
		CoopMod_SendMessage (pnum, "player #%u shield is %f%s", pnum_he, value, connected);
}

void CmdShow_OwnShield (u32 pnum)
{
	CmdShow_PlayerShield (pnum, pnum);
}

void CmdShow_ShowRobotsNum (u32 pnum)
{
	u32 num;

	num = GameUtils_CountObjects_Type (OBJ_ROBOT);
	CoopMod_SendMessage (pnum, "robots number is %u", num);
}

// a bit lame :  the beam can only be between two object centers
void CmdShow_ObjectShowBeam (u32 pnum, u32 obj_ix)
{
	u32 texture_id;
	u32 objhandle1;
	u32 objhandle2;
	msafe_struct mstruct;
	object* obj;

//#define BEAM_TEXTURE_NAME "Acid 2"
#define BEAM_TEXTURE_NAME "FunkyEffect1"
//#define BEAM_TEXTURE_NAME "Static"
	texture_id = Scrpt_FindTextureName (BEAM_TEXTURE_NAME);
	if (texture_id == -1)
	{
// even internal error
		CoopMod_SendMessage (pnum, "failed to get texture \"%s\" handle", BEAM_TEXTURE_NAME);
		return;
	}
	mstruct.slot = pnum;
	MSafe_GetValue (MSAFE_OBJECT_PLAYER_HANDLE, &mstruct);
	objhandle1 = mstruct.objhandle;
// is a failed handle -1 ?
	if (objhandle1 == -1)
	{
// even internal error
		CoopMod_SendMessage (pnum, "failed to get player #%u handle", pnum);
		return;
	}
	if (obj_ix >= MAX_OBJECTS)
	{
		CoopMod_SendMessage (pnum, "object #%u is out of range %u - %u", obj_ix, 0, MAX_OBJECTS);
		return;
	}
	obj = &objects[obj_ix];
	objhandle2 = obj->handle;
//	CoopMod_SendMessage (pnum, "texture is %d, handle 1 is %u, handle 2 is %u", texture_id, objhandle1, objhandle2);
	if (obj->type == OBJ_NONE)
	{
		CoopMod_SendMessage (pnum, "object #%u is unused", obj_ix);
		return;
	}
//	Dallas_LightningCreate (objhandle1, objhandle2, 10.0f, 7.0f, 1, texture_id, 0.5f, 1, 255, 255, 255, 0);
//	Dallas_LightningCreate (objhandle1, objhandle2, 10.0f, 1.0f, 1, texture_id, 2.0f, 1, 255, 255, 255, 0);
	Dallas_LightningCreate (objhandle1, objhandle2, 10.0f, 1.0f, 1, texture_id, 2.0f, 1, 255, 255, 255, 1);
	CoopMod_SendMessage (pnum, "showing beam to object #%u", obj_ix);
}

void CmdShow_ShowRobotNearest (u32 pnum)
{
	u32 obj_ix;

	obj_ix = GameUtils_FindNearestObject (pnum, OBJ_ROBOT);
	if (obj_ix == -1)
	{
		CoopMod_SendMessage (pnum, "can't find robot object");
		return;
	}
// hmmm, already being reported in 'CmdShow_ObjectShowBeam()'
//	CoopMod_SendMessage (pnum, "nearest robot object #%u, distance %f", obj_ix);
	CoopMod_SendMessage (pnum, "nearest robot object #%u", obj_ix);
	CmdShow_ObjectShowBeam (pnum, obj_ix);
}

void CmdShow_PPS (u32 pnum)
{
	bool r;
	player_info_t* player_info;
	netplayer* netplayers = 0;
	u32 server_pps;

	netplayers = DMFCBase->GetNetPlayers ();
	r = CoopMod_TeammateStatus_Get (&player_info, pnum, "CmdShow_PPS");
	if (! r)
		return;
// no
	server_pps = netplayers[0].pps;
	if (pnum)
// normal
//		CoopMod_SendMessage (pnum, "server PPS is %u, your PPS is %u", server_pps, netplayers[pnum].pps);
// patched (the patch is very simple, and the player's pps is not available anymore atm)
//		CoopMod_SendMessage (pnum, "PPS to you is %u", netplayers[pnum].pps);
//		CoopMod_SendMessage (pnum, "server PPS to you is %u, your PPS is %u", netplayers[pnum].pps, player_info->pps_original);
		CoopMod_SendMessage (pnum, "server PPS limit to you is %u, your setup PPS is %u", netplayers[pnum].pps, player_info->pps_original);
	else
		CoopMod_SendMessage (pnum, "server PPS is %u", server_pps);
}


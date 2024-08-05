// appears - most crashes are at the powerups pick up
// this objects numbers can be different; so these lists are for :
// insane difficulty; banned items are (my .mps file content) :
#if 0
OBJBAN	Betty4pack
OBJBAN	buddywingnut
OBJBAN	buddyassist
OBJBAN	buddyspeed
OBJBAN	Buddyextinguisher
OBJBAN	buddycontrol
OBJBAN	buddyantivirus
OBJBAN	ProxMinePowerup
//OBJBAN	Seeker3pack
SHIPBAN	Black Pyro
#endif

// no need anymore; respawn bug is fixed in hooks
#define POWERUPS__RESPAWN_BUG__MOVE 0

#include "powerups.h"
#include "missions.h"
#include "idmfc.h"
//#include "coop_mod.h"
//#define __OSIRIS_IMPORT_H_
#include "../osiris/osiris_import.h"


static int powerups_moved_num = 0;


// :/, I know that hard-coded stuff isn't looks good

// not helps !, crash happens before going at here.
// but am not sure now - why did I left this code then ... for the case if I still will need to do some in 'EvtCollide' with relation here ? - only if so.
#if 1
bool Powerups_MissionRetribution (int handle)
{
	bool result = 1;

	switch (level_num)
	{
	case 1:
// concusion4pack in room 91 (just test; no any bug)
		if (handle == 2059)
			result = 0;
		break;
	case 3:
// quad lasers in room 112 (respawn bug, seems)
		if (handle == 2693)
			result = 0;
		break;
	case 11:
// microwave cannon in room 27 (crash)
		if (handle == 2119)
			result = 0;
// smart missile in room 27 (crash)
		if (handle == 2121)
			result = 0;
		break;
	case 12:
// quad lasers in room 192 (respawn bug)
		if (handle == 2189)
			result = 0;
// perhaps - not these
#if 0
// abc
		if (handle == 2190)
			result = 0;
// md
		if (handle == 4239)
			result = 0;
#endif
		break;
	case 17:
// mass driver in room 109 (crash)
		if (handle == 2180)
			result = 0;
		break;
	}

	return result;
}

bool Powerups_Check (int handle)
{
	bool result = 1;

	switch (mission_index)
	{
	case MISSION_RETRIBUTION:
		result = Powerups_MissionRetribution (handle);
		break;
	}

	return result;
}
#endif

// at map start the handles are stay the same, if ban some powerups
void Powerups_MoveObject (int handle, vector* vector_delta)
{
msafe_struct mstruct;

	mstruct.objhandle = handle;
// does it exist
	MSafe_GetValue (MSAFE_OBJECT_TYPE, &mstruct);
	if (mstruct.type == -1)
	{
		DLLAddHUDMessage ("'Powerups_MoveObject ()' :  object %d not exist", handle);
		return;
	}

	MSafe_GetValue (MSAFE_OBJECT_WORLD_POSITION, &mstruct);
	mstruct.pos += *vector_delta;
// 'MSAFE_OBJECT_POS' seems not works for 'MSafe_CallFunction ()'
	MSafe_CallFunction (MSAFE_OBJECT_WORLD_POSITION, &mstruct);
	powerups_moved_num++;
}

void Powerups_Move_Mission_Retribution ()
{
	vector vector_delta = {0.0, 0.0, 0.0};
//	vector_delta.z = 1.0;

	switch (level_num)
	{
	case 1:
#if 0
// concusion4pack in room 91 (just test; no any bug)
		vector_delta.x = 13.9f;
		vector_delta.y = -13.8f;
		vector_delta.z = 13.9f;
		Powerups_MoveObject (2059, &vector_delta);
//		CoopMod_SendMessage (-1, "level 1");
#endif
		break;

	case 7:
// respawn bug :  smart missile; perhaps one of those which are outside
		break;

	case 10:
#if POWERUPS__RESPAWN_BUG__MOVE
// energy in room 3144 (near base where Juggernaut goes), index 287 (respawn bug, checked 1 times)
// 30 units down is not enough
		vector_delta.z = -100.0;
		Powerups_MoveObject (2335, &vector_delta);
// single homing * 2; seems in destroyed mine, near spawn (12461); doubtly the one in the Juggernaut's view
// something single on the map; maybe napalm ammo in room 40 - nope, it is in secret place also; fusion cannon in room 63 (2286) perhaps; it was something on the way back, at spawn
// shield in room 45 (2413) (mostly probably) or 26 (2393) (doubtly)
#endif
		break;

	case 11:
// (near E center)
// microwave cannon in room 27 (crash)
		vector_delta.x = 30.0;
		vector_delta.z = -30.0;
//		Powerups_MoveObject (2119, &vector_delta);
// shield in room 27 pos x 2260 (crash, seems ok)
//		Powerups_MoveObject (4165, &vector_delta);
// maybe this is right handle ?
//		Powerups_MoveObject (2117, &vector_delta);
// super laser in room 27 (respawn crashes)
		Powerups_MoveObject (2118, &vector_delta);
// (away from E center)
// smart missile in room 27 (crash)
		vector_delta.x = -30.0;
		vector_delta.z = 30.0;
//		Powerups_MoveObject (2121, &vector_delta);
// shield in room 27 pos x 2220 (crash, seems ok)
//		Powerups_MoveObject (2120, &vector_delta);
// napalm missile in room 27 (crash, seems ok)
//		Powerups_MoveObject (2122, &vector_delta);
		break;

	case 12:
#if POWERUPS__RESPAWN_BUG__MOVE
// quad lasers in room 192 (respawn bug, checked 1 times)
		vector_delta.y = -20.0;
		Powerups_MoveObject (2189, &vector_delta);
// perhaps - not these
#if 0
// abc
		Powerups_MoveObject (2190, &vector_delta);
// md
		Powerups_MoveObject (4239, &vector_delta);
#endif
#endif
		break;

	case 13:
#if POWERUPS__RESPAWN_BUG__MOVE
// respawn bug by :
// homing4pack
// homing4pack room 93
		vector_delta.y = 20.0;
		Powerups_MoveObject (4226, &vector_delta);
#endif
		break;

	case 14:
#if POWERUPS__RESPAWN_BUG__MOVE
// not necessary to die here, to produce respawn bug
// 1)
// 91, frag
// 2)
// smart
// 3)
// shield, from some one of two small matcens, near imaginated earth; mostly porbable left one, in room 125
// 4)
// napalm, room 90, left one, checked twice
// 5)
// vauss ammo, and doubtly that the one in room 83; again, doubtly in room 83 (and after that double bug, caused by some smart missile in room 90)
// 6)
// something in room 163 (md maybe) or near; possibly md ammo in room 9     -     wrong, it was vauss ammo, and probably not here
// left one
		vector_delta.y = -30.0;
		Powerups_MoveObject (2290, &vector_delta);
// this is just a test, to prove that another one bugs
// right one
//		Powerups_MoveObject (4337, &vector_delta);
// 5)
// energy, room 87, checked once
// frag in room 91 (respawn bug, perhaps)
		vector_delta.y = -20.0;
		Powerups_MoveObject (2173, &vector_delta);
// shield in room 91 (respawn bug)
//		Powerups_MoveObject (2174, &vector_delta);
#endif
		break;

	case 17:
// mass driver in room 109 (crash)
// for my last try it doesn't crashed, but I think it will, so lets leave
		vector_delta.y = -20.0;
		Powerups_MoveObject (2180, &vector_delta);
		break;
	}
}

void Powerups_Move_Mission_Mercenary ()
{
	vector vector_delta = {0.0, 0.0, 0.0};

	switch (level_num)
	{
	case 5:
// room 3 crashed multiple times, homing4pack respawn could be
// near are :  frag4pack 10379, shield 4453
// but here it crashes when just looking onto the big door, so it could be not powerups relative
// no, crash happens without taking of any of powerups in room 3
		vector_delta.y = +30.0;
//		Powerups_MoveObject (4451, &vector_delta);
// mortar, perhaps room 83
		break;
	case 6:
// nothing such, everything bugs; a first item, which will be taken after matcen will turn on
// or more probably after taking access card, which is above matcen; any powerup, including spews (e ball)
#if 0
// shield ball in room 10
		vector_delta.y = -40.0;
		Powerups_MoveObject (2167, &vector_delta);
// perhaps shield ball in room 6
		vector_delta.y = -40.0;
		Powerups_MoveObject (2171, &vector_delta);
// smart missile in room 16
// so :  everything bugs in this level
#endif
		break;
	}
}

void Powerups_Move_Mission_OperationMoon ()
{
	vector vector_delta = {0.0, 0.0, 0.0};

// single homing in room 34; once respawn bug, once crash
// move into wall near napalm missile
	vector_delta.x = -80.0;
	Powerups_MoveObject (2187, &vector_delta);
}

void Powerups_Move_Mission_Evolution21 ()
{
	vector vector_delta = {0.0, 0.0, 0.0};

	switch (level_num)
	{
	case 1:
		break;
// "Evolution V.2.1" level 2
	case 2:
#if POWERUPS__RESPAWN_BUG__MOVE
// respawn bug :  vauss ammo in room 18, tested twice
// 60 unavailable; 80 seems yet not appears on the surface
		vector_delta.y = 70.0;
		Powerups_MoveObject (2178, &vector_delta);
#endif
		break;
	}
}

void Powerups_Move_Mission_Windmine_1_2 ()
{
	vector vector_delta = {0.0, 0.0, 0.0};

	switch (level_num)
	{
// vauss ammo, respawn bug
	case 1:
		break;
// MD, or perhaps something near it - crash
// crash; taking MD in room 43, or perhaps something respawning near it from room 40 side
	case 2:
//		vector_delta.y = 70.0;
//		Powerups_MoveObject (2178, &vector_delta);
		break;
	}
}

// *rails forcefield switch rooms are 376, 377
void Powerups_Move_Mission_Windmine_6 ()
{
	vector vector_delta = {0.0, 0.0, 0.0};

#if POWERUPS__RESPAWN_BUG__MOVE
// room 228, homing4pack; possibly I had a mistake
// again, second try, not the room 228, which is side "1" (with an earthshaker missile); though the stuff of room 233 also hasn't
	vector_delta.y = -40.0;
	Powerups_MoveObject (4186, &vector_delta);
// room 233, homing4pack
	vector_delta.y = -40.0;
	Powerups_MoveObject (2146, &vector_delta);
#endif
}

void Powerups_Move_Mission_N4Installation ()
{
	vector vector_delta = {0.0, 0.0, 0.0};

// black shark, perhaps in room 268, but also might be another one, near
//	vector_delta.y = -50.0;
//	Powerups_MoveObject (2178, &vector_delta);
}

void Powerups_Move_Mission_OnyxSecretBase ()
{
	vector vector_delta = {0.0, 0.0, 0.0};

	if (level_num != 2)
		return;
// room 29, homing4pack respawn crashes; twice
	vector_delta.y = -60.0;
	Powerups_MoveObject (2126, &vector_delta);
}

// *'netgame_info->level_checksum' would be more reliable
void Powerups_Move ()
{
	switch (mission_index)
	{
	case MISSION_RETRIBUTION:
		Powerups_Move_Mission_Retribution ();
//		break;
		goto print_number;
	case MISSION_MERCENARY:
		Powerups_Move_Mission_Mercenary ();
		goto print_number;
	case MISSION_OPERATION_MOON:
		Powerups_Move_Mission_OperationMoon ();
//		break;
		goto print_number;
	case MISSION_EVOLUTION21:
		Powerups_Move_Mission_Evolution21 ();
//		break;
		goto print_number;
	case MISSION_WINDMINE_1_2:
		Powerups_Move_Mission_Windmine_1_2 ();
		goto print_number;
	case MISSION_WINDMINE_3:
		goto print_number;
	case MISSION_WINDMINE_4:
		goto print_number;
	case MISSION_WINDMINE_5:
		goto print_number;
	case MISSION_WINDMINE_6:
		Powerups_Move_Mission_Windmine_6 ();
		goto print_number;
	case MISSION_N4_INSTALLATION:
		Powerups_Move_Mission_N4Installation ();
		goto print_number;
	case MISSION_ONYX_SECRET_BASE:
		Powerups_Move_Mission_OnyxSecretBase ();
		goto print_number;
	}

	DLLAddHUDMessage ("Powerups :  mission is not in database");
	return;

print_number:
	DLLAddHUDMessage ("Powerups :  %d have been moved", powerups_moved_num);
	powerups_moved_num;
}


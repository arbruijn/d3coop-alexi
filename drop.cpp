//

#include "drop.h"
#include "error.h"
//#include "idmfc.h"
#include "coop_mod.h"
#include "coop.h"
// once it should be without this define; and for other times, this define should be on (.h file has the content, which should be in .cpp)
//#define __OSIRIS_IMPORT_H_
#include "../osiris/osiris_import.h"
#include "objects.h"
#include "packet.h"
#include "screen.h"
#include "cmd.h"

// don't request 'MSafe_CallFunction ()' in a client -  it will then just change (distort) displaying information, but not really do a changes


#define PI 3.14159265358979323846264338327950288
#define PI2 (PI * 2.0)

bool drop = 1;
// is buggy atm
bool drop_weapon = 1;


// improper value here - will break a game :  by taking back the throwing powerup, a player could rise his power
void Drop_GetDifficultyBallContainAmount (float& value)
{
	switch (net_game->difficulty)
	{
	case 0:
		value = 27.0;
		break;
	case 1:
		value = 18.0;
		break;
	case 2:
		value = 12.0;
		break;
	case 3:
		value = 9.0;
		break;
	case 4:
		value = 6.0;
		break;
	default:
		Error_NoExit ("difficulty %u", net_game->difficulty);
	}
}

void Drop_CreateObject (msafe_struct* player_mstruct, int powerup_id, u32 spread_num = 1, u32 spread_ix = 0)
{
	int objnum;
	object* obj;
	float spread_ang;
	float spread_pos;
	float rad;
	matrix m0;
	matrix m1;
	vector v;

	Assert(spread_num);
//	Screen_Restore ();

// get player physics
//	MSafe_GetValue (MSAFE_OBJECT_ROOMNUM, player_mstruct);
// it gets roomnum + pos + orient; nothing more, checked
	MSafe_GetValue (MSAFE_OBJECT_WORLD_POSITION, player_mstruct);
//	MSafe_GetValue (MSAFE_OBJECT_POS, player_mstruct);
//	MSafe_GetValue (MSAFE_OBJECT_ORIENT, player_mstruct);
	MSafe_GetValue (MSAFE_OBJECT_VELOCITY, player_mstruct);

#if 0
	if (spread_num > 1)
// spread
	{
		spread_ang = (float)(PI / 24.0);
		spread_pos = PI2 / spread_num * spread_ix;

		v.z = cos(spread_ang);
		rad = sin(spread_ang);
		v.x = sin(spread_pos) * rad;
		v.y = cos(spread_pos) * rad;
		DLLvm_VectorToMatrix (&m0, &v, 0, 0);
//		player_mstruct->orient *= m0;
//		player_mstruct->orient = player_mstruct->orient * m0;
		DLLvm_MatrixMul (&m1, &player_mstruct->orient, &m0);
	}
	else
		m1 = player_mstruct->orient;
#else
// spread
	if (spread_num > 1)
	{
		spread_ang = (float)(PI / 16.0);
	}
	else
	{
// a bit more exact than for a few items drop
		spread_ang = (float)(PI / 20.0);
	}
// randomize angle
	spread_ang *= rand();
	spread_ang /= RAND_MAX;
// randomize position-on-circle
	spread_pos = PI2 * rand() / RAND_MAX;

	v.z = cos(spread_ang);
	rad = sin(spread_ang);
	v.x = sin(spread_pos) * rad;
	v.y = cos(spread_pos) * rad;
	DLLvm_VectorToMatrix (&m0, &v, 0, 0);
	DLLvm_MatrixMul (&m1, &player_mstruct->orient, &m0);
#endif

// 3.0 - taking it back, if still moving forward slightly
//	mstruct.pos += mstruct.orient.fvec * 5.0;

// oops, suddenly found what I wished :  if set the handle to its dropping ship, then it will not be takeable for those 3 seconds
	objnum = DLLObjCreate (OBJ_POWERUP, powerup_id, player_mstruct->roomnum, &player_mstruct->pos, &player_mstruct->orient, player_mstruct->objhandle);
//	objnum = DLLObjCreate (OBJ_POWERUP, powerup_id, player_mstruct->roomnum, &player_mstruct->pos, &player_mstruct->orient, -1);
	if (objnum == -1)
		return;
// should not happen
	if (objnum < 0)
		return;
	if (objnum >= MAX_OBJECTS)
		return;

	obj = &objects[objnum];
// throw it forward
//	obj->mtype.phys_info.velocity = player_mstruct->orient.fvec * 40.0;
//	obj->mtype.phys_info.velocity = m1.fvec * 40.0;
// randomize also the speed a bit
	double vel;
	vel = (double)rand() / RAND_MAX;
#define SPEED_RANDOMIZATION_PART 4
//#define SPEED_RANDOMIZATION_PART 5
	vel /= SPEED_RANDOMIZATION_PART;
	vel += (1.0 - 0.5 / SPEED_RANDOMIZATION_PART);
// apply average speed
	vel *= 40.0;
	obj->mtype.phys_info.velocity = m1.fvec * vel;
// add ship velocity
	obj->mtype.phys_info.velocity += player_mstruct->velocity;

	DLLMultiSendObject (obj, 0, 1);
// without this, the object still is impossible to take by any player
	DLLInitObjectScripts (obj, 1);
}

void Drop_LocalDropDenyNotice ()
{
// *not a "server command", it really is not
//	DLLAddHUDMessage ("can't drop locally; use a server command with '/' instead");
	DLLAddHUDMessage ("can't drop locally; use a served command beginning with '/' instead");
}


// +++++ drop a component routines +++++

// server drop shield command
void Drop_Shield (int pnum, u32 nitems)
{
	msafe_struct mstruct;
	float value;
	float amount;
	u32 i;

	if (! drop)
	{
		CoopMod_SendMessage (pnum, "drop function is disabled");
		return;
	}
	Drop_GetDifficultyBallContainAmount (amount);

	for (i = 0; i < nitems; i++)
	{
		mstruct.slot = pnum;
		MSafe_GetValue (MSAFE_OBJECT_PLAYER_HANDLE, &mstruct);
		MSafe_GetValue (MSAFE_OBJECT_SHIELDS, &mstruct);
		value = mstruct.shields;
		if (value < amount * 2.0)
		{
			CoopMod_SendMessage (pnum, "low shield");
			return;
		}
		value -= amount;
		mstruct.shields = value;

		MSafe_CallFunction (MSAFE_OBJECT_SHIELDS, &mstruct);

		Drop_CreateObject (&mstruct, POWERUP_SHIELD, nitems, i);
	}
}

void Drop_Shield (u32 pnum)
{
	if (! local_role_is_server)
//// it is correct; don't bother with some custom packets or anything; let a user just type "/ds", and that's it;
//// so maybe even this function is unnecessary (if chat would take commands from player #0)
	{
		Drop_LocalDropDenyNotice ();
		return;
	}
	Drop_Shield (pnum, 1);
}

void Drop_Shield5 (u32 pnum)
{
	if (! local_role_is_server)
	{
		Drop_LocalDropDenyNotice ();
		return;
	}
	Drop_Shield (pnum, 5);
}

#if 0
// local drop shield command dispatcher (becomes unnecessary)
void Drop_Shield ()
{
	Drop_Shield (DMFCBase->GetPlayerNum ());
}

void Drop_Shield5 ()
{
	Drop_Shield (DMFCBase->GetPlayerNum ());
}
#endif

// server drop energy command
void Drop_Energy (int pnum, u32 nitems)
{
	msafe_struct mstruct;
	float value;
	float amount;
	u32 i;

	if (! drop)
	{
		CoopMod_SendMessage (pnum, "drop function is disabled");
		return;
	}
	Drop_GetDifficultyBallContainAmount (amount);

	for (i = 0; i < nitems; i++)
	{
		mstruct.slot = pnum;
		MSafe_GetValue (MSAFE_OBJECT_PLAYER_HANDLE, &mstruct);
		MSafe_GetValue (MSAFE_OBJECT_ENERGY, &mstruct);
		value = mstruct.energy;
		if (value < amount * 2.0)
		{
			CoopMod_SendMessage (pnum, "low energy");
			return;
		}
		value -= amount;
		mstruct.energy = value;

		MSafe_CallFunction (MSAFE_OBJECT_ENERGY, &mstruct);

		Drop_CreateObject (&mstruct, POWERUP_ENERGY, nitems, i);
	}
}

void Drop_Energy (u32 pnum)
{
	if (! local_role_is_server)
	{
		Drop_LocalDropDenyNotice ();
		return;
	}
	Drop_Energy (pnum, 1);
}

void Drop_Energy5 (u32 pnum)
{
	if (! local_role_is_server)
	{
		Drop_LocalDropDenyNotice ();
		return;
	}
	Drop_Energy (pnum, 5);
}

#if 0
void Drop_Energy ()
{
	Drop_Energy (DMFCBase->GetPlayerNum ());
}

void Drop_Energy5 ()
{
	Drop_Energy (DMFCBase->GetPlayerNum ());
}
#endif

// **does not work
// *no need anymore
#if 0
void Drop_PlayerWeapon_Remove (int pnum, int weapon_ix)
{
	player* player;
//	ship* ship;
//	otype_wb_info* wb;

	player = &players[pnum];

	// "Take" the weapon from the player
// 0x008f6380 for plr 0
	player->weapon_flags &= ~(HAS_FLAG(weapon_ix));

#if 0
	ship = &(DMFCBase->GetShips())[player->ship_index];
	wb = &ship->static_wb[weapon_ix];

	if (! wb)
	{
		Error_NoExit ("Drop_PlayerWeapon_Remove () :  no wb");
		return;
	}

	wb->ammo_usage;
#endif

// weapon drop in the engine
// 4932c0 is called for various events
// 4ed5f0, 4ee150, 4edc20, 4edc80 are requested only for weapon add (MSAFE_WEAPON_ADD)
// doesn't matter ...
}

// 'weapon_row' - primary, secondary, countemeasure, enhancement, inventory
// (countemeasure, enhancement, inventory - are possibly autoselecting itself)
// seems the game engine has no exports for that; have to do some hack
//void Drop_PlayerWeapon_AutoSelect (object* obj_plr, int weapon_row)
void Drop_PlayerWeapon_AutoSelect (object* obj_plr, int weapon_ix)
{
	msafe_struct mstruct;

//	DLLAddHUDMessage ("Drop_PlayerWeapon_AutoSelect (%d)", weapon_ix);

	memset (&mstruct, 0, sizeof(mstruct));
	mstruct.objhandle = obj_plr->handle;
// 00
	mstruct.roomnum = 0;
// 44
	mstruct.type = OBJ_WEAPON;
// 48
//	mstruct.id = POWERUP_CANNON_PLASMA;
	mstruct.id = 4;
	mstruct.index = 4;
	mstruct.state = 0;
//	mstruct.count = 0;
	mstruct.count = 1;
// 250
	mstruct.flags = 0;
	Screen_Restore ();
//	MSafe_CallFunction (MSAFE_OBJECT_FIRE_WEAPON, &mstruct);
// uses of 'msafe_struct' :  objhandle
//	MSafe_CallFunction (MSAFE_WEAPON_CHECK, &mstruct);
// uses of 'msafe_struct' :  objhandle, (+108, 4) index, (+114, 1) state, (+24c, 4) count
	MSafe_CallFunction (MSAFE_WEAPON_ADD, &mstruct);
//	DMFCBase->OnWeaponFired (object* weapon_obj, object* shooter);
// hehe, just fires, without counting
//	DLLFireWeaponFromObject (object *obj,int weapon_num,int gun_num, bool f_force_forward, bool f_force_target);
//	DLLFireWeaponFromObject (obj_plr, weapon_ix, 0, 0, 0);
//	Inven_Remove (Inventory *inven,int type,int id);
// uses of 'msafe_struct' :  objhandle, (+44, 4) , (+48, 4) , (+250, 4) flags
//	MSafe_CallFunction (MSAFE_INVEN_ADD_TYPE_ID, &mstruct);

// and this will work only for player #0; and even not works for him
#if 0
	game_controls game_ctrls;

	game_ctrls.fire_primary_down_count = 1;
	game_ctrls.fire_primary_down_state = 1;
	game_ctrls.fire_primary_down_time = 1.0;
	DMFCBase->CallOnDoControls (&game_ctrls);
#endif
}
#endif

//// hmmm, this is a wepon removing subr., not ammo diminishing
// pnum != objnum !  only if for player 0; except if on a level without other objects
//void Drop_PlayerWeapon_Diminish (u32 objnum, u32 weapon_ix, u32 amount)
//void Drop_PlayerWeapon_Remove (u32 objnum, u32 weapon_ix)
//void Drop_PlayerWeapon (u32 objnum, u32 weapon_ix, u32 amount = 0)
//void Drop_PlayerWeapon_Drop (u32 objnum, u32 weapon_ix, u32 amount = 0)
void Drop_PlayerWeapon_Drop (u32 pnum, u32 weapon_ix, u32 amount = 0)
{
	msafe_struct mstruct;
	bool remove;
	object* obj;
	u32 objnum;

// test
#if 0
	if (local_role_is_server)
		Beep (10000, 10);
	else
		Beep (50, 100);
#endif
#if 0
	if (amount != 1)
		Beep (10000, 10);
	else
		Beep (50, 100);
#endif

//// current problems :  #0 still has weapon (plasma), after drop, until switch;
// if just disables a weapon flag, then clients see that they have plasma after drop, and can still locally switch to it
// "Take" the weapon from the player
// 0x008f6380 for plr 0
//	player->weapon_flags &= ~(HAS_FLAG(weapon_ix));

	objnum = players[pnum].objnum;
//	CoopMod_SendMessage (-1, "player %u objnum is %08x", pnum, objnum);
	if (objnum >= MAX_OBJECTS)
	{
		CoopMod_SendMessage (-1, "weapon drop server error :  player %u objnum is %08x", pnum, objnum);
		return;
	}
	obj = &objects[objnum];

// when old grey scratches, requests are :
// 4a8028, 4a805b
// 490b40 (MSafe_CallFunction), 492fcd

#if 0
004A8028 8B 84 24 C0 03 00 00 mov         eax,dword ptr [esp+3C0h]
004A802F 8B 55 00             mov         edx,dword ptr [ebp]
004A8032 89 84 24 1C 01 00 00 mov         dword ptr [esp+11Ch],eax // 108 index
004A8039 83 F8 0A             cmp         eax,0Ah
004A803C 8D 44 24 14          lea         eax,[esp+14h]
004A8040 89 74 24 40          mov         dword ptr [esp+40h],esi // 2c objhandle
004A8044 0F 9C C1             setl        cl
004A8047 50                   push        eax
004A8048 68 FB 00 00 00       push        0FBh
004A804D 88 8C 24 30 01 00 00 mov         byte ptr [esp+130h],cl // 114 state
004A8054 89 94 24 68 02 00 00 mov         dword ptr [esp+268h],edx // 24c count
#endif

	remove = weapon_ix < 10;
//	mstruct.objhandle = obj_plr->handle;
	mstruct.objhandle = obj->handle;
	mstruct.index = weapon_ix;
// more probably is set/add, instead of remove/add
	mstruct.state = remove;
//// didn't work on a client side ?  tested ?
	mstruct.count = remove ? 0 : -(signed)amount;
//	mstruct.count = 0;
// offsets test
#if 0
	mstruct.roomnum = 0;
	mstruct.killer_handle = 0;
#endif
//	Screen_Restore ();
// *is not actually called nor by level start/end, nor by player dead/respawn, nor by weapon select, nor by weapon autoselect due to pickup nor ammo end
// uses of 'msafe_struct' :  objhandle; does not change any value, even if fill 'index', 'state', 'count' for it
//	MSafe_CallFunction (MSAFE_WEAPON_CHECK, &mstruct);
// uses of 'msafe_struct' :  objhandle, (+108, 4) index, (+114, 1) state, (+24c, 4) count
	MSafe_CallFunction (MSAFE_WEAPON_ADD, &mstruct);
// test vauss
#if 0
	mstruct.index = 1;
	mstruct.state = 0;
//	mstruct.count = 0;
	mstruct.count = 1;
	MSafe_CallFunction (MSAFE_WEAPON_ADD, &mstruct);
#endif
}

//// 'amount_ammo' - new ammo amount (not difference)
// set the player's ammo, and set him a packet that he would also set it locally
//void Drop_DiminishAmmo (u32 pnum, u32 weapon_ix, ushort amount_ammo)
void Drop_SetAmmo (u32 pnum, u32 weapon_ix, ushort amount_ammo)
{
	player* plr;

	if (pnum >= DLLMAX_PLAYERS)
		return;
	plr = &players[pnum];
// set first locally
// 008f6384 for player 0
	plr->weapon_ammo[weapon_ix] = amount_ammo;
//	Drop_PlayerWeapon_Diminish (plr->objnum, weapon_ix, 0);
// crap begins :  have to correct ammo amount on a client's side
	switch (weapon_ix)
	{
// 01
	case VAUSS_INDEX:
// 06
	case MASSDRIVER_INDEX:
	case NAPALM_INDEX:
// missiles
// 10
	case CONCUSSION_INDEX:
	case HOMING_INDEX:
	case IMPACTMORTAR_INDEX:
	case SMART_INDEX:
	case MEGA_INDEX:
// 15
	case FRAG_INDEX:
	case GUIDED_INDEX:
	case NAPALMROCKET_INDEX:
	case CYCLONE_INDEX:
	case BLACKSHARK_INDEX:
		Packet_Send_DropWeapon_SetAmmo_ToClient (pnum, weapon_ix, amount_ammo);
		break;
// 00
	case LASER_INDEX:
	case MICROWAVE_INDEX:
	case PLASMA_INDEX:
	case FUSION_INDEX:
// 05
	case SUPER_LASER_INDEX:
	case EMD_INDEX:
	case OMEGA_INDEX:
	default:
		break;
	}
}

void Drop_ThrowObject (u32 pnum, u32 powerup_id)
{
	msafe_struct mstruct;

	mstruct.slot = pnum;
	MSafe_GetValue (MSAFE_OBJECT_PLAYER_HANDLE, &mstruct);

// get player physics
	MSafe_GetValue (MSAFE_OBJECT_WORLD_POSITION, &mstruct);
	MSafe_GetValue (MSAFE_OBJECT_VELOCITY, &mstruct);

	Drop_CreateObject (&mstruct, powerup_id);
}

// current problems :  non #0 are dropping even if low ammo
// *'Drop_WeaponSecondary()' and 'Drop_AmmoPrimary()' are a duplicates of this function :/
void Drop_WeaponPrimary (u32 pnum)
{
	player* plr;
	u32 weapon_ix;
	player_weapon* plr_weapon;
	ushort amount_ammo;
	u32 powerup_id;
//	bool uses_ammo;
//	msafe_struct mstruct;

	Assert(pnum < MAX_PLAYERS);

	if (! drop_weapon)
	{
		CoopMod_SendMessage (pnum, "drop weapon function is disabled");
		return;
	}

// vauss ammo is added through these subroutines
// 493420 (MSafe_DoPowerup), 49349e
// 493670, 493996
// 493310, 49338a
// vauss cannon is added through these subroutines
// 493420 (MSafe_DoPowerup), 4934a3
// 493670, 493727
// 4ed5f0, 4ed616
// but actually a weapon/ammo is sent to a client through
// 497930 (takes 'msafe_struct', but isn't published through osiris table)
// takes 'facenum' - 1, 'objhandle' - item, 'ithandle' - player, 'type' - OBJ_PLAYER, 'playsound' - 1
// but nothing good further; ammo there is sent with a packet like "5F 07 00 B6 02 10 00" (code 5f, len 7, obj ammo, obj plr)

// test
#if 0
//	msafe_struct mstruct;
	Screen_Restore ();
// for the vauss, there are :  objhandle, ithandle
	MSafe_DoPowerup (&mstruct);
#endif

	plr = &players[pnum];
	plr_weapon = &plr->weapon[PW_PRIMARY];
	weapon_ix = plr_weapon->index;
// when a client is shooting, vauss and napalm ammo do not diminish; only md ammo does
//	amount_ammo = plr->weapon_ammo[PRIMARY_INDEX + weapon_ix];
	amount_ammo = plr->weapon_ammo[weapon_ix];
// test
//	CoopMod_SendMessage (-1, "ammo amount is %d", amount_ammo);
//	CoopMod_SendMessage (pnum, "ammo amount is %d", amount_ammo);
// **actually this stuff is game dependent (should use 'ship' structure)
//	uses_ammo = 0;
	switch (weapon_ix)
	{
	case LASER_INDEX:
		bool r;
		Inventory* inv;

		powerup_id = POWERUP_GUN_QUADLASERS;
		inv = &plr->inventory;
// if would like to watch, inventory for the player #1 is at :  '&item->type' is 019790b0, '&item->id' is 019790b8
// or 019780b0; and '&item->flags' at 019780c0, '&item->count' at 019780c8
// '&item->next' at 019780dc, '&item->prev' at 019780e0
// prev 00978120
// modification of inventory when a thief is stealing quad lasers is called by path :
// (in loop) 407810, 407e77; 406ae0, 406bb3;
// (from time to time) 405bb0, 405edd;
// (tear out by thief) 405800, 4058aa; 406ae0, 407778; 4ae8d0, 4aeb37; .....; 4a7d10, 4a81d3;
// (if can steal anything; no, if is stealing quad lasers; the packet is sent from here) 4bebc0, 4bebc9, 4bebf2 (sending packet), 4becd0 (OBJ_POWERUP), 4bed06, 4bed2b;
// 47f5f0 (refered only from 4bebf2; pack a packet and send), 47f68c; 488710, 48875e; 559120
//  the packet is :  71 (type), 0b 00 (len), 07 00 00 00 (OBJ_POWERUP), 01 00 00 00 (pnum).
// (inventory modify, but only locally) 456f10, 456f71; 456f80, 457038.
		r = inv->CheckItem (OBJ_POWERUP, powerup_id);
// eehh, forget ... this engine still wonders me by its shittiest abilities
//		if (0)
//		if (! r)
		{
			CoopMod_SendMessage (pnum, "can't drop this primary weapon");
			return;
		}
#if 0
		{
			byte packet[] = {0x71, 0x0b, 0, 7, 0, 0, 0, 1, 0, 0, 0};
// no, it packs/wraps the packet again on 479ca0 into the 0x22 packet
			DMFCBase->SendPacket (packet, sizeof(packet), SP_ALL);
		}
#endif
#if 0
//		MEMSET0(mstruct);
		mstruct.slot = pnum;
//		mstruct.objhandle = plr->objnum;
		MSafe_GetValue (MSAFE_OBJECT_PLAYER_HANDLE, &mstruct);
#endif
// senseless, it is used for a singlet objects
#if 0
// *this perhaps is the silly way, which is used through the 0x71 packet
// nope, returns 0
		mstruct.ithandle = OBJ_POWERUP;
		MSafe_GetValue (MSAFE_INVEN_CHECK_OBJECT, &mstruct);
		CoopMod_SendMessage (pnum, "primary weapon presense is %u", mstruct.state);
#endif
// still strange
#if 0
		mstruct.ithandle = 0;
		MSafe_CallFunction (MSAFE_INVEN_REMOVE_OBJECT, &mstruct);
#endif
//		break;
//		return;
		goto throw_object;
	case VAUSS_INDEX:
		if (amount_ammo < 5000)
			goto too_low_ammo;
		amount_ammo -= 5000;
//		powerup_id = POWERUP_AMMO_VAUSS;
		powerup_id = POWERUP_CANNON_VAUSS;
//		uses_ammo = 1;
#if 0
__asm
{
	mov eax,4000
	push eax
	mov eax,[weapon_ix]
	push eax
	mov eax,[pnum]
	push eax
// hmmm, does not compile
//	call 0x004ED5F0
// errr, it is a weapon pickup
//	mov eax,0x004ED5F0
// I mean this
	mov eax,0x00493310
	call eax
	add esp,12
}
// test
		return;
#endif
		break;
	case MICROWAVE_INDEX:
		powerup_id = POWERUP_GUN_MICROWAVE;
		break;
	case PLASMA_INDEX:
		powerup_id = POWERUP_CANNON_PLASMA;
		break;
	case FUSION_INDEX:
		powerup_id = POWERUP_CANNON_FUSION;
		break;
	case SUPER_LASER_INDEX:
		powerup_id = POWERUP_GUN_SUPERLASER;
		break;
	case MASSDRIVER_INDEX:
		if (amount_ammo < 10)
			goto too_low_ammo;
		amount_ammo -= 10;
		powerup_id = POWERUP_GUN_MASSDRIVER;
//		uses_ammo = 1;
		break;
	case NAPALM_INDEX:
// *500, not 50.0
		if (amount_ammo < 500)
			goto too_low_ammo;
		amount_ammo -= 500;
		powerup_id = POWERUP_GUN_NAPALM;
//		uses_ammo = 1;
		break;
	case EMD_INDEX:
		powerup_id = POWERUP_CANNON_EMD;
		break;
	case OMEGA_INDEX:
		powerup_id = POWERUP_CANNON_OMEGA;
		break;
	default:
		Error_NoExit ("player %d, primary weapon ix %d", weapon_ix);
		return;
	}
//	plr->weapon_ammo[PRIMARY_INDEX + weapon_ix] = amount_ammo;

//	Drop_PlayerWeapon_Remove (pnum, weapon_ix);
//	Drop_PlayerWeapon_AutoSelect (0);
//	Drop_PlayerWeapon_AutoSelect (&objects[plr->objnum], weapon_ix);

//	Drop_PlayerWeapon_Diminish (plr->objnum, weapon_ix, 0);
	Drop_PlayerWeapon_Drop (pnum, weapon_ix);
//// *after weapon drop, otherwise vauss drops just a single ammo
//	plr->weapon_ammo[weapon_ix] = amount_ammo;
	Drop_SetAmmo (pnum, weapon_ix, amount_ammo);

throw_object:
	Drop_ThrowObject (pnum, powerup_id);

	return;

// to protect against cheating-recharging, goto here
too_low_ammo:
	CoopMod_SendMessage (pnum, "not enough ammo to drop this primary weapon");
}

#if 0
void Drop_WeaponPrimary ()
{
	if (! local_role_is_server)
	{
		Drop_LocalDropDenyNotice ();
		return;
	}
	Drop_WeaponPrimary (DMFCBase->GetPlayerNum ());
}
#endif

void Drop_WeaponSecondary (u32 pnum)
{
	player* plr;
	int weapon_ix;
	player_weapon* plr_weapon;
	ushort amount_ammo;
	u32 amount;
	int powerup_id;
	bool can_4pack;

	Assert(pnum < MAX_PLAYERS);

	if (! drop_weapon)
	{
		CoopMod_SendMessage (pnum, "drop weapon function is disabled");
		return;
	}

	plr = &players[pnum];
	plr_weapon = &plr->weapon[PW_SECONDARY];
	weapon_ix = plr_weapon->index;
//	DLLAddHUDMessage ("weapon ix %u", weapon_ix);
//	amount_ammo = plr->weapon_ammo[SECONDARY_INDEX + weapon_ix];
	amount_ammo = plr->weapon_ammo[weapon_ix];
	if (0)
		goto no_ammo;
	if (! amount_ammo)
		goto no_ammo_this;
	can_4pack = amount_ammo >= 4;
	switch (weapon_ix)
	{
	case CONCUSSION_INDEX:
		if (can_4pack)
			powerup_id = POWERUP_MISSILE_CONCUSION4PACK;
		else
			powerup_id = POWERUP_MISSILE_CONCUSION;
		break;
	case HOMING_INDEX:
		if (can_4pack)
			powerup_id = POWERUP_MISSILE_HOMING4PACK;
		else
			powerup_id = POWERUP_MISSILE_HOMING;
		break;
	case IMPACTMORTAR_INDEX:
		powerup_id = POWERUP_MISSILE_MORTAR;
		can_4pack = 0;
		break;
	case SMART_INDEX:
		powerup_id = POWERUP_MISSILE_SMART;
		can_4pack = 0;
		break;
	case MEGA_INDEX:
		powerup_id = POWERUP_MISSILE_MEGA;
		can_4pack = 0;
		break;
	case FRAG_INDEX:
		if (can_4pack)
			powerup_id = POWERUP_MISSILE_FRAG4PACK;
		else
			powerup_id = POWERUP_MISSILE_FRAG;
		break;
	case GUIDED_INDEX:
		if (can_4pack)
			powerup_id = POWERUP_MISSILE_GUIDED4PACK;
		else
			powerup_id = POWERUP_MISSILE_GUIDED;
		break;
	case NAPALMROCKET_INDEX:
		powerup_id = POWERUP_MISSILE_NAPALM;
		can_4pack = 0;
		break;
	case CYCLONE_INDEX:
		powerup_id = POWERUP_MISSILE_CYCLONE;
		can_4pack = 0;
		break;
	case BLACKSHARK_INDEX:
		powerup_id = POWERUP_MISSILE_BLACK_SHARK;
		can_4pack = 0;
		break;
	default:
		Error_NoExit ("player %d, secondary weapon ix %d", weapon_ix);
		return;
	}
	if (can_4pack)
		amount = 4;
	else
		amount = 1;
	amount_ammo -= amount;
//	plr->weapon_ammo[SECONDARY_INDEX + weapon_ix] = amount_ammo;
//// *not in use
//	plr->weapon_ammo[weapon_ix] = amount_ammo;

//	Drop_PlayerWeapon_Remove (pnum, weapon_ix);
//	Drop_PlayerWeapon_AutoSelect (1);
//	Drop_PlayerWeapon_AutoSelect (&objects[plr->objnum], weapon_ix);

	Drop_PlayerWeapon_Drop (pnum, weapon_ix, amount);
//	Drop_PlayerWeapon_Diminish (plr->objnum, weapon_ix, amount);
// yes, also do it (this at least will make the client not getting "low ammo" message from the server, while there is the ammo)
	Drop_SetAmmo (pnum, weapon_ix, amount_ammo);

	Drop_ThrowObject (pnum, powerup_id);

	return;

no_ammo:
	CoopMod_SendMessage (pnum, "no any secondary ammo remain");
	return;

// temp solution
no_ammo_this:
	CoopMod_SendMessage (pnum, "no ammo for this secondary weapon remained");
}

#if 0
void Drop_WeaponSecondary ()
{
	if (! local_role_is_server)
	{
		Drop_LocalDropDenyNotice ();
		return;
	}
	Drop_WeaponSecondary (DMFCBase->GetPlayerNum ());
}
#endif

void Drop_AmmoPrimary (u32 pnum)
{
	player* plr;
	player_weapon* plr_weapon;
	short amount_ammo;
	u32 weapon_ix;
	u32 powerup_id;

	if (pnum >= MAX_PLAYERS)
		return;
	plr = &players[pnum];
	plr_weapon = &plr->weapon[PW_PRIMARY];
	weapon_ix = plr_weapon->index;
//	Screen_Restore ();
	if (weapon_ix >= 10)
		return;
	amount_ammo = plr->weapon_ammo[weapon_ix];

	switch (weapon_ix)
	{
	case VAUSS_INDEX:
		if (amount_ammo < 1250)
			goto too_low_ammo;
		amount_ammo -= 1250;
		powerup_id = POWERUP_AMMO_VAUSS;
//		uses_ammo = 1;
		break;

	case MASSDRIVER_INDEX:
		if (amount_ammo < 5)
			goto too_low_ammo;
		amount_ammo -= 5;
		powerup_id = POWERUP_AMMO_MASSDRIVER;
//		uses_ammo = 1;
		break;

	case NAPALM_INDEX:
// *100 is 10.0
		if (amount_ammo < 100)
			goto too_low_ammo;
		amount_ammo -= 100;
		powerup_id = POWERUP_NAPALM_FUEL;
		break;

	case LASER_INDEX:
	case MICROWAVE_INDEX:
	case PLASMA_INDEX:
	case FUSION_INDEX:
	case SUPER_LASER_INDEX:
	case EMD_INDEX:
	case OMEGA_INDEX:
		Drop_Energy (pnum);
//		break;
		return;

	default:
//		DLLAddHUDMessage ("drop weapon ammo :  server is sending wrong weapon ix %d", weapon_ix);
		Assert(0);
		return;
	}

	Drop_SetAmmo (pnum, weapon_ix, amount_ammo);

	Drop_ThrowObject (pnum, powerup_id);

	return;

// to protect against cheating-recharging, goto here
too_low_ammo:
	CoopMod_SendMessage (pnum, "low ammo");
}

#if 0
void Drop_AmmoPrimary ()
{
	if (! local_role_is_server)
	{
		Drop_LocalDropDenyNotice ();
		return;
	}
	Drop_AmmoPrimary (DMFCBase->GetPlayerNum ());
}
#endif

void Drop_AmmoSecondary (u32 pnum)
{
	CoopMod_SendMessage (pnum, "unimplemented");
}

#if 0
void Drop_AmmoSecondary ()
{
	if (! local_role_is_server)
	{
		Drop_LocalDropDenyNotice ();
		return;
	}
	Drop_AmmoSecondary (DMFCBase->GetPlayerNum ());
}
#endif

void Drop_Enhancement (u32 pnum)
{
	CoopMod_SendMessage (pnum, "unimplemented");
}

#if 0
void Drop_Enhancement ()
{
	if (! local_role_is_server)
	{
		Drop_LocalDropDenyNotice ();
		return;
	}
	Drop_Enhancement (DMFCBase->GetPlayerNum ());
}
#endif

// ----- drop a component routines -----


void Drop_Init ()
{
	CMD_ADD_CMD("DropShield", Drop_Shield);
	CMD_ADD_CMD("DS", Drop_Shield);
	CMD_ADD_CMD("DS5", Drop_Shield5);
	CMD_ADD_CMD("DropEnergy", Drop_Energy);
	CMD_ADD_CMD("DE", Drop_Energy);
	CMD_ADD_CMD("DE5", Drop_Energy5);
	CMD_ADD_CMD("DropWeaponPrimary", Drop_WeaponPrimary);
	CMD_ADD_CMD("DWP", Drop_WeaponPrimary);
	CMD_ADD_CMD("DropWeaponSecondary", Drop_WeaponSecondary);
	CMD_ADD_CMD("DWS", Drop_WeaponSecondary);
// simplified
	CMD_ADD_CMD("DropAmmo", Drop_AmmoPrimary);
	CMD_ADD_CMD("DA", Drop_AmmoPrimary);
	CMD_ADD_CMD("DropAmmoPrimary", Drop_AmmoPrimary);
	CMD_ADD_CMD("DAP", Drop_AmmoPrimary);
	CMD_ADD_CMD("DropAmmoSecondary", Drop_AmmoSecondary);
	CMD_ADD_CMD("DAS", Drop_AmmoSecondary);
	CMD_ADD_CMD("DropEnhancement", Drop_Enhancement);
	CMD_ADD_CMD("DEN", Drop_Enhancement);
}

bool Drop_Packet_Dispatcher (int pnum, u32 cmd)
{
	switch (cmd)
	{
	case PACKCMD_DROP_SHIELD:
		Drop_Shield (pnum, 1);
		break;
	case PACKCMD_DROP_SHIELD5:
		Drop_Shield (pnum, 5);
		break;
	case PACKCMD_DROP_ENERGY:
		Drop_Energy (pnum, 1);
		break;
	case PACKCMD_DROP_ENERGY5:
		Drop_Energy (pnum, 5);
		break;
	case PACKCMD_DROP_WEAPON_PRIMARY:
		Drop_WeaponPrimary (pnum);
		break;
	case PACKCMD_DROP_WEAPON_SECONDARY:
		Drop_WeaponSecondary (pnum);
		break;
	case PACKCMD_DROP_AMMO_PRIMARY:
		Drop_AmmoPrimary (pnum);
		break;
	case PACKCMD_DROP_AMMO_SECONDARY:
		Drop_AmmoSecondary (pnum);
		break;
	case PACKCMD_DROP_ENHANCEMENT:
		Drop_Enhancement (pnum);
		break;
	default:
		return 0;
	}
	return 1;
}

// set the ammo of the local client
// is called by a packet request from the server
// relative operation; lets to not use the server side ammo amount, since that amont is invalid for vauss and napalm (although could be valid for MD in some cases)
// *yes, relative ariphmetics are guaranteed to give a wrong results at some moment
//void Drop_ClientWeaponDiminishAmmo (u32 weapon_ix)
// absolute
void Drop_ClientWeapon_SetAmmo (u32 weapon_ix, ushort amount)
{
	player* plr;
//	player_weapon* plr_weapon;
	u32 pnum;

	pnum = DMFCBase->GetPlayerNum ();
	Assert(pnum < MAX_PLAYERS);
	plr = &players[pnum];
//	plr_weapon = &plr->weapon[PW_PRIMARY];
//	weapon_ix = plr_weapon->index;
	if (weapon_ix >= 20)
	{
		DLLAddHUDMessage ("drop weapon ammo :  server is sending too high weapon ix %d", weapon_ix);
		return;
	}

//	Beep (10000, 20);
//	Sleep (20);
// when a client is shooting, vauss and napalm ammo do not diminish; only md ammo does
	switch (weapon_ix)
	{
	case VAUSS_INDEX:
		break;
	case MASSDRIVER_INDEX:
		break;
	case NAPALM_INDEX:
		break;
// missiles - let it
	case CONCUSSION_INDEX:
	case HOMING_INDEX:
	case IMPACTMORTAR_INDEX:
	case SMART_INDEX:
	case MEGA_INDEX:
	case FRAG_INDEX:
	case GUIDED_INDEX:
	case NAPALMROCKET_INDEX:
	case CYCLONE_INDEX:
	case BLACKSHARK_INDEX:
		break;
// energy weapons
	case LASER_INDEX:
	case MICROWAVE_INDEX:
	case PLASMA_INDEX:
	case FUSION_INDEX:
	case SUPER_LASER_INDEX:
	case EMD_INDEX:
	case OMEGA_INDEX:
	default:
		DLLAddHUDMessage ("drop weapon ammo :  server is sending wrong weapon ix %d", weapon_ix);
		return;
	}
	plr->weapon_ammo[weapon_ix] = amount;
}

void Drop_CountAmmo (u32 pnum, u32 amount)
{
	player* plr;
	player_weapon* plr_weapon;
	short amount_ammo;
	u32 ammo_usage_full;
	u32 ammo_usage;
	u32 weapon_ix;
	ship* sh;
	otype_wb_info* wb;

// server player doesn't need this
	Assert(pnum);
	if (pnum >= MAX_PLAYERS)
		return;
	plr = &players[pnum];
	plr_weapon = &plr->weapon[PW_PRIMARY];
	weapon_ix = plr_weapon->index;
//	Screen_Restore ();
	if (weapon_ix >= 10)
		return;
	amount_ammo = plr->weapon_ammo[weapon_ix];

// get ammo usage
	sh = &ships[plr->ship_index];
	wb = &sh->static_wb[weapon_ix];
	ammo_usage_full = wb->ammo_usage;

// there are two shots, in sum they take this value, so correct it
// could actually avoid of all this dirt, if the event wouldn't be subdivided (there possibly is one)
	u32 guns_num;
// *should also be used this, but I'll skip
	wb->num_masks;
//	guns_num = 0;
// count bits
	guns_num = wb->gp_fire_masks[0];
__asm
{
	mov ebx,[guns_num]
// hmmm, and there is no single intel asm instruction to do that (I would use)
	mov eax,0
	mov ecx,8
loop0:
	shr ebx,1
	adc eax,0
	loop loop0
	mov [guns_num],eax
}
	if (! guns_num)
		guns_num = 1;
// *do not care about the remainder; instead better care how to exclude this subroutine at all (which atm isn't possible)
	ammo_usage = ammo_usage_full / guns_num;

// need to diminish only vauss and napalm ammo (md ammo is controlled correct by the engine)
	switch (weapon_ix)
	{
	case VAUSS_INDEX:
		amount_ammo -= ammo_usage;
		if (amount_ammo < 0)
			amount_ammo = 0;
		break;
	case NAPALM_INDEX:
		amount_ammo -= ammo_usage;
		if (amount_ammo < 0)
			amount_ammo = 0;
		break;
	default:
		return;
	}
	plr->weapon_ammo[weapon_ix] = amount_ammo;
}


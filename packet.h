#pragma once

#include "types.h"


enum
{
	PACKTYPE_CMDTOSERVER,
// *for a properly working server, this wouldn't be necessary
	PACKTYPE_CMDTOCLIENT,
// yup, this was a bad idea, to use a relative ariphmetics; anyway, this crap should not exist at all, in any shape; ammo should count a server and not a client; client should only assume + get the real amount from time to time
//	PACKTYPE_DROPWEAPONDIMINISHAMMO_TOCLIENT,
	PACKTYPE_DROPWEAPON_SETAMMO_TOCLIENT,
	PACKTYPES_NUM
};

// define any of it once, and not redefine anymore
enum
{
	PACKCMD_NONE,
	PACKCMD_DROP_SHIELD,
	PACKCMD_DROP_SHIELD5,
	PACKCMD_DROP_ENERGY,
	PACKCMD_DROP_ENERGY5,
	PACKCMD_DROP_WEAPON_PRIMARY,
	PACKCMD_DROP_WEAPON_SECONDARY,
	PACKCMD_DROP_AMMO_PRIMARY,
//// senseless for this game
//// *senseless, but I wrote it, and will not change
// almost senseless, but not completely
	PACKCMD_DROP_AMMO_SECONDARY,
	PACKCMD_DROP_ENHANCEMENT,
	PACKCMDS_NUM
};

// *nothing yet
enum
{
	PACKCMD_CL_NONE,
// eehh bad, it isn't just a command; it is with an arguments
//	PACKCMD_CL_SET_AMMO,
	PACKCMDS_CL_NUM
};


//void Packet_Dispatcher (int pnum, u32 cmd);
//void Packet_Receiver_Cmd (byte* data);
void Packet_Receiver_Cmd_ToServer (byte* data);
void Packet_Receiver_Cmd_ToClient (byte* data);
//void Packet_Receiver_DropWeaponDiminishAmmo_ToClient (byte* data);
void Packet_Receiver_DropWeapon_SetAmmo_ToClient (byte* data);
void Packet_Send_Cmd_ToServer (int pnum, u32 cmd);
void Packet_Send_Cmd_ToClient (int pnum, u32 cmd);
//void Packet_Send_DropWeaponDiminishAmmo_ToClient (int pnum, u32 weapon_ix);
void Packet_Send_DropWeapon_SetAmmo_ToClient (int pnum, u32 weapon_ix, u32 amount);


// by Alexander Ilyin

#include "packet.h"
#include "drop.h"
#include "idmfc.h"
#include "screen.h"
#include "coop.h"


// and about the engine packets ...
// send
// + probably +
// 01 game info
//// 75 my msg
// - probably -
// look for some more in 'Hooks_Packet()'


void Packet_Receiver_Cmd_ToServer (byte* data)
{
	bool r;
	int pnum;
	byte cmd;
	int count;

//	Screen_Restore ();
	count = 0;
//	pnum = MultiGetInt (data, &count);
// the shittiest hack :  DMFC doesn't give us pnum who send packet (-13, perhaps 4 bytes int)
// (though possibly could avoid of it, parsing a 'EVT_CLIENT_GAMESPECIALPACKET', but I haven't tried)
	pnum = data[-13];
	if (pnum >= 0x20)
		return;
	cmd = MultiGetInt (data, &count);
	r = Drop_Packet_Dispatcher (pnum, cmd);
}

// *for a properly working server, any such appeals at a client wouldn't be necessary
void Packet_Receiver_Cmd_ToClient (byte* data)
{
}

#if 0
void Packet_Receiver_DropWeaponDiminishAmmo_ToClient (byte* data)
{
	int pnum;
	int count;
	u32 weapon_ix;

	count = 0;
// *not in use
	pnum = data[-13];
	if (pnum >= 0x20)
		return;
	weapon_ix = MultiGetInt (data, &count);
	Drop_ClientWeaponDiminishAmmo (weapon_ix);
}
#else
void Packet_Receiver_DropWeapon_SetAmmo_ToClient (byte* data)
{
	int pnum;
	int count;
	byte weapon_ix;
	ushort amount;

	count = 0;
// *not in use
	pnum = data[-13];
	if (pnum >= 0x20)
		return;
	weapon_ix = MultiGetByte (data, &count);
	amount = MultiGetUshort (data, &count);
	Drop_ClientWeapon_SetAmmo (weapon_ix, amount);
}
#endif

// *"send packet" here doesn't necessarily mean that a packet will be sent; in the case of server, we would just locally dispatch it
void Packet_Send_Cmd_ToServer (int pnum, u32 cmd)
{
	bool r;
	int count;
	byte data[MAX_GAME_DATA_SIZE];

//	Screen_Restore ();
	if (! pnum)
	{
		r = Drop_Packet_Dispatcher (pnum, cmd);
		return;
	}
	count = 0;
	DMFCBase->StartPacket(data, PACKTYPE_CMDTOSERVER, &count);
//	MultiAddInt (pnum, data, &count);
	MultiAddInt (cmd, data, &count);
//	if (pnum)
	DMFCBase->SendPacket (data, count, SP_SERVER);
//	else
//		Packet_Receiver_Cmd_ToServer (data);
}

void Packet_Send_Cmd_ToClient (int pnum, u32 cmd)
{
}

// I don't see any even close 'MSAFE_xxxx', to update an ammo amount at a client
// locally, 'plr->weapon_ammo' even is changed not going through 'MSafe_CallFunction()'; although goes through 'MSafe_DoPowerup()'
#if 0
void Packet_Send_DropWeaponDiminishAmmo_ToClient (int pnum, u32 weapon_ix)
{
	int count;
	byte data[MAX_GAME_DATA_SIZE];

//	Screen_Restore ();
	if (! pnum)
		return;
	count = 0;
	DMFCBase->StartPacket(data, PACKTYPE_DROPWEAPON_SETAMMO_TOCLIENT, &count);
	MultiAddInt (weapon_ix, data, &count);
	DMFCBase->SendPacket (data, count, pnum);
}
#else
void Packet_Send_DropWeapon_SetAmmo_ToClient (int pnum, u32 weapon_ix, u32 amount)
{
	int count;
	byte data[MAX_GAME_DATA_SIZE];

//	Screen_Restore ();
	if (! pnum)
		return;
	count = 0;
	DMFCBase->StartPacket(data, PACKTYPE_DROPWEAPON_SETAMMO_TOCLIENT, &count);
	MultiAddByte (weapon_ix, data, &count);
	MultiAddUshort (amount, data, &count);
	DMFCBase->SendPacket (data, count, pnum);
}
#endif


// by Alexander Ilyin

#include "control.h"
#include "error.h"
#include "packet.h"
#include "coop.h"
#include "coop_mod_cmd.h"


// its just a single event, without a keyup pair
// "send packet" doesn't here mean that a packet will really be sent; if we are the server - we will get it without a send
void Control_OnKeypress (dllinfo* data)
{
	byte key;
	bool ctrled;
	bool shifted;
	u32 pnum;

	Assert(data);
//	if (DMFCBase->GetLocalRole() == LR_SERVER)
//		Beep (1000, 50);
//	Screen_Restore ();
//	if (data->input_key != 0x19)
//	{
//		Beep (10000, 50);
//		Sleep (50);
//		eventnum = eventnum;
//	}
//	if ((data->input_key & 0xff) != 0x9d)
//		eventnum = eventnum;
// hmmm, modifiers aren't declared anywhere; 400 Ctrl, 200 Alt, 100 Shift, no Win key
// futther, only Right Shift modifier disables the other possible ingame meaning of a key, so have to use it
	ctrled = !! (data->input_key & 0x0400);
	shifted = !! (data->input_key & 0x0100);
	key = data->input_key & 0xff;
	pnum = DMFCBase->GetPlayerNum ();
// **keys atm are hardcoded and not configurable; also unbind fire from Ctrl (err... Shift I mean) to use them
	if (shifted && key == K_U)
	{
		Packet_Send_Cmd_ToServer (pnum, PACKCMD_DROP_SHIELD);
//		Screen_Restore ();
		data->iRet = 1;
	}
	if (shifted && key == K_I)
	{
		Packet_Send_Cmd_ToServer (pnum, PACKCMD_DROP_SHIELD5);
		data->iRet = 1;
	}
	if (shifted && key == K_O)
	{
		Packet_Send_Cmd_ToServer (pnum, PACKCMD_DROP_ENERGY);
		data->iRet = 1;
	}
	if (shifted && key == K_P)
	{
		Packet_Send_Cmd_ToServer (pnum, PACKCMD_DROP_ENERGY5);
		data->iRet = 1;
	}
	if (shifted && key == K_J)
	{
		Packet_Send_Cmd_ToServer (pnum, PACKCMD_DROP_WEAPON_PRIMARY);
		data->iRet = 1;
	}
	if (shifted && key == K_K)
	{
		Packet_Send_Cmd_ToServer (pnum, PACKCMD_DROP_AMMO_PRIMARY);
		data->iRet = 1;
	}
	if (shifted && key == K_L)
	{
		Packet_Send_Cmd_ToServer (pnum, PACKCMD_DROP_WEAPON_SECONDARY);
		data->iRet = 1;
	}
//	if (shifted && key == K_COMMA)
	if (shifted && key == K_PERIOD)
	{
		CoopMod_RunMacro ();
		data->iRet = 1;
	}
}


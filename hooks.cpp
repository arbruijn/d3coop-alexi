// by Alexander Ilyin
// oh, compiler dependent code :) ; but it actually is platform dependent also - the code patches Windows exe/dll
//// and here is 'ulong' instead of 'u32' for addresses, because "patches.cpp" requires it; not because I am so care about it
// now using 'u8*' instead of 'void*' or 'ulong' or 'u32' for addresses, because it is more convenient, and because "patches.cpp" requires it

#include "hooks.h"
#include <windows.h>
#include <imagehlp.h>
#include "error.h"
#include "gamedll_header.h"
#include "coop_mod.h"
#include "coop.h"
#include "chat.h"
#include "patches.h"
#include "macros.h"
#include "log_file.h"


bool hooks_initialized = 0;
u8* base_address_exe = 0;
//ulong base_address_exe = 0;
u8* base_address_dmfc = 0;
//ulong base_address_dmfc = 0;
DWORD timestamp_main = 0;
DWORD timestamp_dmfc = 0;
// this actually should be in "patches.cpp"
int version_main = 0;
int version_dmfc = 0;
char* version_main_name = 0;
char* version_dmfc_name = 0;
// version dependent correction
// don't use it - often can be wrong
//ulong hooks_base_offset_main = 0;
// + test +
#define HOOKS_TEST 0
ulong hook_return__test = 0;
u32 netbuf_level_min = -1;
u32 netbuf_level_max = 0;
// - test -
u8* hook_return__packet_parse = 0;
u8* hook_return__chat_messages = 0;
u8* hook_call__chat_message_out = 0;
u8* hook_call__chat_message_out_nondedi = 0;
// *remove me
//u8* hook__chat_message_buffer = 0;
//u8* hook_return__chat_message_out = 0;
u8* hook_return__f4 = 0;
u8* hook_return__gb_release = 0;
u8* hook_return__no_packet_parsing = 0;
u8* hook_00478b10 = 0;
u8* hook_return__respawn_powerup = 0;
u8* hook_pointer__respawn_counter = 0;
u8* hook_return__packet_put = 0;
// + indicators +
//bool hooks_indicator__f4 = 0;
int hooks_indicator__f4 = -1;
//bool hooks_indicator__gb_release = 0;
int hooks_indicator__gb_release = -1;
// - indicators -
u32 hook_store_eax = 0;
u32 hook_store_plix = 0;
// + settings +
// block of GB relative packets (will not work for player #0)
bool hook_block_guidebot_packet = 1;
bool hooks_indicate__socket_traffic = 0;
//bool hooks_indicate__socket_traffic_output = 0;
//bool hooks_indicate__socket_traffic_input = 0;
//bool hooks_send_packets_5e = 0;
bool hooks_send_packets_5e = 1;
// - settings -
//SOCKET hooks__socket = 0;
u8* hooks__socket = 0;
//u32 hooks__socket_traffic = 0;
u32 hooks__socket_traffic_output = 0;
u32 hooks__socket_traffic_input = 0;


// *comments are about version 1.4 NoCD "main.exe", if version is not shown
// *comments are about version 1.4 "dmfc.dll", if version is not shown
// packets parsing subroutine
// pattern 53568B74240C8D0C
// begins at :
// 1.4 xx - 480eb0
// 1.4 NoCD - 4812b0
// 1.5 beta - 4812d0
// packets parsing table start is at :
// 1.4 xx - 4818b0
// 1.4 NoCD - 481cb0
// 1.5 beta - 481cd0
// 006FD930 supposely is packet buffer
// *table indexes are -1, due to "dec ebx"


// *asm has word "offset" reserved
u8 Hooks_PacketBuffer_GetSafe (u8* buffer, u32 offs)
{
	u8 b;

	try
	{
#if 0
	__asm
	{
		mov eax, [buffer]
		add eax, [offs]
// unsafe code
		mov al, [eax]
		mov [b], al
	}
#endif
// unsafe code
		b = buffer[offs];
	}
	catch (...)
	{
		DLLAddHUDMessage ("----- Hooks_PacketBuffer_GetSafe () exception, buffer %08x, offset %u -----", buffer, offs);
	}

	return b;
}

// both, server and client incoming traffics go through here (packets of all kind)
// return 0, if packet needs to be ignored
bool Hooks_Packet (u8* packet, u16 packet_length, u32 ip, u16 port)
{
	bool r;
	u8 type;
	ushort length;

	hooks__socket_traffic_input += packet_length;
//	ip = ntohl (ip);
	ip = SWAPINT(ip);
//	port = SWAPSHORT(port);
	type = packet[0];
// 2 bytes length (together with three bytes of type and length)
	length = *(u16*)&packet[1];

// test
#if 0
//#if 1
//	if (local_role_is_server)
	if (! local_role_is_server)
	{
		if (type == 0x5e)
		{
			FILE* f;
			f = fopen ("packets.log", "a");
			if (f)
			{
//			fprintf (f, "%u\n", type);
				FORJ0(packet_length)
				{
					fprintf (f, "%02x ", packet[j]);
				}
				fprintf (f, "\n");
				fprintf (f, "\n");
				fclose (f);
			}
		}
	}
#endif
// 473690 is called to begin a packet

// incoming to server packet types :
// 'str' - byte for the length, incl. the ending 0, then 0 ended C string
// *note that some packets (like 07) do not go through the network in their plain shape
// 05 - some ingame packet
// 06 - some connection packet
// 07 - some connection packet, len 24 (is formed on 481f50) :  ..., str - nick, ..., str - ship name, ..., byte - pps, ..., str - gb name.
// 09 - some connection packet
// 0c - first packet; only this one incomes, when banned :  <no data>
// 11 - :  <no data>
// 12 - some connection packet
// 14 - :  <no data>
// 15 - some connection packet
// 1e - game ping from the "Direct TCP/IP Games" screen
// 2b - some connection packet
// 4b - some connection packet
// 54 - some connection packet
// 55 - :  <no data>
// 5c - some ingame packet
// 72 - game info request (button "i") from the "Direct TCP/IP Games" screen
// type sequences :
// 0c - when banned
// 0c, 3f, 27 - when have no mission

// server to client :
// 5e - some

// test values in a packet
#if 0
//#if 1
	if (type == 7)
	{
		packet[0x18] = 0x0b;
	}
#endif

// test
#if 0
//#if 1
	if (type == 0x5e)
		return 0;
#endif

// have to block Geordon for some levels :)
// hehe, better not place any ban here statically; better use (implement) lists; because I set, and have forgot where I have set it :)
#if 0
	if ((ip >> 24) == 96)
		if (((ip >> 16) & 0xff) == 254)
			return 0;
#endif

	if (type == 63)
		CoopMod_ConnectionPacket (ip, port);

	r = 1;
//	r = 0;

	return r;
}

#if 0
bool Hooks_Message_Sending (char* str)
{
	char* message;

//**
#if 0
	message = strstr (str, " says: ");
	if (! message)
		return 0;
	message += 7;
#endif
	Chat_Message_Sent (str);

	return 1;
}
#endif

__declspec(naked) void Hooks_Test ()
{
// test netbuf on "defcon1", room 116 (trains entrance) and 376 (that entrance forcefield switch)
	static char netbuf_fmt[] = "netbuf level %04x";
	__asm
	{
		pushad

// ebx currently contains first free packet ix - it is not what I need; count used packets number instead
		lea eax,[ebp+0x8a0]
		mov ebx,0
// MAXNETBUFFERS
		mov ecx,0x96
count_loop:
		cmp [eax],0
		jz not_count
		inc ebx
not_count:
		add eax,4
		dec ecx
		jnz count_loop

#if 0
		push ebx
//		push netbuf_fmt
//		push dword ptr netbuf_fmt
		push offset netbuf_fmt
//		push edi
		push -1
		call CoopMod_SendMessage
		add esp,0x0c
#else
		mov eax,netbuf_level_min
		cmp eax,ebx
		jbe test_max
		mov netbuf_level_min,ebx
test_max:
		mov eax,netbuf_level_max
		cmp eax,ebx
		jae finish
		mov netbuf_level_max,ebx
finish:
#endif
		popad
		mov esi,[esp+0x290]
		jmp [hook_return__test]
	}
}

// [481d30] 4815c8, ix=21
// that is very bad; client can fake any message
// must search for string " says: " in the received message, and after that begins the message
__declspec(naked) void Hooks_Message_In ()
{
	__asm
	{
		mov [hook_store_eax], eax
// this thingy must be 1, look in subr. 00478b10
#if 0
		mov eax, [base_address_exe]
		add eax, [hooks_base_offset_main]
// no, will not work; from 006F4834 it becomes 006fb5cc in 1.4 xx for example
#if 0
		mov al, [eax + 0x006F4834 - 0x00400000]
#else
// get the address of that var straight from the subroutine
		mov eax, [eax + 0x00478b10 - 0x00400000 + 7]
#endif
#else
		mov eax,[hook_00478b10]
		mov eax, [eax+7]
#endif
		mov al, [eax]
		cmp al, 1
		jne go_normally
		mov eax,[esp+8]
		mov [hook_store_plix],eax
		pushad
		add esi,6
		push esi
//		call Hooks_Message_In_Parse
		call Chat_Message_In_Parse
		add esp,4
		test al,al
		popad
		je go_normally
		jmp [hook_return__no_packet_parsing]
go_normally:
		mov eax,[hook_store_eax]
		jmp [hook_return__chat_messages]
	}
}

// [481e64] 4b1b7b, ix=6e
__declspec(naked) void Hooks_F4 ()
{
	__asm
	{
		mov [hook_store_eax], eax
		mov al, [hook_block_guidebot_packet]
		test al,1
		jz parse_packet
//		mov [hooks_indicator__f4], 1
		mov eax, [esp + 8]
		mov [hooks_indicator__f4], eax
// eax seem can be used
		jmp [hook_return__no_packet_parsing]
parse_packet:
		mov eax, [hook_store_eax]
		jmp [hook_return__f4]
	}
}

// this actually isn't just GB release, but any inventory item activation, and third weapons row
// [481d98] 48188f, ix=3b
// esp+8, esp+18, esp+28 contain player index
__declspec(naked) void Hooks_GB_Release ()
{
	__asm
	{
		mov [hook_store_eax], eax
		mov al, [hook_block_guidebot_packet]
		test al,1
		jz parse_packet
// don't take that value from stack, take from esi
//		mov eax, [esp + 0x34]
		pushad
		push 3
//		push eax
		push esi
		call Hooks_PacketBuffer_GetSafe
		add esp, 8
		cmp al, OBJ_ROBOT
		popad
		je no_packet_parsing
parse_packet:
		mov eax, [hook_store_eax]
		jmp [hook_return__gb_release]
no_packet_parsing:
//		mov [hooks_indicator__gb_release], 1
		mov eax, [esp + 8]
		mov [hooks_indicator__gb_release], eax
		jmp [hook_return__no_packet_parsing]
	}
}

// packet type values range is 01-7b
__declspec(naked) void Hooks_PacketParse ()
{
	__asm
	{
		pushad
// esp + 0x42
		mov ax,[esp+0x62]
		push eax
// esp + 0x3c
//		push [esp+0x5c]
		push [esp+0x60]
// esp + 0x10
//		push [esp+0x34]
		push [esp+0x38]
		push esi
		call Hooks_Packet
		add esp,16
		test al,al
		popad
		jz ignore
//		and ebx,0xff
		mov edx,1
		jmp [hook_return__packet_parse]
ignore:
//		jmp [hook_return__packet_parse]
		push edi
// *I don't see that it would skip any serious code
		jmp [hook_return__no_packet_parsing]
	}
}

// one more tap, necessary for own written messages; read about it in "chat.cpp" beginning
// perhaps better would be, to tap it with various game messages, like guidebot being included
// GetString (117) request (dedi) is at 430390, 430484 (seems is one of 'switch' cases); 4301f0, 43024e.
// GetString (117) request (nondedi) is at (three places) 442dee, 45220a, 45229f; 451e70, 451eda.
// pattern for both places :  6a 75, e8
__declspec(naked) void Hooks_Message_Out ()
{
	__asm
	{
//// take the second arg
// take the first arg
		mov eax,[esp+0x08]
		pushad
		push eax
		call Chat_Message_Sending
		add esp,4
		test al,al
		popad
		jz proceed
// do not send or print
		ret
proceed:
		jmp [hook_call__chat_message_out]
	}
}

// duplicate of the function for the nondedi :/
__declspec(naked) void Hooks_Message_Out_Nondedi ()
{
	__asm
	{
// take the fourth arg for the next function request (for not this one)
		mov eax,[esp+0x0c]
		pushad
		push eax
//		push 0x006a63b0
//		mov eax,[hook__chat_message_buffer]
//		push eax
		call Chat_Message_Sending
		add esp,4
		test al,al
		popad
		jz proceed
// do not send or print
// exception (when returning from 451eda request) :/
//		mov eax,0
//		ret
// pop the return address
		pop eax
// remove our arg and two args more
		add esp,12
// jump to 'jne'; the result of this operation is NE, as necessary
		add eax,0x00451efd-0x00451edf
		jmp eax
proceed:
		jmp [hook_call__chat_message_out_nondedi]
	}
}

void Hooks_ReportRespawnBug (u32 counter_value)
{
	CoopMod_SendMessage (-1, "server :  respawn bug (%u) fix attempt", counter_value);
}

// and finally :  fast fix for the respawn bug
// if enters here - means the subroutine has got request to respawn, but can't find what to respawn, and mostly probably has happened a respawn bug
__declspec(naked) void Hooks_RespawnPowerup ()
{
	__asm
	{
		pushad
		mov eax,hook_pointer__respawn_counter
		mov eax,[eax]
		push eax
		call Hooks_ReportRespawnBug
		add esp,4
// reset the counter (drop whatever the engine has attempted to do), so that the engine would be able to register a new taken powerups again
		mov eax,hook_pointer__respawn_counter
		mov [eax],0
		popad
		jmp [hook_return__respawn_powerup]
	}
}

// what is all this about :
// some missions do random disconnects, and are inplayable because of that
// Windmine 6 at trains, after room 258; switch to open forcefield is in room 376/377 (buffer overflow with packets 5e)
// Alien Territory :  random disconnects, like once per 2 minutes (buffer overflow with packets 5e)
// Alien Territory (at.mn3) :  32 KB/s output traffic per player (while normally the traffic is a packet of 7 bytes per 5 seconds)

// no, too much extra stuff is necessary to manage that
#if 0
// various test checks
void Hooks_PacketPut_Checks ()
{
	static u32* sock_1_pack_0 = (u32*)0x00feb3a0;

// the packet should be sent, and this var must be cleared
	if (sock_1_pack_0 != 0)
		sock_1_pack_0 = sock_1_pack_0;
}

//u8* hook_call__packet_put = 0;
u8* hook_call__packet_send_initiation = 0;
u8* hook_call__poll_sockets = 0;
// **not for other versions (yet)
u8* hook_call__packets__some_var_1 = (u8*)0x00fe9c60;

__declspec(naked) void Hooks_PacketPut ()
{
	__asm
	{
		add esp,0x0274
// setup 55aa50 to call 559540
// *not sure if it is safe
//		call 0x00559420
		call [hook_call__packet_send_initiation]
// make 559a02 not jump
// *not sure if it is safe
//		mov al,[0x00fe9c60]
//		mov [0x00fe9c60],byte ptr 1
//		mov byte ptr [0x00fe9c60],1
//		mov [hook_call__packets__some_var_1],1
		mov eax,[hook_call__packets__some_var_1]
		mov al,[eax]
		push eax
		mov eax,[hook_call__packets__some_var_1]
		mov byte ptr [eax],1
//		call 0x0055aa50
		call [hook_call__poll_sockets]
		pop eax
//		mov [0x00fe9c60],al
//		mov byte ptr [0x00fe9c60],al
//		mov byte ptr [00fe9c60h],al
		mov eax,[hook_call__packets__some_var_1]
		mov [eax],al
		call Hooks_PacketPut_Checks
		jmp hook_return__packet_put
	}
}
#endif

//// give our subr inplace of 559120
// *actually, can't send all packets this way; some packets have to be stored in the buffer, and the client must acknowledge its reception
// returns 1 if sent, or 0, what means that the original subr must be called
// *'pnum' - if it really is pnum (!); yes, looks as pnum (tested with two clients, one of which has been changed from pnum #2 to pnum #1)
// *looks like this hook is relative only to packets 5e
// *can't send all of these packets without buffering; some of them contain door openings
// door open in Retribution level 1 looks as :  5e 06 00 ab 06/17/18 00
// door close is not being sent with 5e
// Alien Territory flooding packets look as :  5e 41/43 00 67 ...
// Windmine 6 flooding packets look as :  5e 3a 00 40 ...
// Retribution level 6 boss gives a flow of :  '9e' is 'MSAFE_MISC_LEVELGOAL', '6f' is 'MSAFE_SOUND_OBJECT', '40' is 'MSAFE_OBJECT_WORLD_POSITION', '96' is 'MSAFE_MISC_HUD_MESSAGE'
// 'ab' is 'MSAFE_DOOR_ACTIVATE', '67' is 'MSAFE_WEATHER_LIGHTNING_BOLT', '40' is 'MSAFE_OBJECT_WORLD_POSITION'
//// so lets tap only those 5e packets which are long (would also be good to detect the traffic, but it is extra work)
// *note, the content of this function is not anymore relative at the whole output traffic, but only at packets 5e, and only at a particular ones of 5e packets
#if 1
// 559120 :  void PacketPut (void* some_ptr, u32 pnum, u8* data, u32 len)
//void Hooks_PacketPut (void* some_ptr, u32 pnum, u8* data, u32 len)
//void Hooks_PacketPut (u32 pnum, u8* data, u32 len, bool some)
bool Hooks_PacketPut_TrySend (u32 pnum, u8* data, u32 length, bool some)
{
#if 0
	int i;
	i = MSAFE_DOOR_ACTIVATE;
	i = MSAFE_WEATHER_LIGHTNING_BOLT;
	i = MSAFE_OBJECT_WORLD_POSITION;
	i = MSAFE_MISC_LEVELGOAL;
	i = MSAFE_SOUND_OBJECT;
	i = MSAFE_MISC_HUD_MESSAGE;
#endif
// go to original 559120 subr
// mrgh, needs even more code ... I will not try
#if 0
typedef void PacketPut_t (u32 pnum, u8* data, u32 len, bool some);
	_asm mov eax,0x01008754
	_asm mov eax,[eax]
	;
	((PacketPut_t*)(0x00559120 + 5)) (pnum, data, length, some);
#endif

#if 0
	FILE* f;
	f = fopen ("packets_log.txt", "ab");
	if (f)
	{
		fprintf (f, "%02x\n", *data);
		fclose (f);
	}
#endif

	hooks__socket_traffic_output += length;
//	return 0;

// *socket is set only at 00558a1c (twice), and I don't expect it to be changed
	bool r;
	int i_r;
	SOCKADDR_IN addr_in = {0};
	u8 buf[1500];
	u32 type;
	u32 len;
	u32 msafe_type;

#if 1
	if (! local_role_is_server)
		return 0;
#endif

#if 1
	if (! hooks_send_packets_5e)
		return 0;
#endif
	type = *data;
	len = *(u16*)&data[1];
// this selection solves problem when a player cannot connect soon after level change
// and also, can't beat the final rhomb in the heating chambers (in Alien Territory) without this
// packed on 496070 -> 488710
#if 1
	if (type != 0x5e)
		return 0;
#endif
// size of this packet actually varies very much; from 006 to 121
// so can't just filter them by the size
#if 0
	if (len < 0x27)
//	if (len <= 0x06)
		return 0;
// wow; no, it crashes :)
//	CoopMod_SendMessage (-1, "packet size is %02x", len);
//	LogFile_Chat_Printf ("packet size is %02x", len);
#endif
#if 1
	if (len < 0x04)
		return 0;
	msafe_type = data[3];
// only these two particular cases
// if do otherwise, then at least 'MSAFE_DOOR_xxxx' should go through the reliable buffer
	switch (msafe_type)
	{
	case MSAFE_OBJECT_WORLD_POSITION:
	case MSAFE_WEATHER_LIGHTNING_BOLT:
#if 1
// and actually all other weather events aren't so important
	case MSAFE_WEATHER_RAIN:
	case MSAFE_WEATHER_SNOW:
	case MSAFE_WEATHER_LIGHTNING:
	case MSAFE_WEATHER_LIGHTNING_BOLT_POS:
#endif
// makes disconnections on lets say Retribution level 9, when exploding its spewless boxes in the storage area
// they are short (0a bytes), but the number of them is major
	case MSAFE_SOUND_OBJECT:
		break;
	default:
		return 0;
	}
#endif

//// nonsense ! that data isn't got yet
// lets simplify :  if there is no that data yet available, then call the original subr, instead of trying to get ip/port, hacking the engine deeper
#if 1
	player_info_t* player_info;
	r = CoopMod_TeammateStatus_Get (&player_info, pnum, "Hooks_PacketPut", 0);
	if (! r)
		return 0;
//	A(r);
//	addr_in.sin_addr.s_addr = htonl (player_info->net_address.ip_d);
	addr_in.sin_addr.s_addr = player_info->net_address.ip_d;
	addr_in.sin_port = htons (player_info->net_address.port);
#endif
//// also negative
#if 0
	player_record* pr;
	pr = DMFCBase->GetPlayerRecordByPnum (pnum);
	A(pr);
	addr_in.sin_addr.s_addr = htonl (*(u32*)pr->net_addr.address);
	addr_in.sin_port = htons (pr->net_addr.port);
#endif

	if (length >= sizeof(buf))
		return 0;
// *no idea what this byte is; though when it is 2, looks like the packet is encrypted or compressed
// NWT_UNRELIABLE ?  NF_CHECKSUM ?  RNT_ACK ?  RNF_CONNECTED ?  NW_AGHBN_CANCEL ?
	buf[0] = 1;
	memcpy (&buf[1], data, length);

	addr_in.sin_family = AF_INET;
// *note, normal 'sendto()' is on 55AA1B
//	i_r = sendto (*(SOCKET*)hooks__socket, (const char*)data, length, 0, (SOCKADDR*)&addr_in, sizeof(addr_in));
	i_r = sendto (*(SOCKET*)hooks__socket, (const char*)buf, length + 1, 0, (SOCKADDR*)&addr_in, sizeof(addr_in));

	return 1;
}
#endif

// 559120
// *only packets which has to be put in the reliable buffer go through here
#if 1
__declspec(naked) void Hooks_PacketPut ()
{
	__asm
	{
		push [esp+0x10]
		push [esp+0x10]
		push [esp+0x10]
		push [esp+0x10]
		call Hooks_PacketPut_TrySend
		add esp,0x10
		test al,al
		jz put
		ret

put:
#if 1
// oops, compiles to complete crap :  mov eax,0x01008754
//		mov eax,[0x01008754]
		mov eax,0x01008754
		mov eax,[eax]
#else
		_emit 0xa1
		_emit 0x54
		_emit 0x87
		_emit 0x00
		_emit 0x01
#endif
//		jmp 0x00559120
		jmp [hook_return__packet_put]
	}
}
#endif

// dummy
__declspec(naked) void Hooks_ ()
{
	__asm
	{
	}
}

// --------------------------------------------------------------------------------------------------------------------------------------------------

// a stuff, which isn't straightly relative to us; it is for what we don't offer own subroutines
void Hooks_PatchMainExeOnce ()
{
#if 0
	u8* address;
//	u8 pattern[16];
	u8 buffer[64];
	DWORD num_bytes = -1;

	DLLAddHUDMessage ("\"main.exe\" single patch done ok");
#endif
}

void Hooks_DefineVersions ()
{
	timestamp_main = GetTimestampForLoadedLibrary ((HMODULE)base_address_exe);
	timestamp_dmfc = GetTimestampForLoadedLibrary ((HMODULE)base_address_dmfc);
	switch (timestamp_main)
	{
// *this is German version, so perhaps is EU patched
	case 0x391b3158:
		version_main = VERSION_MAIN_14_XX;
// not sure that it is EU, but it isn't easy to check - the patcher doesn't have the files itself
		version_main_name = "1.4 EU";
		break;
	case 0x3919f324:
		version_main = VERSION_MAIN_14_NOCD;
		version_main_name = "1.4 NoCD";
		break;
	case 0x3bd44cf7:
		version_main = VERSION_MAIN_15_BETA;
		version_main_name = "1.5";
		break;
	default:
		version_main = VERSION_MAIN_UNDEFINED;
	}
	switch (timestamp_dmfc)
	{
	case 0x3919f010:
		version_dmfc = VERSION_DMFC_14;
		version_dmfc_name = "1.4";
		break;
	case 0x3bce06e6:
		version_dmfc = VERSION_DMFC_15_BETA;
		version_dmfc_name = "1.5";
		break;
	default:
		version_dmfc = VERSION_DMFC_UNDEFINED;
	}
}

#if 0
// offset relatively to version 1.4 NoCD
// all three versions - 1.4 xx, 1.4 NoCD, 1.5 beta - are compiled with the same compiler, with the same optimization,
// and have absolutely the same structured subroutines; so no reason to specify separate addresses atm
//u32 Hooks_GetOffset_MainExe ()
void Hooks_GetOffset_MainExe ()
{
	u32 offset;

	Assert(version_main > VERSION_MAIN_UNDEFINED);
	Assert(version_main < VERSIONS_MAIN_NUM);

	switch (version_main)
	{
	case VERSION_MAIN_14_XX:
		offset = -0x400;
		break;
	case VERSION_MAIN_14_NOCD:
		offset = 0;
		break;
	case VERSION_MAIN_15_BETA:
		offset = 0x20;
		break;
	default:
		Error ("Hooks_GetOffset_MainExe () :  invalid version index %d", version_main);
	}

	hooks_base_offset_main = offset;
//	return offset;
}
#endif

// giving an address for NoCD 1.4, receiving back an address for the current version
// region lists are in "diff_bin" folder;
// alignment of all versions :  functions alignment is 0x10; data alignment (perhaps) is 0x04.
// region addresses I've defined with a slightly rough granularity - 0x40
// "exact region" means, that all functions inside are mapped one-to-one ?
//void Hooks_CorrectOffset_MainExe14NoCD (u32& address_out, u32 address_in)
//void Hooks_CorrectOffset_MainExe14NoCD (u32& offset_out, u32 offset_in, int ver_main)
void Hooks_GetDisplacement_MainExe14NoCD (u32& displacement_out, u32 offset_in, int ver_main)
{
// no comment - means is approximate region
	switch (ver_main)
	{
	case VERSION_MAIN_14_XX:
		if (offset_in >= 0x000292c0 && offset_in < 0x000592c0)
		{
			displacement_out = 0x00000040;
			return;
		}
// yes, overlaying with the next range :/ ; this one is from the next bin-comparison attempt, thats why
		if (offset_in >= 0x00072500 && offset_in < 0x00084a80)
		{
			displacement_out = 0xfffffc00;
			return;
		}
// also overlays
		if (offset_in >= 0x000889d0 && offset_in < 0x00088b40)
		{
			displacement_out = 0xfffffc10;
			return;
		}
		if (offset_in >= 0x000792c0 && offset_in < 0x000892c0)
		{
			displacement_out = 0xfffffc00;
			return;
		}
		if (offset_in >= 0x000ac670 && offset_in < 0x000cee40)
		{
			displacement_out = 0xfffffc00;
			return;
		}
		if (offset_in >= 0x00103180 && offset_in < 0x00106260)
		{
			displacement_out = 0xfffffd10;
			return;
		}
		if (offset_in >= 0x00106260 && offset_in < 0x0012b910)
		{
			displacement_out = 0xfffffd00;
			return;
		}
// *below goes ce0 offset
		if (offset_in >= 0x0012b920 && offset_in < 0x00155f70)
		{
			displacement_out = 0xfffffcf0;
			return;
		}
		if (offset_in >= 0x00155ec0 && offset_in < 0x0015ca40)
		{
//			displacement_out = 0x00158e00 - 0x00159120;
			displacement_out = 0xfffffce0;
			return;
		}
		if (offset_in == 0x00fe90a0 - 0x00400000)
		{
			displacement_out = 0x00fefe38 - 0x00fe90a0;
			return;
		}
		break;
	case VERSION_MAIN_14_NOCD:
		displacement_out = 0;
//		break;
		return;
	case VERSION_MAIN_15_BETA:
		if (offset_in >= 0x0002d280 && offset_in < 0x00031f00)
		{
// exact point
//			if (offset_in == 0x00030484)
//			{
//				displacement_out = 0xffffff99;
//				return;
//			}
// exact region
			if (offset_in >= 0x0003042f && offset_in < 0x000304a2)
			{
				displacement_out = 0xffffff99;
				return;
			}
			displacement_out = 0xffffff40;
			return;
		}
// pretty long subr.; begins at 3d720
		if (offset_in >= 0x0003d090 && offset_in < 0x0003e270)
		{
			displacement_out = 0x00000150;
			return;
		}
		if (offset_in >= 0x00050c00 && offset_in < 0x00055840)
		{
			displacement_out = 0x000001d0;
			return;
		}
		if (offset_in == 0x00073afe)
		{
			displacement_out = 0x0007393b - 0x00073afe;
			return;
		}
		if (offset_in >= 0x00078540 && offset_in < 0x0007aac0)
		{
			displacement_out = 0xffffff40;
			return;
		}
		if (offset_in >= 0x0007bbc0 && offset_in < 0x00085500)
		{
// exact region
			if (offset_in >= 0x000811c0 && offset_in < 0x00082a20)
			{
				displacement_out = 0x00000020;
				return;
			}
//			if (offset_in >= 0x00082a20 && offset_in < 0x00082a20)
//				displacement_out = 0x00000060;
			displacement_out = 0xffffffa0;
			return;
		}
		if (offset_in >= 0x000889d0 && offset_in < 0x00088b40)
		{
			displacement_out = 0x000001c0;
			return;
		}
// *below goes 400 offset
		if (offset_in >= 0x000cbc80 && offset_in < 0x000cd090)
		{
			displacement_out = 0x00000410;
			return;
		}
		if (offset_in >= 0x00104d90 && offset_in < 0x00105df0)
		{
			displacement_out = 0xffffe940;
			return;
		}
// *below goes ab0 offset
		if (offset_in >= 0x00106270 && offset_in < 0x00107ff0)
		{
			displacement_out = 0xfffffa90;
			return;
		}
		if (offset_in >= 0x00159120 && offset_in < 0x0015aa51)
		{
//			displacement_out = 0x00158f60 - 0x00159120;
////			displacement_out = 0x0015923a - 0x001593fa;
////			displacement_out = 0x00159260 - 0x00159420;
////			displacement_out = 0x0015a890 - 0x0015aa50;
			displacement_out = 0xfffffe40;
			return;
		}
		if (offset_in == 0x00fe90a0 - 0x00400000)
		{
			displacement_out = 0x01026d28 - 0x00fe90a0;
			return;
		}
		break;
	default:
		Error ("Hooks_GetDisplacement_MainExe14NoCD () :  invalid version index %d", ver_main);
	}
	Error ("Hooks_GetDisplacement_MainExe14NoCD () :  version index %d, unincluded offset %08x", ver_main, offset_in);
}

void Hooks_GetDisplacement_DmfcDll14 (u32& displacement_out, u32 offset_in, int ver_dmfc)
{
	switch (ver_dmfc)
	{
	case VERSION_DMFC_14:
		displacement_out = 0;
		return;
	case VERSION_DMFC_15_BETA:
		if (offset_in == 0x0000b02e)
		{
			displacement_out = 0x0000b0be - offset_in;
			return;
		}
		if (offset_in == 0x0000b058)
		{
			displacement_out = 0x0000b0e8 - offset_in;
			return;
		}
// *not used
		if (offset_in == 0x0000bb4c)
		{
			displacement_out = 0x0000bbdc - offset_in;
			return;
		}
		if (offset_in == 0x0000dc73)
		{
			displacement_out = 0x0000dd03 - offset_in;
			return;
		}
// *not used
		if (offset_in == 0x0001a540)
		{
			displacement_out = 0x0001a5a0 - offset_in;
			return;
		}
// *not used
		if (offset_in == 0x00019600)
		{
			displacement_out = 0x00019660 - offset_in;
			return;
		}
		break;
	default:
		Error ("Hooks_GetDisplacement_DmfcDll14 () :  invalid version index %d", ver_dmfc);
	}
	Error ("Hooks_GetDisplacement_DmfcDll14 () :  version index %d, unincluded offset %08x", ver_dmfc, offset_in);
}

// get modified offset, and btw check, that a regions for all versions are defined
void Hooks_CorrectOffset_MainExe14NoCD (ulong& offset_out, u32 offset_in)
{
	u32 displacement_14;
	u32 displacement_14nocd;
	u32 displacement_15beta;
	u32 offset_14;
	u32 offset_14nocd;
	u32 offset_15beta;

// get for all versions, so that would be possible to test them all at once
	Hooks_GetDisplacement_MainExe14NoCD (displacement_14, offset_in, VERSION_MAIN_14_XX);
	Hooks_GetDisplacement_MainExe14NoCD (displacement_14nocd, offset_in, VERSION_MAIN_14_NOCD);
	Hooks_GetDisplacement_MainExe14NoCD (displacement_15beta, offset_in, VERSION_MAIN_15_BETA);
	offset_14 = offset_in + displacement_14;
	offset_14nocd = offset_in + displacement_14nocd;
	offset_15beta = offset_in + displacement_15beta;
	switch (version_main)
	{
	case VERSION_MAIN_14_XX:
		offset_out = offset_14;
		break;
	case VERSION_MAIN_14_NOCD:
		offset_out = offset_14nocd;
		break;
	case VERSION_MAIN_15_BETA:
		offset_out = offset_15beta;
		break;
	default:
		Error ("Hooks_CorrectOffset_MainExe14NoCD () :  invalid version index %d", version_main);
	}
}

void Hooks_CorrectOffset_DmfcDll14 (ulong& offset_out, u32 offset_in)
{
	u32 displacement_14;
	u32 displacement_15beta;
	u32 offset_14;
	u32 offset_15beta;

	Hooks_GetDisplacement_DmfcDll14 (displacement_14, offset_in, VERSION_DMFC_14);
	Hooks_GetDisplacement_DmfcDll14 (displacement_15beta, offset_in, VERSION_DMFC_15_BETA);
	offset_14 = offset_in + displacement_14;
	offset_15beta = offset_in + displacement_15beta;
	switch (version_dmfc)
	{
	case VERSION_DMFC_14:
		offset_out = offset_14;
		break;
	case VERSION_DMFC_15_BETA:
		offset_out = offset_15beta;
		break;
	default:
		Error ("Hooks_CorrectOffset_DmfcDll14 () :  invalid version index %d", version_dmfc);
	}
}

// give the address, where it would be in v1.4 NoCD, get the address where it is in the current version
// (versions v1.4 NoCD, v1.4 EU, v1.5 beta all have the same compiler and structure)
void Hooks_GetCorrectedAddress_MainExe14NoCD (u8*& address_out, ulong offset_in)
{
	ulong offset;

	Hooks_CorrectOffset_MainExe14NoCD (offset, offset_in);
	address_out = base_address_exe + offset;
}

void Hooks_GetCorrectedAddress_DmfcDll14 (u8*& address_out, ulong offset_in)
{
	ulong offset;

	Hooks_CorrectOffset_DmfcDll14 (offset, offset_in);
	address_out = base_address_dmfc + offset;
}

// packets relative stuff isn't catching player 0 actions
// must also be patched again, if we appear reloaded
// todo :  430084
//[2010-01-10 00:25:52] mzero says: Patches:: Apply() : byte at position 00430084 is ab, and is not 08
//[2010-01-10 00:26:12] mzero says: with ab written in lowercase
// stuff like 'if (*(u8*)address == 0x75)', means :  if patch is not already applied (by something else; or some other dll, or just statically)
void Hooks_PatchMainExe ()
{
	bool r;
	u8* address;
	u8* address_next;
	patch_t patch;

	if (version_main == VERSION_MAIN_UNDEFINED)
		Error ("Hooks_PatchMainExe () :  \"main.exe\" version is undefined");

	Hooks_GetCorrectedAddress_MainExe14NoCD (hook_return__no_packet_parsing, 0x00081cab);

#if HOOKS_TEST
// + test +
	Hooks_GetCorrectedAddress_MainExe14NoCD (address, 0x00159394);
	address_next = address + 5;
	patch.address = address;
	patch->len = 7;
	buffer[0] = 0xe9;
	*(u32*)&buffer[1] = (u32)Hooks_Test - address_next;
	buffer[5] = 0x90;
	buffer[6] = 0x90;
	address_next += 2;
	hook_return__test = (u32)address + 7;
	r = patches.Apply (&patch);
	if (! r)
		return;
// - test -
#endif

// + packets +
// replace 'and ebx, 0xff', 6 bytes
//	Hooks_GetCorrectedAddress_MainExe14NoCD (address, 0x00081352);
// replace 'mov edx, 1', 5 bytes
	Hooks_GetCorrectedAddress_MainExe14NoCD (address, 0x000812ca);
//	address_next = address + 6;
	address_next = address + 5;
	hook_return__packet_parse = address_next;
	patch.address = address;
	patch.len = 5;
	patch.vals[0] = 0xe9;
	*(u32*)&patch.vals[1] = (u8*)Hooks_PacketParse - address_next;
//	patch.vals[5] = 0x90;
	r = patches.Apply (&patch);
	if (! r)
		return;
// - packets -

// test
//	Error_NoExit ("-test-");

// + chat messages +
	Hooks_GetCorrectedAddress_MainExe14NoCD (address, 0x00081d30);
	r = patches.Apply (PATCH_TYPE__RAW_ADDRESS, address, (u8*)Hooks_Message_In, hook_return__chat_messages);
	if (! r)
		return;
	Hooks_GetCorrectedAddress_MainExe14NoCD (hook_00478b10, 0x00078b10);

// the call sequence :  430390, 430484, 4301f0 (called only from one place; its returned eax is not used)
// hook at the place, where "says" is applied
	Hooks_GetCorrectedAddress_MainExe14NoCD (address, 0x00030484);
	r = patches.Apply (PATCH_TYPE__CALL_E8, address, (u8*)Hooks_Message_Out, hook_call__chat_message_out);
	if (! r)
		return;

// duplicate :/
// the call sequence :  44347a, 452170 (called only from one place; its returned eax is not used), 45220a, 451e70, 451eda
// the buffer (6a63b0) remains unchanged up to its invalidation (45201c) (no anything is prepended)

// I perhaps prefer to hook this place (request to 'GetString ()'), instead of any caller of it (them even are three)
// not very good; at 451eda is only a request to 'GetString()', but not the actual packet send or display
	Hooks_GetCorrectedAddress_MainExe14NoCD (address, 0x00051eda);
// bad, no buffer address (6a63b0)
//	Hooks_GetCorrectedAddress_MainExe14NoCD (address, 0x0005220a);
// msg buffer
//	Hooks_GetCorrectedAddress_MainExe14NoCD (hook__chat_message_buffer, 0x002a63b0);
// GetString (117) request (nondedi) is at (three places) 442dee, 45220a, 45229f; 451e70, 451eda.
// not this one
//	Hooks_GetCorrectedAddress_MainExe14NoCD (address, 0x00042def);
// not tested
//	Hooks_GetCorrectedAddress_MainExe14NoCD (address, 0x0005220b);
// not tested
//	Hooks_GetCorrectedAddress_MainExe14NoCD (address, 0x0005229f);
	r = patches.Apply (PATCH_TYPE__CALL_E8, address, (u8*)Hooks_Message_Out_Nondedi, hook_call__chat_message_out_nondedi);
	if (! r)
		return;
// - chat messages -

// + F4 +
	Hooks_GetCorrectedAddress_MainExe14NoCD (address, 0x00081e64);
	r = patches.Apply (PATCH_TYPE__RAW_ADDRESS, address, (u8*)Hooks_F4, hook_return__f4);
	if (! r)
		return;
// - F4 -

// + GB release from inventory +
	Hooks_GetCorrectedAddress_MainExe14NoCD (address, 0x00081d98);
	r = patches.Apply (PATCH_TYPE__RAW_ADDRESS, address, (u8*)Hooks_GB_Release, hook_return__gb_release);
	if (! r)
		return;
// - GB release from inventory -

// + respawn bug +
	Hooks_GetCorrectedAddress_MainExe14NoCD (address, 0x000889d8);
	hook_pointer__respawn_counter = *(u8**)address;
	Hooks_GetCorrectedAddress_MainExe14NoCD (address, 0x00088a54);
	r = patches.Apply (PATCH_TYPE__LONG_JE, address, (u8*)Hooks_RespawnPowerup, hook_return__respawn_powerup);
	if (! r)
		return;
// - respawn bug -

// test with packets
// *this stuff may be unstable; disable it if suspect that it crashes
#if 0
// replace 'add esp,274', 6 bytes
	Hooks_GetCorrectedAddress_MainExe14NoCD (address, 0x001593fa);
	Hooks_GetCorrectedAddress_MainExe14NoCD (hook_call__packet_send_initiation, 0x00159420);
	Hooks_GetCorrectedAddress_MainExe14NoCD (hook_call__poll_sockets, 0x0015aa50);
	address_next = address + 5;
	hook_return__packet_put = address_next + 1;
	patch.address = address;
	patch.len = 6;
	patch.vals[0] = 0xe9;
	*(u32*)&patch.vals[1] = (u8*)Hooks_PacketPut - address_next;
	patch.vals[5] = 0x90;
	r = patches.Apply (&patch);
	if (! r)
		return;
#endif

// test with packets 2
// not all traffic of the output goes through 559120 subr
// this doesn't help much; yes, it lets to play Alien Territory and Windmine 6, but it also adds some bugs, like when a player cannot connect soon after level change, and F7 sometimes doesn't work (for pnum #2 ?)
#if 1
// replace 'mov eax,[0x01008754]', 5 bytes
	Hooks_GetCorrectedAddress_MainExe14NoCD (address, 0x00159120);
	Hooks_GetCorrectedAddress_MainExe14NoCD (hooks__socket, 0x00fe90a0 - 0x00400000);
	if (*address == 0xa1)
	{
		address_next = address + 5;
		hook_return__packet_put = address_next + 0;
		patch.address = address;
		patch.len = 5;
		patch.vals[0] = 0xe9;
		*(u32*)&patch.vals[1] = (u8*)Hooks_PacketPut - address_next;
//		patch.vals[5] = 0x90;
		r = patches.Apply (&patch);
		if (! r)
			return;
	}
#endif

// 473af0 - limit player's PPS by server's one (called at player join).
// *not called on the client's side
// *note, that this limiting thing is done properly, but I am disabling it, because players often have just 12 pps limit, and such a speed is inacceptably outdated at these days
// make server to output packets with a server's pps instead of player's one
// *no, lets better not do it now, but change the pps later, when the player will be joined
#if 0
// disable 'jbe'
	Hooks_GetCorrectedAddress_MainExe14NoCD (address, 0x00073afe);
	if (*address == 0x06)
	{
		patch.address = address;
		patch.len = 1;
		patch.vals[0] = 0x00;
		r = patches.Apply (&patch);
		if (! r)
			return;
	}
#endif

// no, it is server's pps, and the client doesn't send that
//	*(u8*)0x006f483c = 25;
// 55a6d0 - get our pps as client :  5, 6, 7, 8, 9, 12 (12 is diminished to 7 in some case)
// 4820d2 - not less than 8 (but it doesn't go through here)
// whoa !  I am making a complete crap !  no of course :) it will not change the PPS we send
// 4820d5 :  7d 05, b8 08 00 00 00 -> 7d 00, b8 1f 00 00 00
#if 1
	Hooks_GetCorrectedAddress_MainExe14NoCD (address, 0x0015a7c7);
	if (*address == 0x0c)
	{
		patch.address = address;
		patch.len = 1;
// 31 pps
		patch.vals[0] = 0x1f;
// 2 pps
//		patch.vals[0] = 0x02;
		r = patches.Apply (&patch);
		if (! r)
			return;
	}
#endif

	DLLAddHUDMessage ("\"main.exe\" %s patched ok", version_main_name);
	return;
}

// "dmfc.dll" reloaded, so needs to be patched every time
void Hooks_Patch_DMFC_DLL ()
{
	bool r;
	u8* address;
	patch_t patch;

#if 0
	switch (version_dmfc)
	{
	case VERSION_DMFC_14:
	case VERSION_DMFC_15_BETA:
		break;
	default:
//// not patched ? - doubtly anyone cares about pps limit
// not patched ? - doubtly anyone cares much about the minor stuff which is being patched here
		return;
	}
#endif

// pps byte at 0x6f483c is changed by :  48a2ba to 0a, 4300f7 to "PPS=" value in dedi .cfg file, +0dc8b to the SetPPS in "autoexec.dmfc"
// 42fef3 - (5a1f2c) 2 pps min
// 42fefd - (5a1f30) 20 pps max
//// and both of these happens before our 'DLLGameInit()' is being called (thus "dmfc.dll" isn't patched yet; though "co-op.dll" is already loaded)
//// anyway, lets ignore this, and set up PPS now
// *"autoexec.dmfc" is being read in 'DMFCBase->GameInit()', which is being called from our 'CoopGameInit()', so we must be called before it
// *this stuff is unnecessary, since we do PPS management ourselves, in this mod, at the time of client connect
#if 1
// + DMFC 20 PPS limit +
// pattern :  83 f8 14, 7e
	Hooks_GetCorrectedAddress_DmfcDll14 (address, 0x0000dc73);
	if (*address == 0x14)
	{
// max for signed single byte comparison
		*(u32*)patch.vals = 0x0000007f;
		patch.address = address;
		patch.len = 1;
		r = patches.Apply (&patch);
		if (! r)
			return;
		address += 4;
		patch.address = address;
		patch.len = 4;
		r = patches.Apply (&patch);
		if (! r)
			return;
	}
// - DMFC 20 PPS limit -
#endif

#if 0
// sickness, which has began from Descent 1
// + "Waiting For Players" +
// 4473f0 (index in 0065fda0 is 9) (447714) -> 43d270 -> DLLGameCall (EVT_CLIENT_SHOWUI 619) -> +1bca0 -> +1bae0 -> +bb40 -> +1a540
// DMFCBase->OnClientPlayerEntersGame(0) -> +b058 -> +6693
	Hooks_GetCorrectedAddress_DmfcDll14 (address, 0x0000bb4c);
	if (*(u8*)address == 0x75)
	{
		patch.address = address;
		patch.len = 1;
		patch.vals[0] = 0xeb;
		r = patches.Apply (&patch);
		if (! r)
			return;
	}
// - "Waiting For Players" -

// and this goes for a team games
// + "Team Setup" +
// 4473f0 (index in 0065fda0 is 9) (447714) -> 43d270 -> DLLGameCall (EVT_CLIENT_SHOWUI 619) -> +1bca0 -> +1bae0 -> +bb40 -> +19600
// called initially by :  +1671e -> +1b21e -> +16641 -> +1b57e -> +b02e -> +6693
// and by a request by :  +12f6f -> +6693
	Hooks_GetCorrectedAddress_DmfcDll14 (address, 0x00019600);
	if (*(u8*)address == 0x81)
	{
		patch.address = address;
		patch.vals[0] = 0xc2;
		patch.vals[1] = 0x08;
		patch.vals[2] = 0x00;
		patch.len = 3;
		r = patches.Apply (&patch);
		if (! r)
			return;
	}
// - "Team Setup" -
#endif

// sickness, which has began from Descent 1
	patch.address = 0;
	patch.len = 6;
// add esp,8
	patch.vals[0] = 0x83;
	patch.vals[1] = 0xc4;
	patch.vals[2] = 0x08;
	patch.vals[3] = 0x90;
	patch.vals[4] = 0x90;
	patch.vals[5] = 0x90;

// + "Waiting For Players" +
// 4473f0 (index in 0065fda0 is 9) (447714) -> 43d270 -> DLLGameCall (EVT_CLIENT_SHOWUI 619) -> +1bca0 -> +1bae0 -> +bb40 -> +1a540
// DMFCBase->OnClientPlayerEntersGame(0) -> +b058 -> +6693
	Hooks_GetCorrectedAddress_DmfcDll14 (address, 0x0000b02e);
	if (*(u8*)address == 0xff)
	{
		patch.address = address;
		r = patches.Apply (&patch);
		if (! r)
			return;
	}
// - "Waiting For Players" -

// and this goes for a team games
// + "Team Setup" +
// 4473f0 (index in 0065fda0 is 9) (447714) -> 43d270 -> DLLGameCall (EVT_CLIENT_SHOWUI 619) -> +1bca0 -> +1bae0 -> +bb40 -> +19600
// called initially by :  +1671e -> +1b21e -> +16641 -> +1b57e -> +b02e -> +6693
// and by a request by :  +12f6f -> +6693
	Hooks_GetCorrectedAddress_DmfcDll14 (address, 0x0000b058);
	if (*(u8*)address == 0xff)
	{
		patch.address = address;
		r = patches.Apply (&patch);
		if (! r)
			return;
	}
// - "Team Setup" -

	DLLAddHUDMessage ("\"dmfc.dll\" %s patched ok", version_dmfc_name);
}

// *nondedi hud perhaps isn't initialized yet
void Hooks_Init ()
{
	DWORD D_r;
	static char file_name[1024] = {0};
	void* module_address;
//	DWORD id;

// the dll reloads, and this will not protect against double-patching after reload
	if (hooks_initialized)
		return;
// say right now that it is initialized, to avoid mess, if something will be already partially initialized
	hooks_initialized = 1;
	module_address = (void*)0x00400000;
	D_r = GetModuleFileName ((HMODULE)module_address, file_name, sizeof(file_name));
	if (! D_r)
		Error ("Hooks_Init () :  couldn't get module file name at %08x", module_address);
	base_address_exe = (u8*)GetModuleHandle (file_name);
	if (! base_address_exe)
		Error ("Hooks_Init () :  couldn't get base address of module \"%s\"", file_name);

// not flexible atm; exe loads it not on it's default address - 0x10000000, but on 0x030c0000
	strcpy (file_name, "dmfc.dll");
	base_address_dmfc = (u8*)GetModuleHandle (file_name);
	if (! base_address_dmfc)
		Error ("Hooks_Init () :  couldn't get base address of module \"%s\"", file_name);

	Hooks_DefineVersions ();

// and the content above, perhaps should also be in the "patches.cpp"
#if 0
// *seems this function cannot fail
	id = GetCurrentProcessId ();
	process_handle = OpenProcess (PROCESS_ALL_ACCESS, 0, id);
	if (! process_handle)
	{
		Error ("Hooks_Init () :  'OpenProcess ()' fail");
		return;
	}
#endif

#if 1
	Hooks_PatchMainExeOnce ();
	Hooks_PatchMainExe ();
	Hooks_Patch_DMFC_DLL ();
#endif
}

void Hooks_Quit ()
{
	if (! hooks_initialized)
		return;
//	Hooks_UnPatchMainExe ();
	hooks_initialized = 0;
}

void Hooks_Frame ()
{
	if (hooks_indicate__socket_traffic)
	{
		static float gametime_last = 0;
		static float gametime_next = 0;
		float gametime;

		gametime = DMFCBase->GetGametime ();
// if it is time to print, or if something other has happened, like level change or gametime rollback
//		if (gametime >= gametime_last + 1.0)
//		if (gametime >= gametime_next)
		if (gametime >= gametime_next || gametime < gametime_last)
		{
			gametime_last = gametime;
			gametime_next = gametime + 1.0;
//			CoopMod_SendMessage (0, "socket output traffic :  %06u bytes per second", hooks__socket_traffic_output);
			CoopMod_SendMessage (0, "socket traffic :  output %06u, input %06u bytes per second", hooks__socket_traffic_output, hooks__socket_traffic_input);
			hooks__socket_traffic_output = 0;
			hooks__socket_traffic_input = 0;
		}
	}

#if HOOKS_TEST
	if (netbuf_level_min != -1)
		CoopMod_SendMessage (-1, "netbuf level min %04x, max %04x", netbuf_level_min, netbuf_level_max);
	netbuf_level_min = -1;
	netbuf_level_max = 0;
#endif
//	if (hooks_indicator__f4)
	if (hooks_indicator__f4 >= 0)
	{
//		DLLAddHUDMessage ("blocks :  F4");
		CoopMod_SendMessage (-1, "player #%d, F4 is blocked", hooks_indicator__f4);
//		hooks_indicator__f4 = 0;
		hooks_indicator__f4 = -1;
// avoid messages flood
		return;
	}
//	if (hooks_indicator__gb_release)
	if (hooks_indicator__gb_release >= 0)
	{
//		DLLAddHUDMessage ("blocks :  direct release of Guidebot by player #%d", hooks_indicator__gb_release);
//		CoopMod_SendMessage (-1, "player #%d, function currently is disabled", hooks_indicator__gb_release);
		CoopMod_SendMessage (-1, "player #%d, GuideBot release function is blocked", hooks_indicator__gb_release);
//		hooks_indicator__gb_release = 0;
		hooks_indicator__gb_release = -1;
		return;
	}
}


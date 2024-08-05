#pragma once

#include "types.h"


enum
{
	VERSION_MAIN_UNDEFINED,
	VERSION_MAIN_14_XX,
	VERSION_MAIN_14_NOCD,
	VERSION_MAIN_15_BETA,
	VERSIONS_MAIN_NUM
};

enum
{
	VERSION_DMFC_UNDEFINED,
// exist only one of v 1.4
//	VERSION_DMFC_14_XX,
//	VERSION_DMFC_14_NOCD,
	VERSION_DMFC_14,
	VERSION_DMFC_15_BETA,
	VERSIONS_DMFC_NUM
};


extern int version_main;
extern int version_dmfc;
extern char* version_main_name;
extern char* version_dmfc_name;
extern u32 hook_store_plix;
// + settings +
extern bool hook_block_guidebot_packet;
extern bool hooks_indicate__socket_traffic;
//extern bool hooks_indicate__socket_traffic_output;
//extern bool hooks_indicate__socket_traffic_input;
extern bool hooks_send_packets_5e;
// - settings -


void Hooks_Init ();
void Hooks_Quit ();
void Hooks_Frame ();


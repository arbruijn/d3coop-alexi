/*
=========================================================
THE COMPUTER CODE CONTAINED HEREIN IS THE SOLE 
PROPERTY OF OUTRAGE ENTERTAINMENT, INC. 
('OUTRAGE').  OUTRAGE, IN DISTRIBUTING THE CODE TO
END-USERS, AND SUBJECT TO ALL OF THE TERMS AND 
CONDITIONS HEREIN, GRANTS A ROYALTY-FREE, 
PERPETUAL LICENSE TO SUCH END-USERS FOR USE BY 
SUCH END-USERS IN USING, DISPLAYING,  AND 
CREATING DERIVATIVE WORKS THEREOF, SO LONG AS
SUCH USE, DISPLAY OR CREATION IS FOR NON-
COMMERCIAL, ROYALTY OR REVENUE FREE PURPOSES. 
IN NO EVENT SHALL THE END-USER USE THE 
COMPUTER CODE CONTAINED HEREIN FOR REVENUE-
BEARING PURPOSES.  THE END-USER UNDERSTANDS
AND AGREES TO THE TERMS HEREIN AND ACCEPTS THE 
SAME BY USE OF THIS FILE.
COPYRIGHT 1999 OUTRAGE ENTERTAINMENT, INC.  ALL 
RIGHTS RESERVED.
=========================================================
*/

/*
* $Logfile: /DescentIII/Main/coop/coop.h $
* $Revision: 32 $
* $Date: 5/12/99 11:28a $
* $Author: Jeff $
*
* <insert description of file here>
*
* $Log: /DescentIII/Main/coop/coop.h $
 * 
 * 32    5/12/99 11:28a Jeff
 * added sourcesafe comment block
*
* $NoKeywords: $
*/



#ifndef __DMFC_APP_H_
#define __DMFC_APP_H_

#include "idmfc.h"
#include "osiris_share.h"
#include "d3events.h"


//Setup and processing functions
void CoopGameInit(int teams);
void CoopGameClose(void);

void OnKeypress(int key);
void OnHUDInterval(void);
void OnInterval(void);
void OnClientPlayerKilled(object *killer_obj,int victim_pnum);
void OnClientPlayerEntersGame(int player_num);
void OnClientLevelStart(void);
void OnClientLevelEnd(void);
void OnServerGameCreated(void);
void OnPLRInterval(void);
void OnPLRInit(void);
void OnSaveStatsToFile(void);
void OnLevelEndSaveStatsToFile(void);
void OnDisconnectSaveStatsToFile(void);
void OnPrintScores(int level);
//void OnControlMessage(ubyte msg,int from_pnum);
//void OnSpecialPacket(void);
extern IDMFC *DMFCBase;



/*************************************************************************************************
 *The following functions and declaration are needed to connect the DLL to the game.  These must *
 *stay here and must call the functions that are in them.  You can not delete from here, but you *
 *can add to it no problem																								 *
 *************************************************************************************************
*/
// These next two function prototypes MUST appear in the extern "C" block if called
// from a CPP file.
#ifdef __cplusplus
extern "C"
{
#endif
	DLLEXPORT void DLLFUNCCALL DLLGameInit (int *api_func,ubyte *all_ok,int num_teams_to_use);
	DLLEXPORT void DLLFUNCCALL DLLGameCall (int eventnum,dllinfo *data);
	DLLEXPORT void DLLFUNCCALL DLLGameClose ();
	DLLEXPORT void DLLFUNCCALL DLLGetGameInfo (tDLLOptions *options);
	DLLEXPORT int DLLFUNCCALL GetGOScriptID(char *name,ubyte isdoor);
	DLLEXPORT void DLLFUNCCALLPTR CreateInstance(int id);
	DLLEXPORT void DLLFUNCCALL DestroyInstance(int id,void *ptr);
	DLLEXPORT short DLLFUNCCALL CallInstanceEvent(int id,void *ptr,int event,tOSIRISEventInfo *data);
	DLLEXPORT int DLLFUNCCALL SaveRestoreState( void *file_ptr, ubyte saving_state );
#ifdef __cplusplus
}
#endif


#endif

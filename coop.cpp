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
* $Logfile: /DescentIII/Main/coop/coop.cpp $
* $Revision: 80 $
* $Date: 5/23/99 3:04a $
//* $Author: Jason $
// Alex :  who could know the author ...
*
* <insert description of file here>
*
* $Log: /DescentIII/Main/coop/coop.cpp $
 * 
 * 80    5/23/99 3:04a Jason
 * fixed bug with player rankings not being updated correctly
 * 
 * 79    5/12/99 11:28a Jeff
 * added sourcesafe comment block
*
* $NoKeywords: $
*/



#include <string.h>
#include <stdio.h>
#include <stdlib.h>
//#include "idmfc.h"		// Needed for DMFC
#include "coop.h"	// Some prototypes
#include "coopstr.h"	// String table header
#include "coop_mod.h"
#include "coop_mod_cmd.h"
//#define __OSIRIS_IMPORT_H_
#include "../osiris/osiris_import.h"
#include "log_file.h"
#include "rooms.h"
#include "hooks.h"
#include "patches.h"
#include "screen.h"
#include "error.h"
#include "control.h"
#include "drop.h"
#include "exceptions.h"


#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

// Co-op HUD Modes
#define AHD_NONE		0	//No HUD
#define AHD_SCORE		1	//Show player score
#define AHD_EFFICIENCY	2	//Show player efficiency

// Co-op HUD Color Modes
#define ACM_PLAYERCOLOR	0	//Use the color of their ship
#define ACM_NORMAL		1	//Use green

/*
Our DMFC Interface.  All Interaction with DMFC is via this
*/
IDMFC *DMFCBase = NULL;

/*
Our DMFC Stats Interface (F7 Stats screen manager).  All interaction
with the stats is via this
*/
IDmfcStats *dstat = NULL;

/*
SortedPlayers - An array of indicies into the Player records.  This
gets sorted every frame and is used throughout the mod (for instance
when displaying the scores on the HUD.  SortedPlayers[0] = the player record
value of the player in first place.
*/
int SortedPlayers[MAX_PLAYER_RECORDS];	//sorted player nums

/*
DisplayScoreScreen - true if the F7 Stats screen is up
*/
bool DisplayScoreScreen;

/*
Highlight_bmp - Bitmap handle for the highlight bar used on the HUD when
displaying the player names, to highlight your name
*/
int Highlight_bmp = -1;

/*
coop_hud_display - What should we display on the HUD? Nothing? Player
scores? Player efficiencies?  This is changed in the F6 On-Screen Menu
*/
ubyte coop_hud_display = AHD_SCORE;

/*
HUD_color_model - What color mode is the HUD in? Should the players be displayed
using the color of their ship?  Or just green.  This is changed in the F6 
On-Screen menu
*/
ubyte HUD_color_model = ACM_PLAYERCOLOR; 

/*
display_my_welcom - Takes the value of false until we display the "Welcome" message
when you join/start an Co-op game.  HUD messages can't be displayed until the
first frame of the game has displayed, so we wait until then to display the welcome
message.
*/
bool display_my_welcome = false;

// Alex added :  was 6
int hud_names_max = 9;


// HUD callback function when it is time to render our HUD
void DisplayHUDScores(struct tHUDItem *hitem);

// Displays a welcome message for a joining client
void DisplayWelcomeMessage(int player_num);

// Called with a filename to save the current state of the game
// stats to file.
void SaveStatsToFile(char *filename);

// Switches the HUD Color mode to a new value
void SwitchHUDColor(int i);
// SwitchCoopScores
//
//	Call this function to switch the HUD display mode for what
//  we display on the HUD (Player Scores/Player Efficiency/Nothing)
void SwitchCoopScores(int i);

///////////////////////////////////////////////
//localization info (string table functions and variables)
//
// String tables are only needed if you plan on making a multi-lingual mod.
char **StringTable;
int StringTableSize = 0;
char *_ErrorString = "Missing String";
char *GetString(int d){if( (d<0) || (d>=StringTableSize) ) return _ErrorString; else return StringTable[d];}
///////////////////////////////////////////////

#ifdef MACINTOSH
#pragma export on
#endif


//	DLLGetGameInfo
//
//	This function gets called by the game when it wants to learn some info about 
//	the multiplayer mod.
void DLLFUNCCALL DLLGetGameInfo (tDLLOptions *options)
{
	// Specify what values we will be filling in
	options->flags		= DOF_MAXTEAMS;	//only max teams
	options->max_teams	= 1;			//not a team game, so it's a 1

	// Mandatory values
	strcpy(options->game_name,"Co-op");	// The name of our game
	strcpy(options->requirements,"");		// the MSN file KEYWORD requirements
											// there are no requirements for
											// Co-op, all levels are ok
}

//	DLLGameInit
//
//	Call by the game when the D3M is loaded.  Initialize everything
//	required for the mod here.
//
//	Parms:
//		api_func : List of D3 exported functions and variable pointers
//					You don't have to do anything with this but pass it
//					to DMFC::LoadFunctions();
//		all_ok	: When this function returns, Descent 3 checks the bool this
//					pointer points to.  If it's true (1) then the game continues
//					to load.  If it's false (0) then Descent 3 refuses to load
//					the mod.
//		num_teams_to_use : In the case of team games (2, 3 or 4 teams), this is 
//					the value the user selected for the number of teams.
void DLLFUNCCALL DLLGameInit (int *api_func,ubyte *all_ok,int num_teams_to_use)
{
//		Screen_Restore ();
//		__asm int 3

// Alex :  this perhaps is the first what we would like to have - own exceptions handler, which would let MSVC to catch an exception, instead of just the engine would throw its helpless mbox
#ifdef WIN32
	Exceptions_Init ();
#endif

	// Initialize all_ok to true
	*all_ok = 1;

	// This MUST BE the absolute first thing done.  Create the DMFC interface.
	// CreateDMFC() returns a pointer to an instance of a DMFC class.  If it
	// returns NULL there was some problem creating the DMFC instance and you
	// must bail right away!
	DMFCBase = CreateDMFC();
	if(!DMFCBase)
	{
		//no all is not ok!
		*all_ok = 0;
		return;
	}

	// Since we want an F7 stats screen, we will now try to create an instance
	// of DMFCStats class.  Calling CreateDmfcStats() is just like CreateDMFC()
	// but it creates an instance of DMFCStats.
	dstat = CreateDmfcStats();
	if(!dstat)
	{
		//no all is not ok!
		*all_ok = 0;
		return;
	}

	// We can't call any functions exported from Descent 3 until we initialize
	// them.  Calling DMFC::LoadFunctions() will initialize them.
	DMFCBase->LoadFunctions(api_func);

	// In order to capture events from the game, we must tell DMFC what
	// events we want to get notified about.  The following list of functions
	// call the appropriate DMFC member function to register a callback for
	// an event.  When a callback is registered for an event, as soon as the event
	// happens, the callback you give will be called.  It is your responsibility
	// to call the default event handler of the specific event, DMFC performs
	// many behind-the-scenes work in these default handlers.

	// register for the keypress event (the user pressed a key)
	DMFCBase->Set_OnKeypress(OnKeypress);

	// register for the HUD interval event (gets called once per frame when it
	// is time to render the HUD)
	DMFCBase->Set_OnHUDInterval(OnHUDInterval);

	// register for the interval event (gets called once per frame)
	DMFCBase->Set_OnInterval(OnInterval);

	// register for the client version of the player killed event (gets called
	// on all clients when a player dies)
	DMFCBase->Set_OnClientPlayerKilled(OnClientPlayerKilled);

	// register for the client version of the player joined game event (gets 
	// called on all clients when a player enters the game)
	DMFCBase->Set_OnClientPlayerEntersGame(OnClientPlayerEntersGame);

	// register for the client version of the level start event (gets called
	// on all clients when they are about to start/enter a level)
	DMFCBase->Set_OnClientLevelStart(OnClientLevelStart);

	// register for the client version of the level end event (gets called on
	// all clients when the level ends)
	DMFCBase->Set_OnClientLevelEnd(OnClientLevelEnd);

	// register for the server event, that a game has been created (gets called
	// only once, when the mod is loaded)
	DMFCBase->Set_OnServerGameCreated(OnServerGameCreated);

	// register for the Post Level Results interval event (gets called every frame
	// when it's time to render a Post Level Results screen)
	DMFCBase->Set_OnPLRInterval(OnPLRInterval);

	// register for the Post Level Results init event (gets called before the first
	// frame of the Post Level Results, per level, so we can setup some values).
	DMFCBase->Set_OnPLRInit(OnPLRInit);

	// register for the Save Stats event (gets called when the client wants to save
	// the game stats now!)
	DMFCBase->Set_OnSaveStatsToFile(OnSaveStatsToFile);

	// register for the Save End of level stats event (gets called when the level 
	// ends if the client wants the stats saved at the end of each level.)
	DMFCBase->Set_OnLevelEndSaveStatsToFile(OnLevelEndSaveStatsToFile);

	// register for the Save on Disconnect event (gets called when you get disconnected
	// from the server for some reason and the client wants stats saved)
	DMFCBase->Set_OnDisconnectSaveStatsToFile(OnDisconnectSaveStatsToFile);

	// register for the print scores event (gets called when a dedicated server
	// does a $scores request)
	DMFCBase->Set_OnPrintScores(OnPrintScores);

	CoopMod_Init ();

// Since our version of co-op is localized (same code works for multiple languages) we need to load our string table now.
// The function knows what language to load in automatically.
// "coop.str" is located inside "coop.d3m" hog file of Descent 3.
// It needs to be in a place where Descent 3 will be able to find it
// (inside .hog file or in the game root directory or in the game working directory,
// but not in Descent3\netgames directory (if not packed into .hog)).
// Otherwise you will get an error message "error initializing game module".

	DLLCreateStringTable("coop.str",&StringTable,&StringTableSize);
	mprintf((0,"%d strings loaded from string table\n",StringTableSize));
	if(!StringTableSize){
		// we were unable to load the string table, bail out
		*all_ok = 0;
		return;
	}

// Linux can't support that with using of Windows functions :)
#ifdef WIN32
//// init patches' instance before hooks
//// *is requested from here, meaning, that it does not belong to Hooks
//	patches.Init ();
// inject hooks before calling 'DMFCBase->GameInit()', which reads "autoexec.dmfc"
	Hooks_Init ();
#endif

	// Initialize Co-op!
	CoopGameInit(1);

	// Allocate a bitmap for the highlight bar used when displaying the
	// scores on the HUD.  We'll allocate a 32x32 (smallest square bitmap
	// allowed by D3).  You can only draw square bitmaps.
	Highlight_bmp = DLLbm_AllocBitmap(32,32,0);
	if(Highlight_bmp>BAD_BITMAP_HANDLE){
		// get a pointer to the bitmap data so we can
		// initialize it to grey/
		ushort *data = DLLbm_data(Highlight_bmp,0);
		if(!data){
			//bail on out of here
			*all_ok = 0;
			return;
		}

		// go through the bitmap (each pixel is 16 bits) and
		// set the RGB values to 50,50,50.  We also want to set the
		// opaque flag for each pixel, else the pixel will be transparent!
		for(int x=0;x<32*32;x++){
// Alex :  this possibly crashes (debugger shows me exception on this call; though it can't be guilty just a call, isn't it ?)
			data[x] = GR_RGB16(50,50,50)|OPAQUE_FLAG;
		}
	}

	// Register our HUD display callback function with Descent 3...it will
	//be called at the appropriate time to display HUD items.
	DMFCBase->AddHUDItemCallback(HI_TEXT,DisplayHUDScores);

	// Initialize this to false, we're not displaying the stats screen
	DisplayScoreScreen = false;
}

//	DLLGameClose
//
//	Called when the DLL is about to final shutdown
void DLLFUNCCALL DLLGameClose(void)
{
	// if our HUD highlight bar's bitmap was a valid bitmap
	// we must free it, we don't want a memory leak.
	if(Highlight_bmp>BAD_BITMAP_HANDLE)
		DLLbm_FreeBitmap(Highlight_bmp);

	// Since we had a string table (coop.str) we must
	// remember to destroy it to free up all memory it used.
	DLLDestroyStringTable(StringTable,StringTableSize);

	// destroy the instance of our DmfcStats, we want our memory back
	// it's not going to be used anymore.
	if(dstat)
	{
		dstat->DestroyPointer();
		dstat = NULL;
	}	

	// And finally, the absolute last thing before we leave, destroy
	// DMFC.
	if(DMFCBase)
	{
		// Call our Co-op close function for any last minute final
		// shutdown
		CoopGameClose();

		// We need to close the game in DMFC before destroying it
		DMFCBase->GameClose();
		DMFCBase->DestroyPointer();
		DMFCBase = NULL;
	}

	CoopMod_Close ();
#ifdef WIN32
	Hooks_Quit ();
//	patches.Quit ();
	patches.UnpatchAll ();
// Alex :  and this is here for symmetry
	Exceptions_Quit ();
#endif
}

// DetermineScore
//
//	Callback function for the DmfcStats manager.  It gets called for
//	custom text items in the stats.  
//
//	Parms:
//		precord_num : Player record index of the player we are referring to.
//		column_num	: Which column of the stats manager is this call about
//						This is all we have to distinguish multiple custom columns
//		buffer		: The buffer we should fill in
//		buffer_size	: Size of the buffer passed in, DON'T OVERWRITE 
void DetermineScore(int precord_num,int column_num,char *buffer,int buffer_size)
{
	// Get a pointer to the player record data for the given player record
	player_record *pr = DMFCBase->GetPlayerRecord(precord_num);

	// If it was an invalid player record, or the player was never in the game
	// then short-circuit
	if(!pr || pr->state==STATE_EMPTY){
		buffer[0] = '\0';
		return;
	}

	// Fill in the score (kills - suicides) for this level
//	sprintf(buffer,"%d",pr->dstats.kills[DSTAT_LEVEL]-pr->dstats.suicides[DSTAT_LEVEL]);
//	sprintf(buffer,"%d",0 - pr->dstats.kills[DSTAT_LEVEL] * 25);
	sprintf(buffer,"%d",0 - (pr->dstats.kills[DSTAT_LEVEL] + pr->dstats.suicides[DSTAT_LEVEL]));
}

//	CoopGameClose
//	
//	Calls for any final shutdown
void CoopGameClose(void)
{
}

// CoopGameInit
//
//	Initializes Co-op so it is ready to be played
void CoopGameInit(int teams)
{
	// First we are going to create the On-Screen menu submenus for Coop.
	// This must be done before the call to DMFC::GameInit() if we want the
	// Coop sub-menu to be at the top.

	IMenuItem *lev1,*lev2;

	// Using CreateMenuItemWArgs we can create a sub menu instance
	// First we'll create the base sub-directory with the "Coop" sub title
	lev1 = CreateMenuItemWArgs("Coop",MIT_NORMAL,0,NULL);

	// Now we'll create the HUD style sub menu, since it is a state item, we
	// need to pass a callback, which will get called if the state changes
	lev2 = CreateMenuItemWArgs(TXT_MNU_HUDSTYLE,MIT_STATE,0,SwitchCoopScores);
	// Set the text of all the states
	lev2->SetStateItemList(3,TXT_NONE,TXT_SCOREHD,TXT_EFFICIENCY);
	// Set initial value
	lev2->SetState(1);
	// Now add this submenu to the base-subdirectory
	lev1->AddSubMenu(lev2);

	// Next create the sub directory for HUD color state.  Again, this is a
	// state item, so we have to pass a callback also.
	lev2 = CreateMenuItemWArgs(TXT_MNU_HUDCOLOR,MIT_STATE,0,SwitchHUDColor);
	// Set the text of all the states
	lev2->SetStateItemList(2,TXT_PLAYERCOLORS,TXT_NORMAL);
	// Set the initial state
	lev2->SetState(HUD_color_model);
	// Now add this submenu to the base-subdirectory
	lev1->AddSubMenu(lev2);

	// Grab a pointer to the On-Screen menu root
	lev2 = DMFCBase->GetOnScreenMenu();
	// Add the sub-menu tree created above (which was all branched from the
	// base sub-directory "Coop" to the root.
	lev2->AddSubMenu(lev1);

	// Now that we setup our On-Screen menu, we can initialize DMFC,
	// all we have to do is pass the initial number of teams.
	DMFCBase->GameInit(teams);

	// Initialize the Dmfc stats Manager
	tDmfcStatsInit tsi;
	tDmfcStatsColumnInfo pl_col[6];
	char gname[20];
//	strcpy (gname, "Co-op Descent 3");
	strcpy (gname, "Co-op Descent 3 mod");
//	strcpy (gname, GetString (1));

	// we want to show the Pilot Picture/Ship Logo of the pilot if available
	// we also want to show the observer mode icon, if the player is observing
// all flags
//	tsi.flags = DSIF_SEPERATE_BY_TEAM | DSIF_SHOW_PIC | DSIF_ONLY_X_PLAYERS_SHOWN | DSIF_SHOW_OBSERVERICON | DSIF_NOLASTKILLER | DSIF_NOLASTVICTIM | DSIF_NODETAILEDINFO | DSIF_SHOWPLAYERVIEW;
// Anarchy flags
//	tsi.flags = DSIF_SHOW_PIC|DSIF_SHOW_OBSERVERICON;
	tsi.flags = DSIF_SHOW_PIC | DSIF_SHOW_OBSERVERICON | DSIF_NOLASTKILLER | DSIF_NOLASTVICTIM | DSIF_NODETAILEDINFO | DSIF_SHOWPLAYERVIEW;
	tsi.cColumnCountDetailed = 0;	// 0 columns in the detailed list (specific info about the highlighted player)
	tsi.cColumnCountPlayerList = 6;	// 6 columns in the player list
	tsi.clbDetailedColumn = NULL;	// No custom items in the detailed list, no 
									//callback needed
	tsi.clbDetailedColumnBMP = NULL;// No custom bitmap items in the detailed list, no callback needed
	tsi.clbPlayerColumn = DetermineScore;	// we do have a custom text column, so
											// set it's callback
	tsi.clbPlayerColumnBMP = NULL;		// no custom bitamp items in the detailed 
										//list
	tsi.DetailedColumns = NULL;		// pointer to an array of tDmfcStatsColumnInfo
									// to describe the detail columns (there are none,
									// so this is NULL)
	tsi.GameName = gname;			// The title for the Stats screen
	tsi.MaxNumberDisplayed = NULL;	// This can be set to a pointer to an int, that is the maximum number of players to display in the stats screen
									// by default it is all of them.  Make sure you use a global variable or a static, as this pointer must always
									// be valid.  Since we want all of them, just set this to NULL.
	tsi.PlayerListColumns = pl_col;	// pointer to an array of tDmfcStatsColumnInfo
									// to describe the player columns (this array 
									// is filled out below)
	tsi.SortedPlayerRecords = SortedPlayers;	// This is a pointer to an array of ints which is of
												// size MAX_PLAYER_RECORDS.  This array is a sorted list
												// of the player records.  The stats manager uses this
												// so it knows the order that it should display the players.

	// Now setup the Player List column items

	// The first column is the name of the player, we can use the predefined
	// column type.
	pl_col[0].color_type = DSCOLOR_SHIPCOLOR;
	strcpy(pl_col[0].title,TXT_PILOT);
	pl_col[0].type = DSCOL_PILOT_NAME;
	pl_col[0].width = 120;

	// The second column is the score, this is a custom column, so in order to get
	// it's data it will call the custom callback.
	pl_col[1].color_type = DSCOLOR_SHIPCOLOR;
	strcpy(pl_col[1].title,TXT_SCORE);
	pl_col[1].type = DSCOL_CUSTOM;
	pl_col[1].width = 50;

	// The third column is the number of kills so far this level, we can use
	// the prefined column type.
	pl_col[2].color_type = DSCOLOR_SHIPCOLOR;
	strcpy(pl_col[2].title,TXT_KILLS_SHORT);
	pl_col[2].type = DSCOL_KILLS_LEVEL;
	pl_col[2].width = 50;

	// The fourth column is the number of deaths so far this level, we can use
	// the predefined column type.
	pl_col[3].color_type = DSCOLOR_SHIPCOLOR;
	strcpy(pl_col[3].title,TXT_DEATHS_SHORT);
	pl_col[3].type = DSCOL_DEATHS_LEVEL;
	pl_col[3].width = 60;

	// The fifth column is the number of suicides so far this level, we can use
	// the predefined column type.
	pl_col[4].color_type = DSCOLOR_SHIPCOLOR;
	strcpy(pl_col[4].title,TXT_SUICIDES_SHORT);
	pl_col[4].type = DSCOL_SUICIDES_LEVEL;
	pl_col[4].width = 65;

	// The sixth and final column is ping, there is also a predefined column
	// type for this also.
	pl_col[5].color_type = DSCOLOR_SHIPCOLOR;
	strcpy(pl_col[5].title,TXT_PING);
	pl_col[5].type = DSCOL_PING;
	pl_col[5].width = 40;

	// We are done setting up all the structs, do the final step,
	// initialize the stats!
	dstat->Initialize(&tsi);	

	//add the death and suicide messages
	// these are the HUD messages that pop-up when a player dies
	// DMFC has one built in death message and one built in suicide message
	DMFCBase->AddDeathMessage(TXT_DEATH1,true);
	DMFCBase->AddDeathMessage(TXT_DEATH2,true);
	DMFCBase->AddDeathMessage(TXT_DEATH3,false);
	DMFCBase->AddDeathMessage(TXT_DEATH4,false);
	DMFCBase->AddDeathMessage(TXT_DEATH5,true);
	DMFCBase->AddDeathMessage(TXT_DEATH6,true);
	DMFCBase->AddDeathMessage(TXT_DEATH7,false);
	DMFCBase->AddDeathMessage(TXT_DEATH8,true);
	DMFCBase->AddDeathMessage(TXT_DEATH9,true);
	DMFCBase->AddDeathMessage(TXT_DEATH10,true);

	DMFCBase->AddSuicideMessage(TXT_SUICIDE1);
	DMFCBase->AddSuicideMessage(TXT_SUICIDE2);
	DMFCBase->AddSuicideMessage(TXT_SUICIDE3);
	DMFCBase->AddSuicideMessage(TXT_SUICIDE4);
	DMFCBase->AddSuicideMessage(TXT_SUICIDE5);
	DMFCBase->AddSuicideMessage(TXT_SUICIDE6);

	// We must tell DMFC how many teams our game is, this sets up a number
	// of states of DMFC.
	DMFCBase->SetNumberOfTeams(1);

// By default, you are not allowed to hurt your teammates.
// And since there is only 1 team in Coop, everyone is considered to be on the same team.
// What we want is to be able to hurt others, so we need to set the netgame flag for damage friendly.
// *Alex :  was '(NF_DAMAGE_FRIENDLY | NF_TRACK_RANK)' in Anarchy
	netgame_info* Netgame = DMFCBase->GetNetgameInfo();
// *it crashes if 'NF_ALLOWGUIDEBOT' is not set, and somebody presses F4
// *'NF_COOP' does not limit 'netgame_info->max_players' to 4, it makes spewed weapons not disappearing, and does not turn off the team damage, and does not make robots attack the cargo ship in Retribution level 9
//	Netgame->flags |= (NF_DAMAGE_FRIENDLY | NF_TRACK_RANK | NF_USE_ROBOTS | NF_RESPAWN_WAYPOINT | NF_ALLOWGUIDEBOT | NF_COOP);
	Netgame->flags |= (NF_DAMAGE_FRIENDLY | NF_TRACK_RANK | NF_USE_ROBOTS | NF_RESPAWN_WAYPOINT | NF_ALLOWGUIDEBOT);
// without robots, for debug
//	Netgame->flags |= (NF_DAMAGE_FRIENDLY | NF_TRACK_RANK | NF_RESPAWN_WAYPOINT | NF_ALLOWGUIDEBOT);

//	DMFCBase->AddInputCommand ("command", "description",void (*handler)(char *),bool allow_remotely=false);
}


// OnHUDInterval
//
//	Event handler for when it's time to render the HUD
void OnHUDInterval(void)
{
	// We must process the stats manager each frame, so
	// we'll do this now. (Which will render it if it's enabled)
	dstat->DoFrame();

	// Since this is an Outrage mod, calling this will display
	// the Outrage logo (we can call it repeatedly, it will just stop
	// working after 5 seconds)
	DMFCBase->DisplayOutrageLogo();

	// Finally, call the default event handler
	DMFCBase->OnHUDInterval();
}

//	OnInterval
//
//	Event handler, called every frame
void OnInterval(void)
{
	// Since we are only simply sorting by (kills-suicides), typical 
	// Coop scoring style, we can call the built in sort function
	// of DMFC, which sorts this way.
	DMFCBase->GetSortedPlayerSlots(SortedPlayers,MAX_PLAYER_RECORDS);

	CoopMod_OnInterval ();
#ifdef WIN32
	Hooks_Frame ();
#endif

	// Finally, call the default event handler
	DMFCBase->OnInterval();
}

// OnKeypress
//
//	Event handler, called whenever the client presses a key
void OnKeypress(int key)
{
	// first get the pointer to the data structure that was passed
	// to the mod, we may need it to tell Descent 3 to ignore
	// this keypress.
	dllinfo *Data = DMFCBase->GetDLLInfoCallData();

	// if Data->iRet is 1 on return from this function, then
	// Descent 3 will not process this keypress.

	Control_OnKeypress (Data);
	if (Data->iRet)
// *or even return
//		goto finish;
		return;

	// see what key was pressed
	switch(key)
	{
	case K_F7:
		// The user wants to display the stats screen, turn off the
		// on-screen menu if it's up
		DisplayScoreScreen = !DisplayScoreScreen;
		DMFCBase->EnableOnScreenMenu(false);
//		Screen_Restore ();
//		__asm int 3
		dstat->Enable(DisplayScoreScreen);
		break;
	case K_PAGEDOWN:
		// The user wants to scroll down a row in the stats manager
		if(DisplayScoreScreen){
			dstat->ScrollDown();
			Data->iRet = 1;
		}
		break;
	case K_PAGEUP:
		// The user wants to scroll up a row in the stats manager
		if(DisplayScoreScreen){
			dstat->ScrollUp();
			Data->iRet = 1;
		}
		break;
	case K_F6:
		// The user wants to go into the On-Screen menu (DMFC automatically
		// turns this on, so we just need to intercept it to turn off
		// the stats screen if it's on)
		DisplayScoreScreen = false;
		dstat->Enable(false);		
		break;
	case K_ESC:
		// If the stats screen is up, close it
		if(DisplayScoreScreen){
			dstat->Enable(false);
			DisplayScoreScreen = false;
			Data->iRet = 1;
		}
		break;
	}

// Alex :  I think this is the right way
	if (Data->iRet)
		return;

//finish:
	// Call the default event handler
	DMFCBase->OnKeypress(key);
}

// Out-dated, nothing done in here now
// -- event gets called when the mod is loaded and about to start
void OnServerGameCreated(void)
{
//	LogFile_Chat_Printf ("* game start");
	DMFCBase->OnServerGameCreated();
}
// -- event gets called for a client when the level ends
void OnClientLevelEnd(void)
{
	DMFCBase->OnClientLevelEnd();
}

//	OnClientLevelStart
//
//	The server has started a new level, so initialize the sort list to all -1
//	since there has been no sorting yet.
void OnClientLevelStart(void)
{
	for(int i=0;i<MAX_PLAYER_RECORDS;i++){
		SortedPlayers[i] = -1;	
	}

	// Call the default event handler
	DMFCBase->OnClientLevelStart();
}

//	OnClientPlayerEntersGame
//
//	A new player has entered the game, zero their stats out
// *is called or through 'OnServerPlayerEntersGame ()' or through 'DLLGameCall ()' EVT_CLIENT_GAMEPLAYERENTERSGAME 0x060c
void OnClientPlayerEntersGame(int player_num)
{
//	Screen_Restore ();
	// Call the default handler first
	DMFCBase->OnClientPlayerEntersGame(player_num);

	// If the player isn't us (this event gets called for everyone, including
	// the player entering the game) then inform us that the player has
	// joined the game
	if(player_num!=DMFCBase->GetPlayerNum())
// *show to myself
		DisplayWelcomeMessage(player_num);
	else // set the flag that we need to display our welcome message the first frame
		display_my_welcome = true;
}

//	OnClientPlayerKilled
//
//	Usually this is done automatically, but we need to handle if the
//	goal score is reached  (to end the level).
void OnClientPlayerKilled(object *killer_obj,int victim_pnum)
{
	// First call the default handler to do the real processesing
	DMFCBase->OnClientPlayerKilled(killer_obj,victim_pnum);

	// Now we need to do the same thing as the default handler, but
	// we just need to see if the player's score is >= the goal.
	// Another way (and perhaps easier) to do this check is just every
	// frame go through all the players and check their score.
	player_record *kpr;

	int kpnum;

	if(killer_obj)
	{
		if((killer_obj->type==OBJ_PLAYER)||(killer_obj->type==OBJ_GHOST))
			kpnum = killer_obj->id;
		else if(killer_obj->type==OBJ_ROBOT || killer_obj->type == OBJ_BUILDING)
		{
			//countermeasure kill
			kpnum = DMFCBase->GetCounterMeasureOwner(killer_obj);
		}
		else
		{
			kpnum = -1;
		}
	}
	else
	{
		kpnum = -1;
	}

	kpr = DMFCBase->GetPlayerRecordByPnum(kpnum);

	if(kpr)
	{
//		int goal;
//		if(DMFCBase->GetScoreLimit(&goal))
//		{
// A Score limit was set for this game, check it to see if it has been
// met.
//			int score = kpr->dstats.kills[DSTAT_LEVEL] - kpr->dstats.suicides[DSTAT_LEVEL];
//			if(score>=goal)
//			{
//				DMFCBase->EndLevel();
//			}
//		}
	}
}

// compares 2 player records's scores to see which one is ahead of the other
bool compare_slots(int a,int b)
{
	int ascore,bscore;
	player_record *apr,*bpr;
	apr = DMFCBase->GetPlayerRecord(a);
	bpr = DMFCBase->GetPlayerRecord(b);
	if( !apr )
		return true;
	if( !bpr )
		return false;
	if( apr->state==STATE_EMPTY )
		return true;
	if( bpr->state==STATE_EMPTY )
		return false;
	if( (apr->state==STATE_INGAME) && (bpr->state==STATE_INGAME) )
	{
		//both players were in the game
		ascore = apr->dstats.kills[DSTAT_LEVEL] - apr->dstats.suicides[DSTAT_LEVEL];
		bscore = bpr->dstats.kills[DSTAT_LEVEL] - bpr->dstats.suicides[DSTAT_LEVEL];
		return (ascore<bscore);
	}
	if( (apr->state==STATE_INGAME) && (bpr->state==STATE_DISCONNECTED) )
	{
		//apr gets priority since he was in the game on exit
		return false;
	}
	if( (apr->state==STATE_DISCONNECTED) && (bpr->state==STATE_INGAME) )
	{
		//bpr gets priority since he was in the game on exit
		return true;
	}
	//if we got here then both players were disconnected
	ascore = apr->dstats.kills[DSTAT_LEVEL] - apr->dstats.suicides[DSTAT_LEVEL];
	bscore = bpr->dstats.kills[DSTAT_LEVEL] - bpr->dstats.suicides[DSTAT_LEVEL];
	return (ascore<bscore);
}

//	OnPLRInit
//
//	Event handler for to initialize anything we need for Post Level Results
void OnPLRInit(void)
{
	int tempsort[MAX_PLAYER_RECORDS];
	int i,t,j;

	for(i=0;i<MAX_PLAYER_RECORDS;i++){
		tempsort[i] = i;
	}

	for(i=1;i<=MAX_PLAYER_RECORDS-1;i++){
		t=tempsort[i];
		// Shift elements down until
		// insertion point found.
		for(j=i-1;j>=0 && compare_slots(tempsort[j],t); j--){
			tempsort[j+1] = tempsort[j];
		}
		// insert
		tempsort[j+1] = t;
	}

	//copy the array over (we only have to copy over DLLMAX_PLAYERS, because
	//that how much of it we are going to use)
	memcpy(SortedPlayers,tempsort,DLLMAX_PLAYERS*sizeof(int));

	DMFCBase->OnPLRInit();
}

// OnPLRInterval
//
//	Frame interval call for Post Level Results
void OnPLRInterval(void)
{
#define PLAYERS_COL		130
#define SCORE_COL		280
#define DEATHS_COL		330
#define SUICIDES_COL	390
#define TOTAL_COL		460

	// The FIRST thing that must be done is call the default handler...this
	// must be first
	DMFCBase->OnPLRInterval();

	//our Y position on the screen
	int y = 40;
	
	// get the height of our font, so we can space correctly
	int height = DLLgrfont_GetHeight((DMFCBase->GetGameFontTranslateArray())[SMALL_UI_FONT_INDEX]) + 1;
	DLLgrtext_SetFont((DMFCBase->GetGameFontTranslateArray())[SMALL_UI_FONT_INDEX]);

	//print out header
	DLLgrtext_SetColor(GR_RGB(255,255,150));
	DLLgrtext_Printf(PLAYERS_COL,y,TXT_PILOT);
	DLLgrtext_Printf(SCORE_COL,y,TXT_KILLS_SHORT);
	DLLgrtext_Printf(DEATHS_COL,y,TXT_DEATHS_SHORT);
	DLLgrtext_Printf(SUICIDES_COL,y,TXT_SUICIDES_SHORT);
	DLLgrtext_Printf(TOTAL_COL,y,TXT_SCORE);
	y+=height;	//move down the Y value

	//print out player stats
	int rank = 1;
	int slot,pnum;
	player_record *pr;

	// We're only going to print out, at most 32 (DLLMAX_PLAYERS)
	for(int i=0;i<DLLMAX_PLAYERS;i++){
		// use the sort array to translate to a real player record index
		slot = SortedPlayers[i];
		pr = DMFCBase->GetPlayerRecord(slot);

		// we only want player records that are for players that have been in the
		// game
		if((pr) && (pr->state!=STATE_EMPTY) ){

			if(DMFCBase->IsPlayerDedicatedServer(pr))
				continue;//skip a dedicated server

			// since at the end of a level, player's disconnect
			// DMFC stores the players that were in the game at
			// level end.  Calling this function with a given player record
			// will return the player number of the player if they were
			// in the game at level end, else it will return -1
			pnum=DMFCBase->WasPlayerInGameAtLevelEnd(slot);
			if(pnum!=-1)
			{
				// set the current color for text to their player ship color
				DLLgrtext_SetColor((DMFCBase->GetPlayerColors())[pnum]);
			}else
			{
				// set the color to grey (they weren't in the game at the end)
				DLLgrtext_SetColor(GR_RGB(128,128,128));
			}

			// Print out the actual information about the player
			char temp[100];
			sprintf(temp,"%d)%s",rank,pr->callsign); //callsign and rank

			// Calling clip string will ensure that a string doesn't run
			// over into other columns.
			DMFCBase->ClipString(SCORE_COL - PLAYERS_COL - 10,temp,true);
			DLLgrtext_Printf(PLAYERS_COL,y,"%s",temp);

			DLLgrtext_Printf(SCORE_COL,y,"%d",pr->dstats.kills[DSTAT_LEVEL]);
			DLLgrtext_Printf(DEATHS_COL,y,"%d",pr->dstats.deaths[DSTAT_LEVEL]);
			DLLgrtext_Printf(SUICIDES_COL,y,"%d",pr->dstats.suicides[DSTAT_LEVEL]);
//			DLLgrtext_Printf(TOTAL_COL,y,"%d",pr->dstats.kills[DSTAT_LEVEL]-pr->dstats.suicides[DSTAT_LEVEL]);
			DLLgrtext_Printf(TOTAL_COL,y,"%d",-pr->dstats.suicides[DSTAT_LEVEL]);
			y+=height;
			rank++;
			if(y>=440)
				goto quick_exit;

		}
	}

quick_exit:;
}

// SaveStatsToFile
//
//	Save our current stats to the given filename
void SaveStatsToFile(char *filename)
{
	CFILE *file;
	DLLOpenCFILE(&file,filename,"wt");
	if(!file){
		mprintf((0,"Unable to open output file\n"));
		return;
	}

	//write out game stats
	#define BUFSIZE	150
	char buffer[BUFSIZE];
	char tempbuffer[25];
	int sortedslots[MAX_PLAYER_RECORDS];
	player_record *pr,*dpr;
	tPInfoStat stat;
	int count,length,p;

	//sort the stats
	DMFCBase->GetSortedPlayerSlots(sortedslots,MAX_PLAYER_RECORDS);
	count = 1;

	sprintf(buffer,TXT_SAVE_HEADER,(DMFCBase->GetNetgameInfo())->name,(DMFCBase->GetCurrentMission())->cur_level);
	DLLcf_WriteString(file,buffer);

	sprintf(buffer,TXT_SAVE_HEADERB);
	DLLcf_WriteString(file,buffer);
	sprintf(buffer,"-----------------------------------------------------------------------------------------------");
	DLLcf_WriteString(file,buffer);


	for(int s=0;s<MAX_PLAYER_RECORDS;s++){
		p = sortedslots[s];
		pr = DMFCBase->GetPlayerRecord(p);
		if( pr && pr->state!=STATE_EMPTY) {

			if(DMFCBase->IsPlayerDedicatedServer(pr))
				continue;	//skip dedicated server

			memset(buffer,' ',BUFSIZE);

			sprintf(tempbuffer,"%d)",count);
			memcpy(&buffer[0],tempbuffer,strlen(tempbuffer));

			sprintf(tempbuffer,"%s%s",(pr->state==STATE_INGAME)?"":"*",pr->callsign);
			memcpy(&buffer[7],tempbuffer,strlen(tempbuffer));

			sprintf(tempbuffer,"%d[%d]",pr->dstats.kills[DSTAT_LEVEL]-pr->dstats.suicides[DSTAT_LEVEL],pr->dstats.kills[DSTAT_OVERALL]-pr->dstats.suicides[DSTAT_OVERALL]);
			memcpy(&buffer[36],tempbuffer,strlen(tempbuffer));

			sprintf(tempbuffer,"%d[%d]",pr->dstats.kills[DSTAT_LEVEL],pr->dstats.kills[DSTAT_OVERALL]);
			memcpy(&buffer[48],tempbuffer,strlen(tempbuffer));

			sprintf(tempbuffer,"%d[%d]",pr->dstats.deaths[DSTAT_LEVEL],pr->dstats.deaths[DSTAT_OVERALL]);
			memcpy(&buffer[60],tempbuffer,strlen(tempbuffer));

			sprintf(tempbuffer,"%d[%d]",pr->dstats.suicides[DSTAT_LEVEL],pr->dstats.suicides[DSTAT_OVERALL]);
			memcpy(&buffer[71],tempbuffer,strlen(tempbuffer));

			sprintf(tempbuffer,"%s",DMFCBase->GetTimeString(DMFCBase->GetTimeInGame(p)));
			memcpy(&buffer[82],tempbuffer,strlen(tempbuffer));
	
			int pos;
			pos = 82 + strlen(tempbuffer) + 1;
			if(pos<BUFSIZE)
				buffer[pos] = '\0';
							
			buffer[BUFSIZE-1] = '\0';
			DLLcf_WriteString(file,buffer);
			count++;
		}
	}

	DLLcf_WriteString(file,TXT_SAVE_HEADERC);

	// go through all the players individually, and write out detailed stats
	count =1;
	for(p=0;p<MAX_PLAYER_RECORDS;p++){
		pr = DMFCBase->GetPlayerRecord(p);
		if( pr && pr->state!=STATE_EMPTY) {

			if(DMFCBase->IsPlayerDedicatedServer(pr))
				continue;	//skip dedicated server

			//Write out header
			sprintf(buffer,"%d) %s%s",count,(pr->state==STATE_INGAME)?"":"*",pr->callsign);
			DLLcf_WriteString(file,buffer);
			length = strlen(buffer);
			memset(buffer,'=',length);
			buffer[length] = '\0';
			DLLcf_WriteString(file,buffer);
			
			//time in game
			sprintf(buffer,TXT_SAVE_TIMEINGAME,DMFCBase->GetTimeString(DMFCBase->GetTimeInGame(p)));
			DLLcf_WriteString(file,buffer);

			// To go through the list of who killed who stats for each player
			// we use FindPInfoStatFirst/FindPInfoStatNext/FindPInfoStatClose to
			// go through the list.
			if(DMFCBase->FindPInfoStatFirst(p,&stat)){
				sprintf(buffer,TXT_SAVE_KILLERLIST);
				DLLcf_WriteString(file,buffer);
				
				if(stat.slot!=p){
					memset(buffer,' ',BUFSIZE);
					dpr = DMFCBase->GetPlayerRecord(stat.slot);
					int pos;

					ASSERT(dpr!=NULL);
					if(dpr){
						sprintf(tempbuffer,"%s",dpr->callsign);
						memcpy(buffer,tempbuffer,strlen(tempbuffer));

						sprintf(tempbuffer,"%d",stat.kills);
						memcpy(&buffer[30],tempbuffer,strlen(tempbuffer));

						sprintf(tempbuffer,"%d",stat.deaths);
						memcpy(&buffer[40],tempbuffer,strlen(tempbuffer));

						pos = 40 + strlen(tempbuffer) + 1;
						if(pos<BUFSIZE)
							buffer[pos] = '\0';

						buffer[BUFSIZE-1] = '\0';
						DLLcf_WriteString(file,buffer);
					}
				}
						
				// keep going through the list until there are no more
				while(DMFCBase->FindPInfoStatNext(&stat)){															
					if(stat.slot!=p){
						int pos;
						memset(buffer,' ',BUFSIZE);
						dpr = DMFCBase->GetPlayerRecord(stat.slot);

						if(dpr)
						{
							sprintf(tempbuffer,"%s",dpr->callsign);
							memcpy(buffer,tempbuffer,strlen(tempbuffer));

							sprintf(tempbuffer,"%d",stat.kills);
							memcpy(&buffer[30],tempbuffer,strlen(tempbuffer));

							sprintf(tempbuffer,"%d",stat.deaths);
							memcpy(&buffer[40],tempbuffer,strlen(tempbuffer));

							pos = 40 + strlen(tempbuffer) + 1;
							if(pos<BUFSIZE)
								buffer[pos] = '\0';

							buffer[BUFSIZE-1] = '\0';
							DLLcf_WriteString(file,buffer);
						}
					}		
				}
			}
			DMFCBase->FindPInfoStatClose();
			DLLcf_WriteString(file,"");	//skip a line
			count++;
		}
	}

	//done writing stats
	DLLcfclose(file);
	DLLAddHUDMessage(TXT_MSG_SAVESTATS);
}

#define ROOTFILENAME	"Co-op"
// The user wants to save the stats to file
void OnSaveStatsToFile(void)
{
	char filename[256];
	DMFCBase->GenerateStatFilename(filename,ROOTFILENAME,false);
	SaveStatsToFile(filename);
}
// The level ended and the user has it set to auto save stats
void OnLevelEndSaveStatsToFile(void)
{
	char filename[256];
	DMFCBase->GenerateStatFilename(filename,ROOTFILENAME,true);
	SaveStatsToFile(filename);
}
// The player has disconnected and has it set to auto save stats
void OnDisconnectSaveStatsToFile(void)
{
	char filename[256];
	DMFCBase->GenerateStatFilename(filename,ROOTFILENAME,false);
	SaveStatsToFile(filename);
}

//	OnPrintScores
//
//	The dedicated server (us) requested to see the scores (via $scores)
void OnPrintScores(int level)
{
	char buffer[256];
	char name[50];
	memset(buffer,' ',256);

	int t;
	int pos[6];
	int len[6];
	pos[0] = 0;					t = len[0] = 20;	//give ample room for pilot name
	pos[1] = pos[0] + t + 1;	t = len[1] = strlen(TXT_POINTS);
	pos[2] = pos[1] + t + 1;	t = len[2] = strlen(TXT_KILLS_SHORT);
	pos[3] = pos[2] + t + 1;	t = len[3] = strlen(TXT_DEATHS_SHORT);
	pos[4] = pos[3] + t + 1;	t = len[4] = strlen(TXT_SUICIDES_SHORT);
	pos[5] = pos[4] + t + 1;	t = len[5] = strlen(TXT_PING);

	memcpy(&buffer[pos[0]],TXT_PILOT,strlen(TXT_PILOT));
	memcpy(&buffer[pos[1]],TXT_POINTS,len[1]);
	memcpy(&buffer[pos[2]],TXT_KILLS_SHORT,len[2]);
	memcpy(&buffer[pos[3]],TXT_DEATHS_SHORT,len[3]);
	memcpy(&buffer[pos[4]],TXT_SUICIDES_SHORT,len[4]);
	memcpy(&buffer[pos[5]],TXT_PING,len[5]);
	buffer[pos[5]+len[5]+1] = '\n';
	buffer[pos[5]+len[5]+2] = '\0';
	DPrintf(buffer);

	int slot;
	player_record *pr;
	int pcount;

	if(level<0 || level>=MAX_PLAYER_RECORDS)
		pcount = MAX_PLAYER_RECORDS;
	else
		pcount = level;

	int sortedplayers[MAX_PLAYER_RECORDS];
	DMFCBase->GetSortedPlayerSlots(sortedplayers,MAX_PLAYER_RECORDS);
	
	for(int i=0;i<pcount;i++){
		slot = sortedplayers[i];
		pr = DMFCBase->GetPlayerRecord(slot);
		if((pr)&&(pr->state!=STATE_EMPTY)){

			if(DMFCBase->IsPlayerDedicatedServer(pr))
				continue;	//skip dedicated server

			sprintf(name,"%s%s:",(pr->state==STATE_DISCONNECTED)?"*":"",pr->callsign);
			name[19] = '\0';

			memset(buffer,' ',256);
			t = strlen(name); memcpy(&buffer[pos[0]],name,(t<len[0])?t:len[0]);
//			sprintf(name,"%d",pr->dstats.kills[DSTAT_LEVEL]-pr->dstats.suicides[DSTAT_LEVEL]);
			sprintf(name,"%d",- (pr->dstats.kills[DSTAT_LEVEL] + pr->dstats.suicides[DSTAT_LEVEL]));
			t = strlen(name); memcpy(&buffer[pos[1]],name,(t<len[1])?t:len[1]);
			sprintf(name,"%d",pr->dstats.kills[DSTAT_LEVEL]);
			t = strlen(name); memcpy(&buffer[pos[2]],name,(t<len[2])?t:len[2]);
			sprintf(name,"%d",pr->dstats.deaths[DSTAT_LEVEL]);
			t = strlen(name); memcpy(&buffer[pos[3]],name,(t<len[3])?t:len[3]);
			sprintf(name,"%d",pr->dstats.suicides[DSTAT_LEVEL]);
			t = strlen(name); memcpy(&buffer[pos[4]],name,(t<len[4])?t:len[4]);

			if(pr->state==STATE_INGAME)
				sprintf(name,"%.0f",(DMFCBase->GetNetPlayers())[pr->pnum].ping_time*1000.0f);
			else
				strcpy(name,"---");
			t = strlen(name); memcpy(&buffer[pos[5]],name,(t<len[5])?t:len[5]);
			buffer[pos[5]+len[5]+1] = '\n';
			buffer[pos[5]+len[5]+2] = '\0';
			DPrintf(buffer);
		}
	}
}

// DisplayHUDScores
//
//	Our callback function when it's time to render what we need on the HUD
void DisplayHUDScores(struct tHUDItem *hitem)
{
	// It's definitly ok to display the welcome message now
	// before (when we joined) it wasn't a good idea because the first
	// frame of the game hasn't been displayed yet, so internal variables (for
	// screen size) haven't been set yet, and so if we tried to display a HUD
	// message before the first frame, it would get clipped by a pretty much
	// random value (for screen size)....now it is ok, if we are scheduled to
	// display our welcome message
	if(display_my_welcome)
	{
		DisplayWelcomeMessage(DMFCBase->GetPlayerNum());
		display_my_welcome = false;
	}

	// Don't draw anything on the HUD if our F7 stats screen is up
	if(DisplayScoreScreen)
		return;
	
	int height = DLLgrfont_GetHeight((DMFCBase->GetGameFontTranslateArray())[HUD_FONT_INDEX]) + 3;

	// ConvertHUDAlpha takes the given alpha value, and if the F6 On-Screen menu is
	// up, it will lower it a little bit, to dim HUD items
	ubyte alpha = DMFCBase->ConvertHUDAlpha((ubyte)(255));
	int y = (DMFCBase->GetGameWindowH()/2) - ((height*5)/2);
	int x = 520;
	ddgr_color color;

	int rank = 1;
	player_record *pr;

// Alex off
/*
	//Display your Kills & Deaths on the top corners of the screen
	pr = DMFCBase->GetPlayerRecordByPnum(DMFCBase->GetPlayerNum());
	if(pr)
	{
		int y = 25,x;
		int lwidth;
		char buffer[20];

		int w_kill,w_death,max_w;
		w_kill = DLLgrtext_GetTextLineWidth(TXT_KILLS);
		w_death = DLLgrtext_GetTextLineWidth(TXT_DEATHS);
		max_w = max(w_kill,w_death);

		x = DMFCBase->GetGameWindowW() - DMFCBase->GetGameWindowW()*0.0078125f;
		DLLgrtext_SetColor(GR_GREEN);
		DLLgrtext_SetAlpha(alpha);
		DLLgrtext_Printf(x-(max_w/2)-(w_kill/2),y,TXT_KILLS);
		y+=height;

		sprintf(buffer,"%d",pr->dstats.kills[DSTAT_LEVEL]);
		lwidth = DLLgrtext_GetTextLineWidth(buffer);
		DLLgrtext_SetColor(GR_GREEN);
		DLLgrtext_SetAlpha(alpha);
		DLLgrtext_Printf(x-(max_w/2)-(lwidth/2),y,buffer);
		y+=height+3;

		DLLgrtext_SetColor(GR_GREEN);
		DLLgrtext_SetAlpha(alpha);
		DLLgrtext_Printf(x - (max_w/2) - (w_death/2),y,TXT_DEATHS);
		y+=height;

		sprintf(buffer,"%d",pr->dstats.deaths[DSTAT_LEVEL]);
		lwidth = DLLgrtext_GetTextLineWidth(buffer);
		DLLgrtext_SetColor(GR_GREEN);
		DLLgrtext_SetAlpha(alpha);
		DLLgrtext_Printf(x - (max_w/2) - (lwidth/2),y,buffer);
	}
*/

	int ESortedPlayers[DLLMAX_PLAYERS];

	switch(coop_hud_display)
	{
	case AHD_NONE:
		return;
		break;
	case AHD_EFFICIENCY:
		DMFCBase->GetSortedPlayerSlots(ESortedPlayers,DLLMAX_PLAYERS);
		break;
	}

	char name[30];

	//determine coordinates to use here
	//we'll use a virtual width of 85 pixels on a 640x480 screen
	//so first determine the new width
	int name_width = 85.0f * DMFCBase->GetHudAspectX();
	int score_width = DLLgrtext_GetTextLineWidth("888");
	int name_x = DMFCBase->GetGameWindowW() - name_width - score_width - 10;
	int score_x = DMFCBase->GetGameWindowW() - score_width - 5;

	for(int i=0;i<DLLMAX_PLAYERS;i++)
	{
		int slot;

		if(coop_hud_display==AHD_EFFICIENCY)
			slot = ESortedPlayers[i];
		else
			slot = SortedPlayers[i];

		pr = DMFCBase->GetPlayerRecord(slot);

		if((pr)&&(pr->state!=STATE_EMPTY)){

			if(DMFCBase->IsPlayerDedicatedServer(pr))
				continue;	//skip dedicated server

			if( (pr->state==STATE_DISCONNECTED) || (pr->state==STATE_INGAME && !DMFCBase->IsPlayerObserver(pr->pnum)) ){
	
				if(pr->pnum==DMFCBase->GetPlayerNum()){

					switch(HUD_color_model){
					case ACM_PLAYERCOLOR:
						color = (DMFCBase->GetPlayerColors())[pr->pnum];
						break;
					case ACM_NORMAL:
						color = GR_RGB(40,255,40);
						break;
					};					

					if(Highlight_bmp>BAD_BITMAP_HANDLE){
						//draw the highlite bar in the background
						DLLrend_SetAlphaValue(alpha*0.50f);
						DLLrend_SetZBufferState (0);
						DLLrend_SetTextureType (TT_LINEAR);
						DLLrend_SetLighting (LS_NONE);
						DLLrend_SetAlphaType (AT_CONSTANT_TEXTURE);
//						DLLrend_DrawScaledBitmap(name_x-2,y-2,score_x+score_width+2,y+height-1,Highlight_bmp,0,0,1,1,1.0);
// Alex :  fix for the compilers beginning from GCC 3.4.x, because of C++ standard
						DLLrend_DrawScaledBitmap(name_x-2,y-2,score_x+score_width+2,y+height-1,Highlight_bmp,0,0,1,1,1.0, -1, NULL);
						DLLrend_SetZBufferState (1);
					}

					strcpy(name,pr->callsign);
					DMFCBase->ClipString(name_width,name,true);
						
					DLLgrtext_SetAlpha(alpha);
					DLLgrtext_SetColor(color);
					DLLgrtext_Printf(name_x,y,"%s",name);

					if(coop_hud_display==AHD_EFFICIENCY){
						float t = pr->dstats.kills[DSTAT_LEVEL]+pr->dstats.suicides[DSTAT_LEVEL]+pr->dstats.deaths[DSTAT_LEVEL];
						float value = (float)(pr->dstats.kills[DSTAT_LEVEL])/((t)?t:0.0000001f);
						DLLgrtext_Printf(score_x,y,"%.1f",value);
					}
					else
					{
//						DLLgrtext_Printf(score_x,y,"%d",pr->dstats.kills[DSTAT_LEVEL]-pr->dstats.suicides[DSTAT_LEVEL]);
						int efficiency = CoopMod_GetEfficiency (slot);
						if (efficiency >= 0)
							DLLgrtext_Printf (score_x, y, "%d", efficiency);
					}
					
					y+=height;
				}
				else if (rank < hud_names_max)
				{

					if(pr->state==STATE_DISCONNECTED){
						color = GR_GREY;
					}else{
						switch(HUD_color_model){
						case ACM_PLAYERCOLOR:
							color = (DMFCBase->GetPlayerColors())[pr->pnum];
							break;
						case ACM_NORMAL:
							color = GR_RGB(40,255,40);
							break;
						};
					}
					strcpy(name,pr->callsign);
					DMFCBase->ClipString(name_width,name,true);

					DLLgrtext_SetAlpha(alpha);
					DLLgrtext_SetColor(color);
					DLLgrtext_Printf(name_x,y,"%s",name);

					if(coop_hud_display==AHD_EFFICIENCY)
					{
						float t = pr->dstats.kills[DSTAT_LEVEL]+pr->dstats.suicides[DSTAT_LEVEL]+pr->dstats.deaths[DSTAT_LEVEL];
						float value = (float)(pr->dstats.kills[DSTAT_LEVEL])/((t)?t:0.0000001f);
						DLLgrtext_Printf(score_x,y,"%.1f",value);
					}
					else
					{
//						DLLgrtext_Printf(score_x,y,"%d",-(pr->dstats.kills[DSTAT_LEVEL] + pr->dstats.suicides[DSTAT_LEVEL]));
						int efficiency = CoopMod_GetEfficiency (slot);
						if (efficiency >= 0)
							DLLgrtext_Printf (score_x, y, "%d", efficiency);
					}

					y+=height;
				}
				rank++;
			}
		}
	}
}

// DisplayWelcomeMessage
//
//	Given a player number, if it's us, then say welcome to the game, if it's
//	someone else, say that they have joined the game
void DisplayWelcomeMessage(int player_num)
{
	char name_buffer[64];
	strcpy(name_buffer,(DMFCBase->GetPlayers())[player_num].callsign);

	if(player_num==DMFCBase->GetPlayerNum())
	{
		DLLAddHUDMessage(TXT_WELCOME,name_buffer);
	}
	else
	{
		DLLAddHUDMessage(TXT_JOINED,name_buffer);
	}
}

// SwitchHUDColor
//	
//	Used by the On-Screen menu when the user has changed the HUD color style
void SwitchHUDColor(int i)
{
	if(i<0 || i>1)
		return;
	HUD_color_model = i;

	switch(HUD_color_model){
	case ACM_PLAYERCOLOR:
		DLLAddHUDMessage(TXT_MSG_COLORPLR);
		break;
	case ACM_NORMAL:
		DLLAddHUDMessage(TXT_MSG_COLORNORM);
		break;
	};
}

// SwitchCoopScores
//
//	Call this function to switch the HUD display mode for what
//  we display on the HUD (Player Scores/Player Efficiency/Nothing)
void SwitchCoopScores(int i)
{
	// clamp the values to make sure they are valid
	if(i<0)
		i = 0;
	if(i>2)
		i = 2;

	// Set the new HUD display mode
	coop_hud_display = i;

	// Give a HUD message to the player telling them the HUD mode has changed
	switch(i){
	case AHD_NONE:
		DLLAddHUDMessage(TXT_HUDD_NONE);
		break;
	case AHD_SCORE:
		DLLAddHUDMessage(TXT_HUDD_SCORES);
		break;
	case AHD_EFFICIENCY:
		DLLAddHUDMessage(TXT_HUDD_EFFIC);
		break;
	};
}

// The main entry point where the game calls the dll
// Just translate the event, this function should always be the
// same, just pass control to DMFC
void DLLFUNCCALL DLLGameCall (int eventnum,dllinfo *data)
{
	u32 pnum;

//	if (eventnum == EVT_CREATED)
//		DLLAddHUDMessage ("----- object is created -----");
// Alex :  this event is as 'DMFCBase->OnInputString (input_string)' - for "$...." string only too
//	if (eventnum == EVT_CLIENT_INPUT_STRING)
//		return;

// Alex :  not helped - strings from other clients not gone through here
// Alex :  not helped - objects spawn/spew not gone through here
// grey scratcher scratched weapons do not produce any event here

#if 0
	switch (eventnum)
	{
	case 0x0502:
	case 0x0512:
	case 0x0513:
	case 0x0611:
	case 0x0612:
//	case 0x0620:
	case 0x0624:
		break;

	case EVT_CLIENT_USE:
		DLLAddHUDMessage ("'EVT_CLIENT_USE' %08x, %08x, %08x", data->me_handle, data->it_handle, data->iParam);
#if 1
	default:
		if (data)
//		if (data && data->input_string)
//		if (data && data->input_string && data->input_string[0])
		{
			FILE* f;

			f = fopen ("dbg2.txt", "ab");

//			if (data->input_string)
//				fprintf (f, "%04x \"%s\"", eventnum, data->input_string);
//			else
				fprintf (f, "%04x", eventnum);
			fprintf (f, "\r\n");

			fclose (f);
		}
#endif
	}
#endif

	// Filter out all server events from clients
	// this is important, or weird things will happen (a client
	// trying to preform server calls)
	if((eventnum<EVT_CLIENT_INTERVAL) && (DMFCBase->GetLocalRole()!=LR_SERVER))
	{
		return;
	}

// we may also need some control
// its just a single event, without a keyup pair
// events from other clients aren't rising here; only from self
//	if (eventnum == EVT_CLIENT_KEYPRESS)
//	{
//	}

// I am not sure, maybe there is a single event of shot, instead of this dual/quad, which actually means a weapon bullet creation
	if (eventnum != EVT_WEAPONFIRED)
		goto after_weaponfired;
	if (DMFCBase->GetLocalRole() != LR_SERVER)
		goto after_weaponfired;
	msafe_struct mstruct;
	object* obj;
// there is no pnum in the structure; there is no ammo amount also
//// the heck, don't know atm how to get pnum from objhandle
// 'it_handle' contains plr obj; 'me_handle' contains bullet obj
	mstruct.objhandle = data->it_handle;
	MSafe_GetValue (MSAFE_OBJECT_TYPE, &mstruct);
	if (mstruct.type != OBJ_PLAYER)
		goto after_weaponfired;
//	Screen_Restore ();
	MSafe_GetValue (MSAFE_OBJECT_ID, &mstruct);
	pnum = mstruct.id;
	DLLObjGet (data->me_handle, &obj);
	if (! obj)
		goto after_weaponfired;
	if (obj->type != OBJ_WEAPON)
		goto after_weaponfired;
// there is no ammo amount in the object data :/ (34 units single (dual) shot in Phoenix)
// may also try 'GetObjectInfo()', to get 'otype_wb_info', but I don't think there will be
	obj = obj;
// so no ammo amount :/
// only for remote clients
	if (pnum)
		Drop_CountAmmo (pnum, 0);
after_weaponfired:

// findout unknown events
// here are no scores on/off events
#if 0
	switch (eventnum)
	{
	case 0x502:
	case 0x503:
// early, 1st
	case 0x506:
// 2nd event
	case 0x508:
	case 0x510:
	case 0x512:
	case 0x513:
	case 0x514:
	case 0x515:
	case 0x611:
	case 0x612:
// 3rd
	case 0x619:
	case 0x620:
// 4th
	case 0x624:
		break;

	default:
		Screen_Restore ();
		eventnum = eventnum;
	}
#endif
#if 0
	if (eventnum == 0x512)
	{
//		Screen_Restore ();
		eventnum = eventnum;
		if (data->special_data[0x20])
		{
			Screen_Restore ();
			eventnum = eventnum;
		}
	}
#endif
#if 0
	if (eventnum == EVT_CLIENT_GAMEPLAYERENTERSGAME)
	{
//		Screen_Restore ();
		eventnum = eventnum;
//		if (data->special_data[0x20])
		{
			Screen_Restore ();
			eventnum = eventnum;
		}
	}
#endif
#if 0
	if (eventnum == EVT_GAME_INTERVAL)
	{
		float f;
		Screen_Restore ();
		eventnum = eventnum;
// both floats are zeroes
		f = data->fParam;
		f = data->fRet;
	}
#endif

	DMFCBase->TranslateEvent(eventnum,data);
}

/*
*****************************************
OSIRIS Section
	- Since no OSIRIS scripts needed for
Coop, they are all just stub functions
*****************************************
*/

// Alex :  it differs from the Dallas, - the Dallas does not need that 'GetGOScriptID ()' return valid value

//	GetGOScriptID
//	Purpose:
//		Given the name of the object (from it's pagename), this function will search through it's
//	list of General Object Scripts for a script with a matching name (to see if there is a script
//	for that type/id of object within this DLL).  If a matching scriptname is found, a UNIQUE ID
//	is to be returned back to Descent 3.  This ID will be used from here on out for all future
//	interaction with the DLL.  Since doors are not part of the generic object's, it's possible
//	for a door to have the same name as a generic object (OBJ_POWERUP, OBJ_BUILDING, OBJ_CLUTTER
//	or OBJ_ROBOT), therefore, a 1 is passed in for isdoor if the given object name refers to a
//	door, else it is a 0.  The return value is the unique identifier, else -1 if the script
//	does not exist in the DLL.
int DLLFUNCCALL GetGOScriptID(char *name,ubyte isdoor)
{
#if 0
	FILE* f;

	f = fopen ("dbg4.txt", "ab");

	fprintf (f, "\"%s\", %02x", name, isdoor);
	fprintf (f, "\r\n");

	fclose (f);
#endif

//	return -1;
	return 0;
}

//	CreateInstance
//	Purpose:
//		Given an ID from a call to GetGOScriptID(), this function will create a new instance for that
//	particular script (by allocating and initializing memory, etc.).  A pointer to this instance
//	is to be returned back to Descent 3.  This pointer will be passed around, along with the ID
//	for CallInstanceEvent() and DestroyInstance().  Return NULL if there was an error.
void DLLFUNCCALLPTR CreateInstance(int id)
{
	void* ptr = (void*)1;
#if 0
FILE* f;

	f = fopen ("dbg3.txt", "ab");

	fprintf (f, "%08x", id);
	fprintf (f, "\r\n");

	fclose (f);
#endif

//	return NULL;
	return ptr;
}

//	DestroyInstance
//	Purpose:
//		Given an ID, and a pointer to a particular instance of a script, this function will delete and
//	destruct all information associated with that script, so it will no longer exist.
void DLLFUNCCALL DestroyInstance(int id,void *ptr)
{
}

//	CallInstanceEvent
//	Purpose:
//		Given an ID, a pointer to a script instance, an event and a pointer to the struct of
//	information about the event, this function will translate who this event belongs to and
//	passes the event to that instance of the script to be handled.  Return a combination of
//	CONTINUE_CHAIN and CONTINUE_DEFAULT, to give instructions on what to do based on the
//	event. CONTINUE_CHAIN means to continue through the chain of scripts (custom script, level
//	script, mission script, and finally default script).  If CONTINUE_CHAIN is not specified,
//	than the chain is broken and those scripts of lower priority will never get the event.  Return
//	CONTINUE_DEFAULT in order to tell D3 if you want process the normal action that is built into
//	the game for that event.  This only pertains to certain events.  If the chain continues
//	after this script, than the CONTINUE_DEFAULT setting will be overridden by lower priority
//	scripts return value.
short DLLFUNCCALL CallInstanceEvent(int id,void *ptr,int event,tOSIRISEventInfo *data)
{
	object* obj;
	msafe_struct mstruct;
	bool result;

// no any special event here when a grey scratches a concusion missile out of a ship, except if EVT_CREATED

#if 0
	switch (event)
	{
	case 0x0100:
	case 0x0101:
	case 0x0103:
//	case 0x0110:
	case 0x011c:
	case 0x0120:
	case 0x0121:
//	case 0x0100:
		break;
	case 0x0110:
		if (data->evt_ai_notify.notify_type == 0x04)
			break;
		if (data->evt_ai_notify.notify_type == 0x05)
			break;
		Beep (10000, 20);
		Sleep (20);
	default:
		FILE* f;
		f = fopen ("dbg5.txt", "ab");
		fprintf (f, "%04x, %04x", id, event);
		fprintf (f, "\r\n");
		fclose (f);
	}
#endif

	switch (event)
	{
//	case 0x0102:
	case EVT_DAMAGED:
		break;

//	case 0x0103:
	case EVT_COLLIDE:
//		DLLAddHUDMessage ("CallInstanceEvent (EVT_COLLIDE)");
		result = CoopMod_EvtCollide (data->me_handle, data->evt_collide.it_handle);
		if (! result)
// block, if not allowed to take
			return 0;
		break;

// *is not requested for the guidebot creation in some cases
//	case 0x0104:
	case EVT_CREATED:
//		DLLAddHUDMessage ("CallInstanceEvent (EVT_CREATED)");
//		data->me_handle;
		DLLObjGet (data->me_handle, &obj);
		if (! obj)
			goto no_obj;
//		Screen_Restore ();
//		obj = obj;
// make a spewed object to be of a limited lifetime; otherwise on lets say Mercenary level 3, a joined players can play only a half of the level; later it disconnects too easy
// identify the cause of that object creation
// heh, don't use the value of esp here as an orient - it is too volatile :)
		u32 a;
		a = ((u32*)data)[3];
//			if (a == 0x00437a3e)
		if (a < 0x00410000)
			goto next_test_on_obj;
		if (a >= 0x00500000)
			goto next_test_on_obj;
// spews from containers and robots
// push ecx, edx, edi, eax is unique, as the next 'mov edx,[byte esp+0x28]' also is
		if (*(u32*)(a - 9) != 0x50575251)
			goto next_test_on_obj;
#if 0
		Beep (10000, 10);
		Sleep (10);
#endif
// they actually all are without lifetime
		if (! (obj->flags & OF_USES_LIFELEFT))
		{
//			float f;
			obj->flags |= OF_USES_LIFELEFT;
//			obj->lifeleft = 5.0f;
// oops, server shuts down, if 255 seconds or more
#if 0
// 5-10 minutes
			f = rand();
			f *= 5.0 / 32768;
			f += 5.0;
			f *= 60.0;
//					f = 254;
			obj->lifeleft = f;
#endif
			obj->lifeleft = 254.0f;
		}

next_test_on_obj:
#if 0
		if (IS_GUIDEBOT(obj))
		{
			gb_released_handle = data->me_handle;
			gb_released_obj = obj;

//			int room_num = 0;

//			obj->id = 13;
//			mstruct.objhandle = data->me_handle;
//			mstruct.id = 14;
//			MSafe_CallFunction (MSAFE_OBJECT_ID, &mstruct);
//			mstruct.playsound = 0;
//			MSafe_CallFunction (MSAFE_OBJECT_REMOVE, &mstruct);

//	MSafe_GetValue (MSAFE_OBJECT_ORIENT, &mstruct);
//	Room_Value (room_num, VF_GET, RMSV_V_PORTAL_PATH_PNT, &mstruct.pos, 0);
//	DLLAddHUDMessage ("new position is :  %f, %f, %f", mstruct.pos.x, mstruct.pos.y, mstruct.pos.z);
//	mstruct.roomnum = room_num;
//	MSafe_CallFunction (MSAFE_OBJECT_WORLD_POSITION, &mstruct);

			DLLAddHUDMessage ("GB release, obj->type %08x", obj->type);
//			CoopMod_OnGB_Release (obj);
		}
#endif
no_obj:
		break;

	case EVT_DESTROY:
		CoopMod_EvtDestroy (data->me_handle, data->evt_destroy.is_dying);
		break;

// *in case of use GB by F4 it returns a handle at 'OBJ_DUMMY' for the 'me_handle', and 'OBJ_NONE' for the 'evt_use.it_handle' (handle value is really not valid)
	case EVT_USE:
#if 0
//		mstruct.objhandle = data->me_handle;
		mstruct.objhandle = data->evt_use.it_handle;
		MSafe_GetValue (MSAFE_OBJECT_TYPE, &mstruct);
		MSafe_GetValue (MSAFE_OBJECT_ID, &mstruct);

		DLLAddHUDMessage ("'CallInstanceEvent (EVT_USE)' :  it_handle %08X, type %08X, id %08X", data->evt_use.it_handle, mstruct.type, mstruct.id);
//		data->me_handle;
		DLLObjGet (data->evt_use.it_handle, &obj);
		if (obj)
		{
			DLLAddHUDMessage ("use :  obj->type %08x, obj->id %08x", obj->type, obj->id);

			if (IS_GUIDEBOT(obj))
			{
				gb_released_handle = data->me_handle;
				gb_released_obj = obj;

//			msafe_struct mstruct;
//			int room_num = 0;

//				obj->id = 13;
//				mstruct.objhandle = data->me_handle;
//				mstruct.id = 14;
//				MSafe_CallFunction (MSAFE_OBJECT_ID, &mstruct);
//				mstruct.playsound = 0;
//				MSafe_CallFunction (MSAFE_OBJECT_REMOVE, &mstruct);

//	MSafe_GetValue (MSAFE_OBJECT_ORIENT, &mstruct);
//	Room_Value (room_num, VF_GET, RMSV_V_PORTAL_PATH_PNT, &mstruct.pos, 0);
//	DLLAddHUDMessage ("new position is :  %f, %f, %f", mstruct.pos.x, mstruct.pos.y, mstruct.pos.z);
//	mstruct.roomnum = room_num;
//	MSafe_CallFunction (MSAFE_OBJECT_WORLD_POSITION, &mstruct);

				DLLAddHUDMessage ("GB release, obj->type %08x", obj->type);
				CoopMod_OnGB_Release (obj);
			}
		}
#endif
		break;

// *'OBJ_DUMMY' again is for guidebot :/, and first goes 'AIN_USER_DEFINED' notification,
// and later here comes 'AIN_PLAYER_SEES_YOU' notifications
//	case 0x0110:
	case EVT_AI_NOTIFY:

//typedef struct{
//	int notify_type;
//	int it_handle;
//	int goal_num;
//	int goal_uid;
//	union{
//		int enabler_num;
//		int attack_num;
//	};	
//}tOSIRISEVTAINOTIFY;								// struct for EVT_AI_NOTIFY data
 
		mstruct.objhandle = data->me_handle;
		MSafe_GetValue (MSAFE_OBJECT_TYPE, &mstruct);
		MSafe_GetValue (MSAFE_OBJECT_ID, &mstruct);

		if ((mstruct.type == OBJ_ROBOT && mstruct.id > 0) || mstruct.type == OBJ_CLUTTER)
// *many events
			break;

		if (0)
		{
			DLLAddHUDMessage ("'CallInstanceEvent (EVT_AI_NOTIFY)' :  notify_type %08X, me_handle %08X, type %08X, id %08X", data->evt_ai_notify.notify_type, data->me_handle, mstruct.type, mstruct.id);

			mstruct.objhandle = data->evt_use.it_handle;
			MSafe_GetValue (MSAFE_OBJECT_TYPE, &mstruct);
			MSafe_GetValue (MSAFE_OBJECT_ID, &mstruct);

			DLLAddHUDMessage ("it_handle %08X, type %08X, id %08X", data->evt_use.it_handle, mstruct.type, mstruct.id);
		}

		if (mstruct.type == OBJ_DUMMY && mstruct.id == 0 && data->evt_ai_notify.notify_type == AIN_USER_DEFINED)
		{
			MSafe_GetValue (MSAFE_OBJECT_PARENT, &mstruct);
			MSafe_GetValue (MSAFE_OBJECT_TYPE, &mstruct);
			MSafe_GetValue (MSAFE_OBJECT_ID, &mstruct);

//			DLLAddHUDMessage ("'EVT_AI_NOTIFY' parent object is :  type %08X, id %08X", mstruct.type, mstruct.id);

// *observer can release guidebot too
			if (mstruct.type == OBJ_PLAYER || mstruct.type == OBJ_OBSERVER)
			{
//				gb_released_handle = data->me_handle;
//				gb_released_obj = obj;

				switch (coop_anti_guidebot_mode)
				{
// nothing
				case 0:
					{
						ddgr_color color = GR_RED;
// it still is needed to show some info, while such actions may crash the server
//						DLLAddColoredHUDMessage (color, "'EVT_AI_NOTIFY' caused by the player #%d", mstruct.id);
						DLLAddColoredHUDMessage (color, "'AIN_USER_DEFINED' caused by the player #%d", mstruct.id);
					}
					break;

// ignore event
				case 1:
//					mstruct.objhandle = data->me_handle;
//					MSafe_CallFunction (MSAFE_OBJECT_REMOVE, &mstruct);
//					DLLAddHUDMessage ("attempt to remove guidebot object");

					CoopMod_OnGB_Release_Notify (mstruct.id);
					return 0;
//					goto remove_guidebot;

// kick + ignore
				case 2:
// kick immediately
				case 3:
// kick, but do not block the event
				case 4:
// + kick immediately
				case 5:
// removing this object produces crashes
//					mstruct.objhandle = data->me_handle;
//					MSafe_CallFunction (MSAFE_OBJECT_REMOVE, &mstruct);
//					DLLAddHUDMessage ("attempt to remove guidebot object");

//					CoopMod_Kick (mstruct.id, 1, "Guidebot release");
					CoopMod_Kick (mstruct.id, coop_anti_guidebot_mode == 2, "AI scripts usage");
//					CoopMod_OnGB_Release_Notify (mstruct.id);

					if (coop_anti_guidebot_mode < 4)
// *seems it is requested only once, if the case is not such :(
// crashy :/
//						return 0;
// crashy too
						return CONTINUE_DEFAULT;
//					goto remove_guidebot;

				default:
					;
				}

//remove_guidebot:
//				mstruct.objhandle = data->me_handle;
//				MSafe_CallFunction (MSAFE_OBJECT_REMOVE, &mstruct);
//				DLLAddHUDMessage ("attempt to remove guidebot object");
//				return 0;
			}
			else if (mstruct.type == OBJ_GHOST)
			{
				CoopMod_ConnectionActivity (mstruct.id);
			}
		}

		break;

// no anything :/
#if 0
//	case 0x0096:
	case MSAFE_MISC_HUD_MESSAGE:
		DLLAddHUDMessage ("MSAFE_MISC_HUD_MESSAGE");
		break;

//	case 0x00a0:
	case MSAFE_MISC_FILTERED_HUD_MESSAGE:
		DLLAddHUDMessage ("MSAFE_MISC_FILTERED_HUD_MESSAGE");
		break;
#endif

//	case 0x0502:
//	case 0x0512:
//	case 0x0611:
//	case 0x0624:
//		break;

	case 0x0100:
//	case 0x0101:

//	case 0x0115:
//	case 0x011e:
//	case 0x0123:
//	case 0x0125:
//	case 0x0126:
		break;

	default:
		if (0)
		{
		FILE* f;

		float time = DMFCBase->GetGametime ();

			f = fopen ("dbg8.txt", "ab");

			fprintf (f, "%04x, %1f", event, time);
			fprintf (f, "\r\n");

			fclose (f);
		}
	}

	return CONTINUE_CHAIN|CONTINUE_DEFAULT;
}

//	SaveRestoreState
//	Purpose:
//		This function is called when Descent 3 is saving or restoring the game state.  In this function
//	you should save/restore any global data that you want preserved through load/save (which includes
//	demos).  You must be very careful with this function, corrupting the file (reading or writing too
//	much or too little) may be hazardous to the game (possibly making it impossible to restore the
//	state).  It would be best to use version information to keep older versions of saved states still
//	able to be used.  IT IS VERY IMPORTANT WHEN SAVING THE STATE TO RETURN THE NUMBER OF _BYTES_ WROTE
//	TO THE FILE.  When restoring the data, the return value is ignored.  saving_state is 1 when you should
//	write data to the file_ptr, 0 when you should read in the data.
int DLLFUNCCALL SaveRestoreState( void *file_ptr, ubyte saving_state )
{
	return 0;
}

#ifdef MACINTOSH
#pragma export off
#endif

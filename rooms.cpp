// this file must not exist at all;
// this problem must be resolved by normal way, instead of such hacks;
// but ... :/
// stupid stuff here, what I can say more

#include "rooms.h"
#include <stdio.h>
#ifdef WIN32
#include <io.h>
#endif
//#include "idmfc.h"
#include "coop.h"
#include "coop_mod.h"
#include "missions.h"
#include "assert.h"
#ifdef __unix__
// 'chsize ()'
//#include <unistd.h>
#endif


// todo :  go to "logs/rooms" folder instead of "log"
#define ROOMS_LOG_DIR "log"
#define ROOMS_LOG_FILENAME "rooms_dbg"
#define ROOMS_LOG_EXTENSION "txt"
#define ROOMS_LOG_FILEPATH ROOMS_LOG_DIR"\\"ROOMS_LOG_FILENAME"."ROOMS_LOG_EXTENSION

// *perhaps OS writes full clusters, and not just by one sector (512 bytes)
#define ROOMS_LOG_FILESIZE_MAX 800

// for changings watching
int rooms_players_rooms[DLLMAX_PLAYERS];

bool rooms_debug_logging = 1;

// not helps, crashes appears are not rooms dependent, but powerups dependent
bool rooms_anticrash = 0;

enum
{
	ROOMS_LOCATION_AT_TOP,
	ROOMS_LOCATION_AT_BOTTOM,
	ROOMS_LOCATION_AT_NORTH,
	ROOMS_LOCATION_AT_SOUTH,
	ROOMS_LOCATION_AT_EAST,
	ROOMS_LOCATION_AT_WEST,
	ROOMS_LOCATIONS_NUM
};

typedef struct
{
	int roomnum;
	int location;
	float coordinate;
	float damage_amount;
	float damage_period;
// the coordinate (x, y or z), depending on the 'location'
} rooms_crash_place_t;

#define ROOMS_CRASH_PLACES_MAX 100

rooms_crash_place_t rooms_crash_places[ROOMS_CRASH_PLACES_MAX];

// current start
//rooms_crash_place_t* rooms_crash_place = 0;

// + data indexation +
enum
{
	ROOMS_RETRIBUTION_LEVEL1,
	ROOMS_RETRIBUTION_LEVEL11,
	ROOMS_RETRIBUTION_LEVEL12,
	ROOMS_RETRIBUTION_LEVEL17,
	ROOMS_ARRAYS_NUM
};

int rooms_list_beginnings[ROOMS_ARRAYS_NUM];
int rooms_list_sizes[ROOMS_ARRAYS_NUM];

// current start
int rooms_list_index = 0;
int rooms_list_begin = 0;
int rooms_list_size = 0;
rooms_crash_place_t* rooms_list = 0;
// - data indexation -

#define ROOMS_STRING_SIZE 300
char string_backup[ROOMS_STRING_SIZE];


void Rooms_Init ()
{
// list
//	int array_index = 0;
	int place_index = 0;
	rooms_crash_place_t crash_place;
	int size;

	string_backup[0] = 0;
// *for 'strncpy ()'
	string_backup[ROOMS_STRING_SIZE - 1] = 0;

// Retribution level 1
// *no places here I know, but just for test
	size = 0;
//	rooms_list_beginnings[array_index] = place_index;
	rooms_list_beginnings[ROOMS_RETRIBUTION_LEVEL1] = place_index;

#if 1
	crash_place.roomnum = 90;
	crash_place.location = ROOMS_LOCATION_AT_BOTTOM;
	crash_place.coordinate = 0.0;
	crash_place.damage_amount = 0.01f;
	crash_place.damage_period = 0.01f;
	rooms_crash_places[place_index] = crash_place;
	place_index++;
	size++;
#endif

//	rooms_list_sizes[array_index] = size;
//	array_index++;
	rooms_list_sizes[ROOMS_RETRIBUTION_LEVEL1] = size;

// Retribution level 11
	size = 0;
//	rooms_list_beginnings[array_index] = place_index;
	rooms_list_beginnings[ROOMS_RETRIBUTION_LEVEL11] = place_index;

	crash_place.roomnum = 27;
	crash_place.location = ROOMS_LOCATION_AT_TOP;
	crash_place.coordinate = 0.0;
//	crash_place.damage_amount = 10.0f;
	crash_place.damage_amount = 1.0f;
	crash_place.damage_period = 0.2f;
	rooms_crash_places[place_index] = crash_place;
	place_index++;
	size++;

//	rooms_list_sizes[array_index] = size;
	rooms_list_sizes[ROOMS_RETRIBUTION_LEVEL11] = size;
}

void Rooms_GameStart ()
{
	FILE* f0;
	FILE* f1;
	char name_temp[100];
	char str_temp[100];
	int file_postfix = 0;

	Rooms_Init ();

	if (f0 = fopen (ROOMS_LOG_FILEPATH, "rb"))
// remains from previous session
	{
// find out nonexisting filename
		while (file_postfix < 10000)
		{
			sprintf (name_temp, ROOMS_LOG_DIR"\\"ROOMS_LOG_FILENAME"_%04d."ROOMS_LOG_EXTENSION, file_postfix);

			if (f1 = fopen (name_temp, "rb"))
// such file exist
				fclose (f1);
			else
// stop filename search
				break;

			file_postfix++;
		}

		if (f1 = fopen (name_temp, "wb"))
		{
// copy file content
			str_temp[99] = 0;
			while (fgets (str_temp, 99, f0))
			{
				fputs (str_temp, f1);
			}

			fclose (f1);
		}

// no ! these functions requires write mode
#if 0
#ifdef WIN32
		_chsize (f0->_file, 0);
#endif
#ifdef __unix__
//		fallocate (f0->_file, 0, 0, 0);
		ftruncate (f0->_file, 0);
#endif
#endif

		fclose (f0);

// easier way
//		remove (ROOMS_LOG_FILEPATH);
	}

//	truncate (ROOMS_LOG_FILEPATH, 0);

	if (f0 = fopen (ROOMS_LOG_FILEPATH, "wb"))
	{
		fclose (f0);
	}
}

void Rooms_WriteLogHeader (bool changes)
{
	FILE* f0;
	int i;

	if (! rooms_debug_logging)
		return;

	if (f0 = fopen (ROOMS_LOG_FILEPATH, "wb"))
	{
		fprintf (f0, "%s\r\n", net_game->name);

//		fprintf (f0, "mission \"%s\", level %d\r\n", mission_index == MISSION_NONE ? "(none)" : (mission_index == MISSION_UNKNOWN ? "(unknown)" : mission_names[mission_index]), level_num);
		fprintf (f0, "mission \"%s\" (index %d), level %d\r\n", mission_name, mission_index, level_num);
		for (i = 0; i < DLLMAX_PLAYERS; i++)
		{
// *slow a bit :/
			if (DMFCBase->CheckPlayerNum (i))
//				fprintf (f0, "%d, %s\r\n", rooms_players_rooms[i], players[i].callsign);
				fprintf (f0, "%d, %s\r\n", rooms_players_rooms[i], players[i].callsign);
		}

		if (changes)
// put last info, cause the log sometimes resets after last taking
			fprintf (f0, "(%s)\r\n", string_backup);
		else
// show, that it is the continue
			fprintf (f0, "-----\r\n");

		fclose (f0);
	}
}

// for external use, because the file cleanup at room change is very useful for the debug + gameplay
void Rooms_LogString (char* string)
{
	FILE* f0;

	strncpy (string_backup, string, ROOMS_STRING_SIZE - 1);

	if (! rooms_debug_logging)
		return;

	f0 = fopen (ROOMS_LOG_FILEPATH, "rb");
	if (f0)
	{
	int file_length;

#ifdef WIN32
		file_length = _filelength (f0->_file);
#endif
#ifdef __unix__
		struct stat statbuf;
// here you perhaps did error Cent ? not 'stat' but 'fstat' ? in this case you have improperly configured C++ compiler warnings, if your compiler does not complies
		stat (fileno (f0), &statbuf);
//		fstat (fileno (f0), &statbuf);
		file_length = statbuf.st_size;
#endif
		fclose (f0);
		if (file_length >= ROOMS_LOG_FILESIZE_MAX)
// reset log
			Rooms_WriteLogHeader (0);
	}

	if (f0 = fopen (ROOMS_LOG_FILEPATH, "ab+"))
	{
		fprintf (f0, "%s\r\n", string);
		fclose (f0);
	}
}

void Rooms_CheckPlayer (int pnum, int roomnum)
{
//	rooms_crash_place_t* crash_places_array;
	rooms_crash_place_t* crash_place = rooms_list;
	int size = rooms_list_size;

	assert(rooms_list);

//	if (! rooms_crash_place)
//		return;

	while (size)
	{
//		DLLAddHUDMessage ("checking the player :  roomnum %d/%d, index %d, begin %d, size %d", roomnum, crash_place->roomnum, rooms_list_index, rooms_list_begin, size);

		if (roomnum == crash_place->roomnum)
		{
//			DLLAddHUDMessage ("roomnum is equal, for the player %d", pnum);
			CoopMod_PlayerInProhibitedRoom (pnum, roomnum, crash_place->location, crash_place->damage_amount, crash_place->damage_period);
		}

		crash_place++;
		size--;
	}
}

void Rooms_DefineLevel ()
{
	rooms_list_index = 0;
	rooms_list_size = 0;

	switch (mission_index)
	{
	case MISSION_RETRIBUTION:
		switch (level_num)
		{
		case 1:
			rooms_list_index = ROOMS_RETRIBUTION_LEVEL1;
			break;
		case 11:
			rooms_list_index = ROOMS_RETRIBUTION_LEVEL11;
			break;
		case 12:
			rooms_list_index = ROOMS_RETRIBUTION_LEVEL12;
			break;
		case 17:
			rooms_list_index = ROOMS_RETRIBUTION_LEVEL17;
			break;
		}
		break;
	}

	rooms_list_begin = rooms_list_beginnings[rooms_list_index];
	rooms_list_size = rooms_list_sizes[rooms_list_index];
	rooms_list = &rooms_crash_places[rooms_list_begin];
}

void Rooms_OnInterval ()
{
	int i;
	int object_index;
	object* object_ptr;
	int roomnum;
	bool are_changes = 0;

// define mission and level
	Rooms_DefineLevel ();

	for (i = 0; i < DLLMAX_PLAYERS; i++)
	{
// slow :/
		if (! DMFCBase->CheckPlayerNum (i))
			continue;

		object_index = players[i].objnum;
		object_ptr = &objects[object_index];
		roomnum = object_ptr->roomnum;

		if (rooms_anticrash)
			Rooms_CheckPlayer (i, roomnum);

		if (rooms_players_rooms[i] != roomnum)
		{
			rooms_players_rooms[i] = roomnum;
			are_changes = 1;
		}
	}

	if (are_changes)
		Rooms_WriteLogHeader (1);
}

void Rooms_GameFinish ()
{
	FILE* f0;

	if (f0 = fopen (ROOMS_LOG_FILEPATH, "rb"))
	{
		fclose (f0);

// unnecessary, because game finishes without a crash
		remove (ROOMS_LOG_FILEPATH);
	}
}

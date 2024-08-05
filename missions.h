#pragma once


enum
{
	MISSION_NONE,
	MISSION_UNKNOWN,
	MISSION_RETRIBUTION,
	MISSION_MERCENARY,
	MISSION_OPERATION_MOON,
	MISSION_EVOLUTION21,
	MISSION_WINDMINE_1_2,
	MISSION_WINDMINE_3,
	MISSION_WINDMINE_4,
	MISSION_WINDMINE_5,
	MISSION_WINDMINE_6,
	MISSION_N4_INSTALLATION,
	MISSION_ONYX_SECRET_BASE,
	MISSIONS_NUM,
};

char* const mission_names[MISSIONS_NUM] =
{
	"",
	"",
	"Descent 3: Retribution",
	"Descent 3: Mercenary",
	"Operation: Moon",
	"Evolution V.2.1",
	"WindMine",
	"WindMine III StarGate",
	"WindMine IV Secret Planet",
	"Hidden Worlds",
	"DefCon1",
	"N-4 Installation 1.1",
	"- Onyx Secret Base -",
};

extern const char* mission_name;
extern int mission_index;
extern int level_num;


void Missions_Recognize ();

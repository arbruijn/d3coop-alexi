// code for recognizing individual missions

#include "missions.h"
#include "coop.h"
#include "assert.h"


const tMission* t_mission;

const char* mission_name = 0;
int mission_index = MISSION_NONE;
int level_num;


void Missions_Recognize ()
{
	int i;

	t_mission = DMFCBase->GetCurrentMission ();
	assert(t_mission);
	mission_name = t_mission->name;
	assert(mission_name);
	mission_index = MISSION_UNKNOWN;
	for (i = 0; i < MISSIONS_NUM; i++)
	{
		if (! strcmp (mission_name, mission_names[i]))
		{
			mission_index = i;
			break;
		}
	}
	level_num = t_mission->cur_level;
}


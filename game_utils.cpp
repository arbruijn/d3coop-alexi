// by Alexander Ilyin

#include "game_utils.h"
#include "coop.h"
#include "coop_mod.h"
//#include "macros.h"
#if 0
#undef rad2
#undef __OSIRIS_IMPORT_H_
#include "../osiris/osiris_vector.h"
#endif


u32 GameUtils_CountPlayers ()
{
	u32 i;
	u32 num;

// count players number
	num = 0;
	for (i = 0; i < DLLMAX_PLAYERS; i++)
	{
// *two versions of 'DMFCBase->IsPlayerDedicatedServer()' do exist, one requires 'player_record'
		if (DMFCBase->IsPlayerDedicatedServer (i))
			continue;
		num += DMFCBase->CheckPlayerNum (i);
	}
	return num;
}

// *these two should actually count/find only enemy robots, but I will not bother with it now

u32 GameUtils_CountObjects_Type (u32 type)
{
	u32 i;
	u32 count;

	count = 0;
	for (i = 0; i < MAX_OBJECTS; i++)
	{
		if (objects[i].type == type)
			count++;
	}
	return count;
}

u32 GameUtils_FindNearestObject (u32 pnum, u32 type)
{
	u32 i;
	u32 ix;
//	vector delta;
	double len;
	double len_min;
	object* obj;
	object* obj_me;

// some big value
	len_min = 1024 * 1024;
	ix = -1;
	if (! player_info_ptrs[pnum])
		goto finish;
	obj_me = player_info_ptrs[pnum]->object_ptr;
// or this
#if 0
	r = CoopMod_TeammateStatus_Get (&player_info, pnum, "GameUtils_FindNearestObject");
	if (! r)
		return;
#endif
	if (! obj_me)
		goto finish;
	for (i = 0; i < MAX_OBJECTS; i++)
	{
		obj = &objects[i];
// skip objects of undesired type
		if (type != -1)
			if (obj->type != type)
				continue;
		if (obj == obj_me)
			continue;
//		delta = obj->pos - obj_me->pos;
// stupid, these functions already are defined in "dallas.cpp", so can't use them again
//		len = vm_VectorDistance (&obj->pos, &obj_me->pos);
		len = DLLvm_VectorDistance (&obj->pos, &obj_me->pos);
		if (len_min > len)
		{
			len_min = len;
			ix = i;
		}
	}
finish:
	return ix;
}


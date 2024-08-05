//

#include "inventory.h"
#include "idmfc.h"
#include "macros.h"


// could actually use 'Inven_CheckItem()' instead; anyway ...
bool Inventory::CheckItem (int type, int id)
{
//// unimplemented
//	Assert(id != -1);
	inven_item* item;

	item = root;
	FORI0(count)
	{
// no, 'count' is not for the 'root' index
//		item = &root[i];
//		if (id == -1)
//			goto check_by_handle;
// *'inven_item' also supports 'id == -1' thing for the objhandle
// so there is no any special code for the 'id == -1' case
// look 'INVF_OBJECT' also
		if (item->type == type && item->id == id)
			return 1;
// some combination of these values should be
//		item->type = item->type;
//		item->otype = item->otype;
//		item->id = item->id;
//		item->oid = item->oid;
//		item->prev = item->prev;
//		item->next = item->next;
// next item
		item = item->next;
		continue;
#if 0
check_by_handle:
// *there is no handle
		Assert(0);
//		if (item->handle == type)
//			return 1;
#endif
	}
	return 0;
}


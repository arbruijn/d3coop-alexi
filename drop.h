#pragma once

#include "types.h"


extern bool drop;
extern bool drop_weapon;


#if 0
void Drop_Shield ();
void Drop_Energy ();
#endif
void Drop_SetAmmo (u32 pnum, u32 weapon_ix, ushort amount_ammo);
#if 0
void Drop_WeaponPrimary ();
void Drop_WeaponSecondary ();
void Drop_AmmoPrimary ();
void Drop_AmmoSecondary ();
void Drop_Enhancement ();
#endif
//bool Drop_Dispatcher (int pnum, const char* command);
void Drop_Init ();
bool Drop_Packet_Dispatcher (int pnum, u32 cmd);
//void Drop_ClientWeaponDiminishAmmo (u32 weapon_ix);
void Drop_ClientWeapon_SetAmmo (u32 weapon_ix, ushort amount);
void Drop_CountAmmo (u32 pnum, u32 amount);


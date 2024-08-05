#pragma once

#include "types.h"


void CmdShow_Version (u32 pnum);
void CmdShow_Sleep (u32 pnum);
void CmdShow_Difficulty (u32 pnum);
//void CmdShow_Settings (u32 pnum);
void CmdShow_Logging (u32 pnum);
void CmdShow_ObjectInfo (u32 pnum, u32 obj_ix);
void CmdShow_ObjectShield (u32 pnum, u32 obj_ix);
void CmdShow_PlayerShield (u32 pnum, u32 pnum_he);
void CmdShow_OwnShield (u32 pnum);
void CmdShow_ShowRobotsNum (u32 pnum);
void CmdShow_ObjectShowBeam (u32 pnum, u32 obj_ix);
void CmdShow_ShowRobotNearest (u32 pnum);
void CmdShow_PPS (u32 pnum);


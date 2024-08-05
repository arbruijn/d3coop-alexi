#pragma once

#include "types.h"
#include <string>
//#include <sstream>


typedef void cmd_funct_cmd_t (u32 pnum);
typedef void cmd_funct_uint_t (u32 pnum, u32 val);
typedef void cmd_funct_uint_uint_t (u32 pnum, u32 val0, u32 val1);

enum
{
	CMD_TYPE__NONE,
// no args
	CMD_TYPE__CMD,
	CMD_TYPE__UINT,
	CMD_TYPE__UINT_UINT,
	CMD_TYPES_NUM
};

struct cmd_t
{
// *not in use
	u32 type;
//	char name[16];
	std::string name;
	std::string description;
	void* func;
};

#if 0
// *move out from here
bool Strlcpy (char* dest, const char* src, u32 buffer_size);
{
	;
}
#define STRCPY(_out_, _in_) Strlcpy (_out_, _in_, sizeof(_out_))
#endif


template <u32 _type, typename _Func_Type>
// it is funny to see how MSVC 6.0 is spoiled with a default parameters; can't set them to 0
void TCmd_Add (const char* name, _Func_Type* func_ptr = 0, const char* description = 0);
#define CMD_ADD_CMD TCmd_Add<CMD_TYPE__CMD, cmd_funct_cmd_t>
#define CMD_ADD_UINT TCmd_Add<CMD_TYPE__UINT, cmd_funct_uint_t>
#define CMD_ADD_UINT_UINT TCmd_Add<CMD_TYPE__UINT_UINT, cmd_funct_uint_uint_t>

//void Cmd_PushBack (u32 type, const char* name, void* func_ptr);
void Cmd_Init ();
bool Cmd_Dispatch (u32 plix, const char* str);


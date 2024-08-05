#pragma once

#include "types.h"
#include <vector>
#include <windows.h>
//#include <imagehlp.h>


enum
{
	PATCH_TYPE__NONE,
// raw 4 bytes absolute address
	PATCH_TYPE__RAW_ADDRESS,
// rewrite some code with the 5 bytes e9 jmp
	PATCH_TYPE__REWRITTING_JMP_E9,
// modify a 5 bytes e8 call
	PATCH_TYPE__CALL_E8,
// modify a 5 bytes e9 jump
	PATCH_TYPE__JMP_E9,
// modify a 6 bytes 0f84 je
	PATCH_TYPE__LONG_JE,
// call []
	PATCH_TYPE__CALL_FF15,
	PATCH_TYPE_NUM
};

//#define PATCH_LENGTH_MAX 16

struct patch_t
{
	u8* address;
	u32 len;
// give some and align it
	u8 vals[16 - (sizeof(u32) + sizeof(u8*))];
};

typedef std::vector<patch_t> vec_patch_t;

class Patches
{
public:
//	void Init ();
//	void Quit ();
	bool Unpatch (patch_t* patch);
	void UnpatchAll ();
	void Clear ();
	bool Apply (patch_t* patch);
	bool Apply (u32 type, u8* address, u8* value, u8*& address_return_to);

private:
	patch_t m_org;
//public:
//	HANDLE m_process_handle;
//	static HANDLE m_process_handle;
public:
	vec_patch_t m_original_backup;
};

extern Patches patches;


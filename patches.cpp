// executable's patches storage and the patcher itself
// original belongs at Descent 3 Coop Mod project
// note :  addresses are 'u8*' and not 'void*' or 'ulong' or 'uint' or 'u32', this is because it is convenient in calculations and has size of an address at the same time

// incorrect a bit; should be :
// kb127904
// wait, this kb is for self modify; but it are we; I am lost ! :P
//   1. Call VirtualProtect() on the code pages you want to modify, with the PAGE_WRITECOPY protection.
//   2. Modify the code pages.
//   3. Call VirtualProtect() on the modified code pages, with the PAGE_EXECUTE protection.
//   4. Call FlushInstructionCache(). 

// and looks like there is no need to adjust SE_DEBUG_NAME token on MSVC 2008, because we don't call 'OpenProcess()' (and 'VirtualProtectEx()' accordingly)

#include "patches.h"
#include "error.h"


// instantiate us
Patches patches;

// no need; imo, 'WriteProcessMemory ()' already does everything
#define NEW_GARBAGE 0
//#define NEW_GARBAGE 1

//Patches patches_dummy;
//HANDLE Patches::m_process_handle = 0;


// *'m_process_handle' is not needed anymore
#if 0
//void Patches::Init ()
void Patches::InitStatic ()
{
	DWORD id;

//	Assert(m_original_backup.empty());
// *seems this function cannot fail
	id = GetCurrentProcessId ();
	m_process_handle = OpenProcess (PROCESS_ALL_ACCESS, 0, id);
	if (! m_process_handle)
	{
		Error ("Patches::Init () :  'OpenProcess ()' fail");
		return;
	}
}

//void Patches::Quit ()
void Patches::QuitStatic ()
{
	BOOL R;

	if (m_process_handle)
	{
		R = CloseHandle (m_process_handle);
		m_process_handle = 0;
	}
}
#endif

bool Patches::Unpatch (patch_t* patch)
{
	DWORD num_bytes = -1;
	BOOL R;
	u8* address;
	u32 len;
	u8* vals;
	DWORD D;
	DWORD protect_old;

	address = patch->address;
	len = patch->len;
	vals = patch->vals;

	SetLastError (0);
	R = VirtualProtect ((void*)address, len, PAGE_EXECUTE_READWRITE, &protect_old);
	if (! R)
	{
		D = GetLastError ();
		ErrorReport ("VirtualProtect (%08x, %08x, readwrite) has failed, error %u", address, len, D);
		return 0;
	}

#if 0
	R = WriteProcessMemory (m_process_handle, (void*)address, vals, len, &num_bytes);
	if (! R)
	{
		ErrorReport ("Patches::Quit () :  WriteProcessMemory (%08x, %08x) has failed", address, len);
		return;
	}
	if (num_bytes != len)
	{
		ErrorReport ("Patches::Quit () :  WriteProcessMemory (%08x, %08x) has returned %08x bytes written", address, len, num_bytes);
		return;
	}
#else
	memcpy ((void*)address, vals, len);
#endif

	SetLastError (0);
	R = VirtualProtect ((void*)address, len, protect_old, &D);
	if (! R)
	{
		D = GetLastError ();
		ErrorReport ("VirtualProtectEx (%08x, %08x, old) has failed, error %u", address, len, D);
		return 0;
	}

// *possibly it is not necessary; I would expect that 'VirtualProtect()' already has flushed the cache
	HANDLE h;
	h = GetCurrentProcess ();
//	R = FlushInstructionCache (h, (void*)address, num_bytes);
	R = FlushInstructionCache (h, (void*)address, len);
//	A(R);
	if (! R)
	{
		DWORD D;
		D = GetLastError ();
		D = D;
		A(0);
//		ET;
	}

	return 1;
}

// nicely cleanup after ourselves, to allow the exe be patched again (or not patched, if another netgame type will be)
void Patches::UnpatchAll ()
{
	patch_t* patch;

	while (! m_original_backup.empty())
	{
		patch = &m_original_backup.back();
		Unpatch (patch);
		m_original_backup.pop_back();
	}
}

// drop the content
// call this instead of 'UnpatchAll()', in the case if dll was already unloaded
void Patches::Clear ()
{
	m_original_backup.clear();
}

// somewhere do change some bytes
// this subroutine could actually be broken onto two parts - reading and writing; but I perhaps will leave it as is
bool Patches::Apply (patch_t* patch)
{
//	DWORD num_bytes = -1;
	BOOL R;
//	u8* vals;
	u8* address;
	u32 len;
	DWORD D;
	DWORD protect_old;
//	MEMORY_BASIC_INFORMATION mbi;

	address = patch->address;
	len = patch->len;
	if (len > sizeof(patch->vals))
	{
		ErrorReport ("Patches::Apply (%08x, %08x) - too long length", address, len);
		return 0;
	}

// *not sure about VirtualQuery, maybe it isn't necessary
	SetLastError (0);

// *can also check page (pages, if be more correct) with VirtualQueryEx() first, but it is an extra code
// we are hacking, rather than writing a fully functional code (its misfunctionality only needed to be defined at a new injections test time)
#if 0
	num_bytes = VirtualQueryEx (m_process_handle, (void*)address, &mbi, sizeof(mbi));
	if (num_bytes != sizeof(mbi))
	{
		D = GetLastError ();
		ErrorReport ("VirtualQueryEx (%08x) has returned %u instead of %u, error %u", address, num_bytes, sizeof(mbi), D);
		return 0;
	}
#endif

// *basically not needing this, but vtab however cannot be patched with just 'WriteProcessMemory()' (error 998 ERROR_NOACCESS)
	SetLastError (0);
// "All pages in the specified region must be within the same reserved region allocated when calling the VirtualAlloc or VirtualAllocEx function using MEM_RESERVE."
// "The pages cannot span adjacent reserved regions that were allocated by separate calls to VirtualAlloc or VirtualAllocEx using MEM_RESERVE."
//	R = VirtualProtectEx (m_process_handle, (void*)address, len, PAGE_EXECUTE_READWRITE, &protect_old);
	R = VirtualProtect ((void*)address, len, PAGE_EXECUTE_READWRITE, &protect_old);
	if (! R)
	{
		D = GetLastError ();
//		ErrorReport ("VirtualProtectEx (%08x, %08x, readwrite) has failed, error %u", address, len, D);
		ErrorReport ("VirtualProtect (%08x, %08x, readwrite) has failed, error %u", address, len, D);
		return 0;
	}

	m_org.address = address;
	m_org.len = len;
#if 0
	SetLastError (0);
	R = ReadProcessMemory (m_process_handle, (void*)address, m_org.vals, len, &num_bytes);
	if (! R)
	{
		D = GetLastError ();
		ErrorReport ("ReadProcessMemory (%08x, %08x) has failed, error %u", address, len, D);
		return 0;
	}
	if (num_bytes != len)
	{
		ErrorReport ("ReadProcessMemory (%08x, %08x) has returned %08x bytes written", address, len, num_bytes);
		return 0;
	}
#else
	memcpy (m_org.vals, (void*)address, len);
#endif

#if 0
	SetLastError (0);
	R = WriteProcessMemory (m_process_handle, (void*)address, patch->vals, len, &num_bytes);
	if (! R)
	{
		D = GetLastError ();
		ErrorReport ("WriteProcessMemory (%08x, %08x) has failed, error %u", address, len, D);
		return 0;
	}
	if (num_bytes != len)
	{
		ErrorReport ("WriteProcessMemory (%08x, %08x) has returned %08x bytes written", address, len, num_bytes);
		return 0;
	}
#else
	memcpy ((void*)address, patch->vals, len);
#endif

	SetLastError (0);
//	R = VirtualProtectEx (m_process_handle, (void*)address, len, protect_old, &D);
	R = VirtualProtect ((void*)address, len, protect_old, &D);
	if (! R)
	{
		D = GetLastError ();
		ErrorReport ("VirtualProtectEx (%08x, %08x, old) has failed, error %u", address, len, D);
		return 0;
	}

	HANDLE h;
// *it only calls 'NtCurrentProcess()' (and gets the pseudohandle this way)
	h = GetCurrentProcess ();
// "Applications should call FlushInstructionCache if they generate or modify code in memory."
// "The CPU cannot detect the change, and may execute the old code it cached."
	R = FlushInstructionCache (h, (void*)address, len);
	A(R);

	m_original_backup.push_back(m_org);

	return 1;
}

// make a smart patch, with any necessary calculations and checks
// address - address, where to do the patch
// value - new address for the jump
//// original_value - something, which there was earlier (ordinary it is an address)
// address_original_value - address, which there was earlier
bool Patches::Apply (u32 type, u8* address, u8* value, u8*& address_original_value)
{
	bool result;
	patch_t patch;
	u8* address_next;
	u8 val;
	u8 op_byte = 0;

	switch (type)
	{
	case PATCH_TYPE__RAW_ADDRESS:
		patch.address = address;
		patch.len = 4;
		*(u8**)patch.vals = value;
		result = Apply (&patch);
		if (! result)
// *an error should already be reported
			return 0;
		address_original_value = *(u8**)m_org.vals;
		break;

	case PATCH_TYPE__REWRITTING_JMP_E9:
		address_next = address + 5;
		patch.address = address;
		patch.len = 5;
		patch.vals[0] = 0xe9;
		*(u32*)&patch.vals[1] = value - address_next;
		result = Apply (&patch);
		if (! result)
			return 0;
// there is no any original address; a user should know upon what stuff he is overwritting
		address_original_value = 0;
		break;

// relative transitions
	case PATCH_TYPE__CALL_E8:
		op_byte = 0xe8;
	case PATCH_TYPE__JMP_E9:
		if (! op_byte)
			op_byte = 0xe9;
		val = *(u8*)address;
		if (val != op_byte)
		{
			ErrorReport ("Patches::Apply () :  byte at position %08x is %02x, and is not %02x", address, val, op_byte);
			return 0;
		}
		address++;
		address_next = address + 4;
		patch.address = address;
		patch.len = 4;
		*(u32*)patch.vals = value - address_next;
		result = Apply (&patch);
		if (! result)
			return 0;
		address_original_value = *(u32*)m_org.vals + address_next;
		break;

	case PATCH_TYPE__LONG_JE:
		val = *(u8*)address;
		op_byte = 0x0f;
		if (val != op_byte)
		{
			ErrorReport ("Patches::Apply () :  byte at position %08x is %02x, and is not %02x", address, val, op_byte);
			return 0;
		}
		address++;
		val = *(u8*)address;
		op_byte = 0x84;
		if (val != op_byte)
		{
			ErrorReport ("Patches::Apply () :  byte at position %08x is %02x, and is not %02x", address, val, op_byte);
			return 0;
		}
		address++;
		address_next = address + 4;
		patch.address = address;
		patch.len = 4;
		*(u32*)patch.vals = value - address_next;
		result = Apply (&patch);
		if (! result)
			return 0;
		address_original_value = *(u32*)m_org.vals + address_next;
		break;

	case PATCH_TYPE__CALL_FF15:
		val = ((u8*)address)[0];
		op_byte = 0xff;
		if (val != op_byte)
		{
			ErrorReport ("Patches::Apply () :  byte at position %08x is %02x, and is not %02x", address, val, op_byte);
			return 0;
		}
		val = ((u8*)address)[1];
		op_byte = 0x15;
		if (val != op_byte)
		{
			ErrorReport ("Patches::Apply () :  byte at position %08x is %02x, and is not %02x", address, val, op_byte);
			return 0;
		}
		address_next = address + 5;
		patch.address = address;
// *call is more comfort than jump :  the return address will be stored on stack
//		patch.vals[0] = 0xe9;
		patch.vals[0] = 0xe8;
		*(u32*)&patch.vals[1] = value - address_next;
		patch.vals[5] = 0x90;
		address_next++;
		patch.len = 6;
		result = Apply (&patch);
		if (! result)
			return 0;
		address_original_value = *(u8**)&m_org.vals[2];
		break;

	default:
		ErrorReport ("Patches::Apply () :  wrong type %u", type);
		return 0;
	}

	return 1;
}


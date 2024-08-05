// exceptions handler

#include "exceptions.h"
#ifdef WIN32
#include <windows.h>
#endif
#include <stdio.h>
#include "error.h"
//#include "idmfc.h"
#include "coop_mod.h"
#include "hooks.h"
#include "log_file.h"
//#include <vector>
#include <deque>
//using namespace std;
#include "screen.h"

// I don't think 'ReadProcessMemory ()' is necessary;
// it takes a handle, so means is oriented at any process, while the exception handler here is relative at the own thread, and should always have direct memory access, I think

// MSVC 8.0 needs it
#pragma warning (disable:4733)


// +++ config +++
// by default - off, because should not crash the main exe, instead of doing a cure above it
bool exceptions_catch = 0;
// log floods; have to exclude it how is possible :/
bool exceptions_avoid_repeats = 1;
// --- config ---
// to find out, that it is own exception; those, which can appear in the exception catching routine
bool exceptions_in_exception = 0;
ulong exceptions_address = 0;
// *each access violation fix try malloc currently eats 64KB
#define EXCEPTIONS_PER_FRAME_MAX__ACCESS_VIOLATION 30
#define EXCEPTIONS_PER_FRAME_MAX__FIXED 100
// do not flood the log much
//#define EXCEPTIONS_PER_FRAME_MAX 10
//ulong exceptions_count = 0;
// helper counter, for the case of excessive flow of exceptions
ulong exceptions__counts_per_frame = 0;
// known exceptions
ulong exceptions__counts_per_frame__fixed = 0;
// unknown exceptions; access violation, but maybe is possible to fix by just allocating that region of mem for it
ulong exceptions__counts_per_frame__access_violation = 0;
// + flood detection +
ulong exceptions__counts_flow = 0;
bool exceptions__is_in_flood = 0;
// when to consider, that appears in flood
#define EXCEPTIONS_FLOOD_LEVEL_TRIGGER 150
// to limit time, which will be necessary to go back, to normal mode, so it wouldn't be undefined
#define EXCEPTIONS_FLOOD_LEVEL_OVER (EXCEPTIONS_FLOOD_LEVEL_TRIGGER + 200)
// - flood detection -
char exceptions_filename[1024];
const char exceptions_filename_unknown[] = "-unknown-";
// use this, instead of buffer directly
const char* exceptions_filename_ptr = exceptions_filename_unknown;
u32 exceptions_offset = -1;
bool exceptions_filename_is_defined = 0;
// not in use atm
char* exceptions_modulename = "-not initialized-";
char* exceptions_cause = "-not initialized-";
// + malloc sizes +
// *alloc aligns not to page size; for 0x061fffff 'VirtualAlloc ()' returns 0x061f0000
//#define PAGE_BITS_NUM (12)
//#define ALLOC_ALIGN_BITS_NUM (16)
#if 0
#define ALLOC_ALIGN_SIZE (1 << ALLOC_ALIGN_BITS_NUM)
// to ensure, that a possible next access will not produce an exception also
#define INDENT_FROM_BLOCK_END (ALLOC_ALIGN_SIZE >> 1)
#endif
u32 alloc_align_bits_num = 0;
u32 alloc_align_size = 0;
u32 alloc_indent_from_block_end = 0;
// - malloc sizes -
//char exceptions_server_message[1024] = {0};
//char exceptions_message[1024] = {0};
bool exceptions_say_message = 0;
// for the case if exceptions amount will become big; record then just one per frame
char exceptions_log_text[1024] = {0};
// stack all initial handlers; it is including the ones of the engine
//std::vector<ulong> exceptions_vec_handlers;
std::deque<ulong> exceptions_deq_handlers;
// the one, which will be left; is necessary for comparison check
ulong exceptions_handler_left = 0;
// + stack +
#define EXCEPTIONS_STACK_SIZE (128 * 1024)
ulong exceptions_stack_new = 0;
ulong exceptions_stack_backup = 0;
u32 exceptions_initial_esp = 0;
// - stack -

//static void Exceptions_Handler ();
#ifdef WIN32
HANDLE log_file_handle = 0;
LPTOP_LEVEL_EXCEPTION_FILTER top_level_exception_filter = 0;

static LONG WINAPI TopLevelExceptionFilter (EXCEPTION_POINTERS* exception_pointers);
bool Exceptions_Fix (CONTEXT* context);
bool Exceptions_Fix_AccessViolation (ulong address);
#endif


#define LOG_FILE_NAME "exceptions.log"
static va_list argptr;
#define VA_PRINTF \
	va_start (argptr, format); \
	_vsnprintf (string, sizeof(string), format, argptr); \
	va_end (argptr);


void Exceptions_Log_Print (const char* string)
{
#ifdef WIN32
	log_file_handle = CreateFile (LOG_FILE_NAME, GENERIC_WRITE, FILE_SHARE_READ, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (log_file_handle == INVALID_HANDLE_VALUE) 
//		Error ("Can't open log file.");
		return;

	int i;
	int string_length = strlen (string);
	BOOL B_result;

	DWORD bytes_written = 0;
	DWORD D_result = SetFilePointer (log_file_handle, 0, NULL, FILE_END);
	if (D_result == 0xffffffff)
//		Error ("'LogFile_Print ()' :  'SetFilePointer ()' fails");
		return;

//	B_result = WriteFile (log_file_handle, string, strlen (string), &bytes_written, NULL);
//	if (B_result == 0)
//		Error ("'LogFile_Print ()' :  'WriteFile ()' fails");

// handle line endings
	for (i = 0; i < string_length; i++)
	{
		char character = string[i];

		if (character == '\n')
		{
			B_result = WriteFile (log_file_handle, "\r", 1, &bytes_written, NULL);
			if (B_result == 0)
//				Error ("'LogFile_Print ()' :  'WriteFile ()' fails");
				break;
		}

		B_result = WriteFile (log_file_handle, &character, 1, &bytes_written, NULL);
		if (B_result == 0)
//			Error ("'LogFile_Print ()' :  'WriteFile ()' fails");
			break;
	}

// flush the file buffers; the second time, when I am getting unfinished file, so lets exclude it
// not helps; still strings are partial; I will try own allocated stack
//	B_result = FlushFileBuffers (log_file_handle);

	B_result = CloseHandle (log_file_handle);
#endif
}

void Exceptions_Log_Printf (const char* format, ...)
{
// 256 is not good; my current string sizes are around 200
	static char string[1024];

	VA_PRINTF;

	Exceptions_Log_Print (string);
}

void Exceptions_Error_NoExit (const char* format, ...)
{
	static char string[1024];

	VA_PRINTF;

#ifdef WIN32
// restore screen; now, after a D3 exception handlers were removed, has become necessary
	Screen_Restore ();
	MessageBox (0, string, "Descent 3 Co-op Mod Exception Filter error", MB_OK);
// WinXP restores the window, after the mbox, so have to remove that black window again, if wish to debug further
	Screen_Restore ();
// this should only help, I think
	__asm int 3
#endif
#ifdef __unix__
	printf ("\aDescent 3 Co-op Mod Exception Filter error: %s\n", string);
#endif
}

// get system malloc alignment sizes
void Exceptions_GetMallocSizes ()
{
	ulong test_address;
	void* malloc_area;
	u32 rotor;

	alloc_align_bits_num = 0;

#ifdef WIN32
	BOOL B_result;

	test_address = 0x5fffffff;
	malloc_area = VirtualAlloc ((void*)test_address, 1 << 12, MEM_RESERVE, PAGE_READWRITE);
	B_result = VirtualFree (malloc_area, 0, MEM_RELEASE);
	rotor = test_address - (ulong)malloc_area;

	while (rotor)
	{
		rotor >>= 1;
		alloc_align_bits_num++;
	}
	alloc_align_size = 1 << alloc_align_bits_num;
// give any
	alloc_indent_from_block_end = alloc_align_size >> 1;
#endif
}

__declspec(naked) ulong Exceptions_Handler_Get ()
{
__asm
{
	mov eax,fs:[0]
	ret
}
}

__declspec(naked) ulong Exceptions_Handler_Set (ulong new_address)
{
__asm
{
	mov eax,fs:[0]
	push eax
// uses ebp
//	mov eax,new_address
	mov eax,[esp+8]
	mov fs:[0],eax
	pop eax
	ret
}
}

// *still, not all exceptions go through here; some still are throwing crappy mbox, which even is invisible, because of a spoiled screen
void Exceptions_Init ()
{
	ulong address;
	int size;

	__asm mov [exceptions_initial_esp],esp
	Exceptions_Log_Printf ("----------\n");
#ifdef WIN32
	Exceptions_GetMallocSizes ();
// remove D3's nonsense from D3 :)
#if 0
__asm
{
#if 0
	mov eax,fs:[0]
	test eax,eax
	jz not_rotate
	mov eax,[eax]
	mov fs:[0],eax
not_rotate:
#endif
#if 0
	mov eax,0
	mov eax,0xffffffff
	mov fs:[0],eax
#endif
#if 0
	mov eax,fs:[0]
	mov eax,[eax]
	mov eax,[eax]
	mov eax,[eax]
	mov fs:[0],eax
#endif
// leave only the last handler
// not very good :  in case of crash, the screen now always remains in the current resolution
#if 1
	mov ebx,fs:[0]
	mov eax,ebx
rotate_loop:
	test eax,eax
	jz finish_rotate
	cmp eax,-1
	jz finish_rotate
	mov ebx,eax
	mov eax,[ebx]
	jmp rotate_loop
finish_rotate:
	mov fs:[0],ebx
#endif
}
#endif
	address = Exceptions_Handler_Get ();
	while (1)
	{
		if (! address)
			break;
		if (address == -1)
			break;
		exceptions_deq_handlers.push_front(address);
// get a previous handler
		address = *(ulong*)address;
	}
	size = exceptions_deq_handlers.size();
	if (size)
	{
//		ulong addr;
// leave only the last handler
// not very good :  in case of crash, the screen now always remains in the current resolution
// currently :  leaves two levels from initial five
		if (size == 1)
			address = exceptions_deq_handlers[0];
		else
			address = exceptions_deq_handlers[1];
		Exceptions_Handler_Set (address);
	}
// 'address' will be -1, if size == 0
	exceptions_handler_left = address;
// *this handler works in the thread context, so could be a problem, if for example the stack is corrupted
// as a comment on MSDN is saying :  should take extreme care, when using it from a dll :)
// *restores levels number of the handlers back to five; all have the same addresses
	top_level_exception_filter = SetUnhandledExceptionFilter (&TopLevelExceptionFilter);

	exceptions_stack_backup = 0;
	exceptions_stack_new = (ulong)VirtualAlloc (0, EXCEPTIONS_STACK_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (! exceptions_stack_new)
		Error ("Exceptions_Init () :  VirtualAlloc (%08x) for an exception stack has failed", EXCEPTIONS_STACK_SIZE);
#endif
}

void Exceptions_Quit ()
{
#ifdef WIN32
	BOOL B_result;
//	ulong address;
//	int size;

	B_result = VirtualFree ((void*)exceptions_stack_new, 0, MEM_RELEASE);
// really wrong
#if 0
// *I hope this is a proper way to remove own exception handler from the stack
__asm
{
	mov eax,fs:[0]
	mov eax,[eax]
	mov fs:[0],eax
}
#endif
// return the previous exceptions filter
// *without this, the game crashes, at the moment of second quit of a mission (Arrilen Po here is not relative - it crashes at the second exit itself, independently)
	SetUnhandledExceptionFilter (top_level_exception_filter);
// restore the previous stack of exception handlers also
// no, this stuff doesn't work; the address isn't the same as the one which has been left;
// the idea to restore all previous handlers would require to remember a bit other things, than the structure base addresses, I think
#if 0
	address = Exceptions_Handler_Get ();
	if (address != exceptions_handler_left)
		Error ("Exceptions_Quit () :  current exception handler %08x is not the same as has been left %08x", address, exceptions_handler_left);
	size = exceptions_deq_handlers.size();
	if (size)
	{
// take the upper, which at the mod initialization time was the current
		address = exceptions_deq_handlers.back();
		Exceptions_Handler_Set (address);
	}
#endif
// for the case, if dll will not be unloaded
	exceptions_deq_handlers.clear();
#endif
}

void Exceptions_Frame ()
{
// it currently is for exceptions flood mode only
	if (exceptions_log_text[0])
	{
// no, this is possible to understand wrong
//		Exceptions_Log_Printf ("(%u) %s", exceptions__counts_per_frame, exceptions_log_text);
// still isn't very clear
// currently, no separator appears after that
		Exceptions_Log_Printf ("%u, %s", exceptions__counts_per_frame, exceptions_log_text);
		exceptions_log_text[0] = 0;
	}

	exceptions__counts_per_frame = 0;
	exceptions__counts_per_frame__access_violation = 0;
	exceptions__counts_per_frame__fixed = 0;

// theme of the stuff which is below this condition a bit differs; but currently it not pains
#if 0
	if (exceptions_message[0])
	{
		exceptions_message[0] = 0;
#else
	if (exceptions_say_message)
	{
		exceptions_say_message = 0;
#endif
#if 0
		CoopMod_SendMessage (-1, "%s", exceptions_message);
#else
		if (local_role_is_server)
			CoopMod_SendMessage (-1, "server :  handled exception at %08x", exceptions_address);
		else
			DLLAddHUDMessage ("client :  handled exception at %08x", exceptions_address);
#endif
// important thing :)
// with this, it not always is possible to define an exceptions chains; but without this - is always impossible 
		Exceptions_Log_Printf ("---\n");
	}

	if (exceptions__counts_flow)
	{
		bool is_in_flood;
		is_in_flood = exceptions__counts_flow >= EXCEPTIONS_FLOOD_LEVEL_TRIGGER;
// inform, if mode has changed
		if (exceptions__is_in_flood != is_in_flood)
		{
			static char string[256];
			exceptions__is_in_flood = is_in_flood;
			_snprintf (string, sizeof(string) - 1, "flood of exceptions has %s", is_in_flood ? "began" : "over");
			if (local_role_is_server)
				CoopMod_SendMessage (-1, "server :  %s", string);
			else
				DLLAddHUDMessage ("client :  %s", string);
//// no, don't record it
			Exceptions_Log_Printf ("* %s", string);
		}
		exceptions__counts_flow--;
	}
}

bool Exceptions_IsRepeat (ulong address)
{
#define EXCEPTIONS_REPEATS_REMEMBER 10
	static ulong arr[EXCEPTIONS_REPEATS_REMEMBER] = {0};
	u32 i;
	ulong a;

	i = 0;
	while (a = arr[i])
	{
		if (a == address)
			return 1;
		i++;
		if (i >= EXCEPTIONS_REPEATS_REMEMBER)
			break;
	}
//	i = EXCEPTIONS_REPEATS_REMEMBER - 2;
//// rotate all addresses
// rotate addresses
	while (i)
	{
		i--;
		arr[i + 1] = arr[i];
//		if (! i)
//			break;
	};
// put on the list head
	arr[0] = address;
	return 0;
}

#if 0
__declspec(naked) void Exceptions_Handler ()
{
}
#endif

#ifdef WIN32
// this one should be called, after stack is done safe
// *don't call 'DLLAddHUDMessage ()' from here :)
LONG WINAPI TopLevelExceptionFilter_StackSafe (EXCEPTION_POINTERS* exception_pointers)
{
	bool r;
	static char text[512];
	bool fixed = 0;
	EXCEPTION_RECORD* exception_record;
	CONTEXT* exception_context;
	ulong* exception_information;
	ulong violation_address;
	LONG ret;

// give whatever
	ret = EXCEPTION_CONTINUE_SEARCH;

// it does :  dead loop of some tables
//	__asm int 3
// bad, uses standard search order; perhaps means dead loop
// actually :  some table; I am pressing "enter", and then the process terminates without anything more appearing
//	DebugBreak ();
//	EFaultRepRetVal e_fault_rep_ret_val;
//	e_fault_rep_ret_val = ReportFault (exception_pointers, 0);
// same as 'DebugBreak ()' - just one mbox, and nothing after it
//	__asm int 1

	if (! exceptions_catch)
		return EXCEPTION_CONTINUE_SEARCH;

// possibly not everything will succeed to get, so cleanup it
	exceptions_address = -1;
// **hmmm, uninitialized forever; what this value actually means ?
	exceptions_modulename = "-uninitialized-";
	exceptions_filename[0] = 0;
	exceptions_offset = -1;
	exceptions_cause = "-uninitialized-";
	exception_context = exception_pointers->ContextRecord;
	if (! exception_context)
		goto finish;
	exceptions_address = exception_context->Eip;

	exceptions__counts_per_frame++;
	if (exceptions__counts_flow < EXCEPTIONS_FLOOD_LEVEL_OVER)
		exceptions__counts_flow++;

	DWORD D_result;
	ulong image_base;

	MEMORY_BASIC_INFORMATION memory_basic_information;
	D_result = VirtualQuery ((VOID*)exceptions_address, &memory_basic_information, sizeof(memory_basic_information));
	if (! D_result)
	{
		exceptions_filename_ptr = 0;
		strcpy (exceptions_filename, "-unknown-");
//		ChangeDisplaySettings (0, 0);
//		sprintf (text, "VirtualQuery () fail, for the exception at %08x", exceptions_address);
//		MessageBox (0, text, "Exception Filter", MB_OK);
		Exceptions_Error_NoExit ("VirtualQuery () fail, for the exception at %08x", exceptions_address);
		goto finish;
	}

// it perhaps is section base, not image base
//	image_base = (ulong)memory_basic_information.BaseAddress;
	image_base = (ulong)memory_basic_information.AllocationBase;
	exceptions_offset = exceptions_address - image_base;

	D_result = GetModuleFileName ((HMODULE)image_base, exceptions_filename, sizeof(exceptions_filename));

	exceptions_filename_is_defined = !! D_result;

	if (! exceptions_filename_is_defined)
	{
		exceptions_filename_ptr = 0;
// why removing filename ?
//		strcpy (exceptions_filename, "-unknown-");
//		ChangeDisplaySettings (0, 0);
//		sprintf (text, "can't get module name for the exception at %08x", exceptions_address);
//		MessageBox (0, text, "Exception Filter", MB_OK);
		Exceptions_Error_NoExit ("can't get module name for the exception at %08x", exceptions_address);
		goto finish;
	}

	exceptions_filename_ptr = exceptions_filename;

	exception_record = exception_pointers->ExceptionRecord;
	if (! exception_record)
		goto finish;
	if (exception_record->ExceptionFlags & EXCEPTION_NONCONTINUABLE)
		goto finish;

	fixed = Exceptions_Fix (exception_pointers->ContextRecord);
	if (fixed)
	{
		exceptions__counts_per_frame__fixed++;
// condition for that actually is necessary only for a fixed exceptions; others can not repeat so much or can not repeat at all
		if (exceptions__is_in_flood)
		{
// don't update; printed should be just a first
			if (! exceptions_log_text[0])
				_snprintf (exceptions_log_text, sizeof(exceptions_log_text) - 1, "+ exception at %08x, module \"%s\", \"%s\", offset %08x, region size %08x, cause %s\n", exceptions_address, exceptions_modulename, exceptions_filename, exceptions_offset, memory_basic_information.RegionSize, exceptions_cause);
			goto finish;
		}
// avoid of repeats
		r = exceptions_avoid_repeats ? Exceptions_IsRepeat (exceptions_offset) : 0;
		if (! r)
		{
// the cause is necessary mandatory
//			Exceptions_Log_Printf ("+ exception at %08x, module \"%s\", \"%s\", offset %08x, region size %08x\n", exceptions_address, exceptions_modulename, exceptions_filename, exceptions_offset, memory_basic_information.RegionSize);
			Exceptions_Log_Printf ("+ exception at %08x, module \"%s\", \"%s\", offset %08x, region size %08x, cause %s\n", exceptions_address, exceptions_modulename, exceptions_filename, exceptions_offset, memory_basic_information.RegionSize, exceptions_cause);
//			_snprintf (exceptions_message, sizeof(exceptions_message) - 1, "server :  handled exception at %08x", exceptions_address);
			exceptions_say_message = 1;
		}
//		goto continue_execution;
		goto finish;
	}

	if (exception_record->ExceptionCode != EXCEPTION_ACCESS_VIOLATION)
		goto finish;
	exception_information = exception_record->ExceptionInformation;
	if (! exception_information)
		goto finish;
	violation_address = exception_information[1];
	fixed = Exceptions_Fix_AccessViolation (violation_address);
	if (fixed)
	{
		exceptions__counts_per_frame__access_violation++;
// the cause is necessary mandatory
//		Exceptions_Log_Printf ("+ exception at %08x, module \"%s\", \"%s\", offset %08x, region size %08x\n", exceptions_address, exceptions_modulename, exceptions_filename, exceptions_offset, memory_basic_information.RegionSize);
//		Exceptions_Log_Printf ("+ exception at %08x, module \"%s\", \"%s\", offset %08x, region size %08x, cause %s\n", exceptions_address, exceptions_modulename, exceptions_filename, exceptions_offset, memory_basic_information.RegionSize, exceptions_cause);
// '+' means fixed, '-' means don't know how to fix, and here is "not sure"
//		Exceptions_Log_Printf (". exception at %08x, module \"%s\", \"%s\", offset %08x, region size %08x, access violation at %08x\n", exceptions_address, exceptions_modulename, exceptions_filename, exceptions_offset, memory_basic_information.RegionSize, violation_address);
		Exceptions_Log_Printf (". exception at %08x, \"%s\", offset %08x, region size %08x, access violation at %08x\n", exceptions_address, exceptions_filename, exceptions_offset, memory_basic_information.RegionSize, violation_address);
//		_snprintf (exceptions_message, sizeof(exceptions_message) - 1, "server :  access violation exception; addressses :  IP %08x, mem %08x", exceptions_address, violation_address);
		exceptions_say_message = 1;
//		goto continue_execution;
		goto finish;
	}

//noncontinuable:
finish:
//continue_execution:
// *not logging for a client atm
	LogFile_Chat_Printf ("* %c%s exception", fixed ? '+' : '-', local_role_name);

	if (exceptions__counts_per_frame__access_violation > EXCEPTIONS_PER_FRAME_MAX__ACCESS_VIOLATION)
	{
		Exceptions_Log_Printf ("* exceptions overflow; %u access violations per frame is max\n", EXCEPTIONS_PER_FRAME_MAX__ACCESS_VIOLATION);
//		ChangeDisplaySettings (0, 0);
//		sprintf (text, "exceptions overflow; %u per frame is max.\nlast exception at %08x", EXCEPTIONS_PER_FRAME_MAX, exceptions_address);
//		MessageBox (0, text, "Exception Filter", MB_OK);
		Exceptions_Error_NoExit ("exceptions overflow; %u access violations per frame is max.\nlast exception at %08x", EXCEPTIONS_PER_FRAME_MAX__ACCESS_VIOLATION, exceptions_address);
// don't display the next messagebox, with the exception info
		return EXCEPTION_CONTINUE_SEARCH;
	}
	if (exceptions__counts_per_frame__fixed > EXCEPTIONS_PER_FRAME_MAX__FIXED)
	{
		Exceptions_Log_Printf ("* exceptions overflow; %u fixed per frame is max\n", EXCEPTIONS_PER_FRAME_MAX__FIXED);
		Exceptions_Error_NoExit ("exceptions overflow; %u fixed per frame is max.\nlast exception at %08x", EXCEPTIONS_PER_FRAME_MAX__FIXED, exceptions_address);
// don't display the next messagebox, with the exception info
		return EXCEPTION_CONTINUE_SEARCH;
	}

	if (fixed)
	{
// test
//		Exceptions_Log_Printf ("+ continuing execution\n");
//		Exceptions_Error_NoExit ("fixed exception at %08x", exceptions_address);
		ret = EXCEPTION_CONTINUE_EXECUTION;
	}
	else
	{
//		Exceptions_Log_Printf ("- exception at %08x, module \"%s\", \"%s\", offset %08x, region size %08x\n", exceptions_address, exceptions_modulename, exceptions_filename, exceptions_offset, memory_basic_information.RegionSize);
		Exceptions_Log_Printf ("- exception at %08x, \"%s\", offset %08x, region size %08x\n", exceptions_address, exceptions_filename, exceptions_offset, memory_basic_information.RegionSize);
// restore screen; now, after a D3 exception handlers were removed, has become necessary
//		ChangeDisplaySettings (0, 0);
//		sprintf (text, "exception at %08x\nmodule \"%s\"\n\"%s\"\noffset %08x\nregion size %08x\n\ncannot be fixed.", exceptions_address, exceptions_modulename, exceptions_filename, exceptions_offset, memory_basic_information.RegionSize);
//		MessageBox (0, text, "Exception Filter", MB_OK);
		Exceptions_Error_NoExit ("exception at %08x\nmodule \"%s\"\n\"%s\"\noffset %08x\nregion size %08x\n\ncannot be fixed.", exceptions_address, exceptions_modulename, exceptions_filename, exceptions_offset, memory_basic_information.RegionSize);
//		return EXCEPTION_EXECUTE_HANDLER;
		ret = EXCEPTION_CONTINUE_SEARCH;
	}

	return ret;
}

// one more function, to let to maximally diminish of a possible stack overflowing
LONG WINAPI TopLevelExceptionFilter (EXCEPTION_POINTERS* exception_pointers)
{
// away from stack
	static LONG l;
	static CONTEXT* exception_context;
	static ulong exceptions_address;

	if (! exceptions_stack_new)
		return EXCEPTION_CONTINUE_SEARCH;
//	Assert(! exceptions_stack_backup);
	if (exceptions_in_exception)
	{
		exception_context = exception_pointers->ContextRecord;
		if (! exception_context)
			goto error_mbox;
		exceptions_address = exception_context->Eip;
		Exceptions_Log_Printf ("----- exception in the exception handler; address %08x -----\n", exceptions_address);
error_mbox:
// this possibly has happened because of stack, so rewind the stack to some safe value
		__asm mov esp,[exceptions_initial_esp]
//		Error ("exception in the exception handler; have a nice day :)");
		Error_NoExit ("exception in the exception handler; have a nice day :)");
		ExitProcess (1);
	}
	exceptions_in_exception = 1;
	__asm
	{
		mov exceptions_stack_backup,esp
		mov esp,exceptions_stack_new
		add esp,EXCEPTIONS_STACK_SIZE
		push 0x0badbeef
	}
	l = TopLevelExceptionFilter_StackSafe (exception_pointers);
	__asm
	{
		mov esp,exceptions_stack_backup
		mov exceptions_stack_backup,0
	}
	exceptions_in_exception = 0;
	return l;
}

// +++++ unimplemented +++++

// Mission at the saturn, part 1
// level 2
// 58a4fa - when bumping an eye

// ----- unimplemented -----

#if 1
bool Exceptions_Fix (CONTEXT* context)
{
//	char* str;
//	char* str_extension;
	bool result = 0;
//	int i_result;
	byte* pattern;

//	exceptions_modulename = "-unknown-";
	exceptions_cause = "-unknown-";

// test int 3
#if 0
	if (exceptions_offset == 0x000018cc)
	{
		context->Eip += 1;
		return 1;
	}
#endif

//	if (! exceptions_filename_is_defined)
//		return 0;

#if 0
// **also is possible to define by content
// get filename
	str = strrchr (exceptions_filename, '\\');
	if (! str)
		return 0;
	str++;
// get extension
	str_extension = strrchr (str, '.');
	if (! str_extension)
		return 0;
	str_extension++;

	i_result = stricmp (str_extension, "dll");
	if (i_result)
		goto check_on_exe;
#endif

// someone has come, has promised to release new D3 version; others are listening him ... ; lets bind these patterns only at the current versions then
	switch (version_main)
	{
	case VERSION_MAIN_14_XX:
	case VERSION_MAIN_14_NOCD:
	case VERSION_MAIN_15_BETA:
		break;
	default:
		return 0;
	}

	pattern = (byte*)exceptions_address;

// exceptions are sorted by popularity

// classic crash :  be killed, when is in some other window (with Alt+Tab)
//	if (exceptions_address == 0x004bd43b)
	if (pattern[0] == 0x8b && pattern[1] == 0x01)
		if (pattern[2] == 0xff && pattern[3] == 0x60)
			if (pattern[4] == 0x10 && pattern[5] == 0xc3)
			{
				context->Eip += 0x004bd440 - 0x004bd43b;
				exceptions_cause = "die, while OS displays another window";
				return 1;
			}

// this is a script message failure, as I understand
//	if (exceptions_address == 0x004a5585)
	if (pattern[0] == 0x0f && pattern[1] == 0xbf)
		if (pattern[2] == 0x44 && pattern[3] == 0xca)
			if (pattern[6] == 0x04)
			{
				context->Eip += 0x004a578f - 0x004a5585;
				exceptions_cause = "message memory failure 1";
				return 1;
			}
// not tested
// simplification
	pattern -= 0x5590 - 0x5585;
//	if (exceptions_address == 0x004a5590)
	if (pattern[0] == 0x0f && pattern[1] == 0xbf)
		if (pattern[2] == 0x44 && pattern[3] == 0xca)
			if (pattern[6] == 0x04)
			{
				context->Eip += 0x004a578f - 0x004a5590;
				exceptions_cause = "message memory failure 2";
				return 1;
			}
	pattern += 0x5590 - 0x5585;

// Mercenary level 5 room 3 big door script; just push that door from the room 3, and move slightly, to produce a crash
//	if (exceptions_address == 0x004bf2d6)
	if (pattern[0] == 0x8a && pattern[1] == 0x46 && pattern[2] == 0xfe)
		{
			context->Eip += 0x004bf35f - 0x004bf2d6;
			exceptions_cause = "mercen level 5 room 3 big door script 1";
			return 1;
		}
// not tested
//	if (exceptions_address == 0x004bf1ba)
	if (pattern[-1] == 0x10 && pattern[0] == 0x66)
		if (pattern[1] == 0x89 && pattern[2] == 0xbe)
		{
			context->Eip += 0x004bf273 - 0x004bf1ba;
			exceptions_cause = "mercen level 5 room 3 big door script 2";
			return 1;
		}
//	if (exceptions_address == 0x004bf1c4)
	if (pattern[0] == 0x8a && pattern[1] == 0x44)
		if (pattern[2] == 0xd0 && pattern[3] == 0x02)
		{
			context->Eip += 0x004bf273 - 0x004bf1c4;
			exceptions_cause = "mercen level 5 room 3 big door script 3";
			return 1;
		}

// not tested
// maybe is a player respawn relative
// is chained with 00416233 next exception, then 004e838a
//	if (exceptions_address == 0x004a1fb3)
	if (pattern[-1] == 0x04 && pattern[0] == 0x66)
		if (pattern[1] == 0x8b && pattern[2] == 0x91)
		{
			context->Eip += 0x004a1ff2 - 0x004a1fb3;
			exceptions_cause = "some bug 1_1";
			return 1;
		}
	if (pattern[0] == 0x66 && pattern[1] == 0x8b)
		if (pattern[2] == 0x8c && pattern[3] == 0x42)
			if (pattern[0x0c] == 0x08)
			{
				context->Eip += 0x00416248 - 0x00416233;
// bool
				context->Eax = 0;
				exceptions_cause = "some bug 1_2";
				return 1;
			}
//	if (exceptions_address == 0x004e838a)
	if (pattern[-2] == 0xe1 && pattern[-1] == 0x04)
		if (pattern[0] == 0xd8 && pattern[1] == 0xa1)
			if (pattern[6] == 0xdc && pattern[7] == 0x1d)
			{
				context->Eip += 0x004e839d - 0x004e838a;
				exceptions_cause = "some bug 1_3";
				return 1;
			}

// Mercenary level 3, attempting to join the server;
// mzero has reported, that is getting too high amount of objects message, when joining; but I am getting just a crash.
// isn't fixable this way;
// instead, for the dedicated server, ban "Shield" powerup, to allow a client to connect;
// for nondedi the necessary to ban powerups are different;
// no need to ban shield, but need to ban a group of them, beginning from "4packConc" and ending with "Cyclone";
// these two powerups needs to be banned, and which of powerups between them could be allowed - I didn't tested, and am banning all of them;
// e-to-s-converter can be allowed.
// the tests above were done on "insane" difficulty, and on other difficulties the results may differ.
#if 0
//	if (exceptions_address == 0x00590851)
	if (pattern[0] == 0x8a && pattern[1] == 0x50)
		if (pattern[2] == 0x01 && pattern[-4] == 0xa6)
		{
			context->Eip += 0x00590a37 - 0x00590851;
			exceptions_cause = "merc level 3 \"Receiving data\" 1";
			return 1;
		}
//	if (exceptions_address == 0x00590b97)
	if (pattern[0] == 0xc6 && pattern[1] == 0x41)
		if (pattern[2] == 0x02 && pattern[3] == 0x01)
		{
			context->Eip += 0x00590b9b - 0x00590b97;
			exceptions_cause = "merc level 3 \"Receiving data\" 2_1";
			return 1;
		}
// also appears at Mercenary level 5 door bug
//	if (exceptions_address == 0x0058bd24)
	if (pattern[0] == 0x8a && pattern[1] == 0x51)
		if (pattern[2] == 0x1d)
		{
			context->Eip += 0x0058be48 - 0x0058bd24;
			exceptions_cause = "merc level 3 \"Receiving data\" 2_2";
			return 1;
		}
#endif

// strange, this one is already mentioned above
// what this kind of names means; bad, have not commented it :/
#if 0
// not tested
//	if (exceptions_address == 0x004a1fb3)
	if (pattern[-1] == 0x04 && pattern[0] == 0x66)
		if (pattern[1] == 0x8b && pattern[2] == 0x91)
		{
			context->Eip += 0x004a1ff2 - 0x004a1fb3;
			exceptions_cause = "_1_1";
			return 1;
		}
#endif
// not tested
// it is a powerup respawn relative
//	if (exceptions_address == 0x00525b56)
	if (pattern[0] == 0x8b && pattern[1] == 0x30)
		if (pattern[2] == 0x89 && pattern[3] == 0x74)
			if (pattern[4] == 0x24 && pattern[5] == 0x34)
			{
				context->Eip += 0x00525bbb - 0x00525b56;
				exceptions_cause = "_1_2";
				return 1;
			}

// Mzero says, that is getting also on 0053d39a, after this 0053d50b
//	if (exceptions_address == 0x0053d81b)
	if (pattern[0] == 0x66 && pattern[1] == 0x8b)
		if (pattern[2] == 0x54 && pattern[3] == 0x55)
			if (pattern[0x13] == 0xda && pattern[0x1f] == 0x27)
			{
				context->Eip += 0x0053db22 - 0x0053d81b;
				exceptions_cause = "at join 1";
				return 1;
			}

// Mercenary level 3, putting a seeker mine
// nope, it doesn't looks powerups relative
// *0x00590b97 and 0x0058bd24 appears also instead of well known addresses for when on Mercenary level 5 room 3 touching big door;
// so these perhaps appears only when exception handler which is in 'fs:[0]' isn't modified; otherwise those addresses do change.
// chained, but not mandatory
	if (pattern[0] == 0xc6 && pattern[1] == 0x41)
		if (pattern[2] == 0x02 && pattern[3] == 0x01)
		{
			context->Eip += 0x00590b9b - 0x00590b97;
			exceptions_cause = "some bug 2.1";
			return 1;
		}
// *near 'Sleep()'
// ebx == 0
// not tested
	if (pattern[0] == 0x8a && pattern[1] == 0x50)
		if (pattern[2] == 0x01 && pattern[3] == 0x3a)
			if (pattern[4] == 0xd3 && pattern[5] == 0x0f)
			{
				context->Eip += 0x00590a37 - 0x00590851;
				exceptions_cause = "some bug 2.2";
				return 1;
			}

// often happens on "eight sided star", when robots fight (perhaps when a thresher is being destroyed)
// no, the thresher isn't destroyed yet, but superthief still fights with him with md
// pattern 20, d94108 or d94108, c3
	if (pattern[0] == 0xd9 && pattern[1] == 0x41)
		if (pattern[2] == 0x08 && pattern[3] == 0xc3)
		{
			context->Eip += 0x0043617a - 0x00436177;
			exceptions_cause = "\"eight sided star\", bug 1";
			return 1;
		}

	return result;
}
#endif

bool Exceptions_Fix_AccessViolation (ulong address)
{
	void* malloc_area;
	ulong aligned_address;
	u32 size;
	u32 distance;

// sizes should be defined
	if (! alloc_align_bits_num)
		return 0;
// area could only be on a some sane address
	if (address < 0x00100000)
		return 0;
	if (address >= 0x70000000)
		return 0;

	aligned_address = address >> alloc_align_bits_num;
	aligned_address <<= alloc_align_bits_num;
	distance = aligned_address + alloc_align_size - address;
	size = alloc_align_size;
// no, do not grow the size :  this will produce one more trick, so a second exception, which will be on a slightly earlier address, will require previously allocated area
#if 0
	if (distance < alloc_indent_from_block_end)
// double the size, to rise the indent
		size <<= 1;
#endif

	malloc_area = VirtualAlloc ((void*)aligned_address, size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
	if (! malloc_area)
	{
		Exceptions_Log_Printf ("Exceptions_Fix_AccessViolation () :  VirtualAlloc (%08x) failed\n", aligned_address);
		Error_NoExit ("Exceptions_Fix_AccessViolation () :  VirtualAlloc (%08x) failed", aligned_address);
		return 0;
	}
// this is possible :  'VirtualAlloc ()' does auto-align (to 16 bit for me)
	if ((ulong)malloc_area != aligned_address)
	{
		Exceptions_Log_Printf ("Exceptions_Fix_AccessViolation () :  VirtualAlloc (%08x) returned another address %08x\n", aligned_address, malloc_area);
		Error_NoExit ("Exceptions_Fix_AccessViolation () :  VirtualAlloc (%08x) returned another address %08x", aligned_address, malloc_area);
		return 0;
	}
	Exceptions_Log_Printf (". allocated :  address %08x, size %08x\n", malloc_area, size);
// *could be possible to memorize this value in a 'vector<mem_area>', but no big need
	malloc_area;

	return 1;
}
#endif


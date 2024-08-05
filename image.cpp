// bored of old dll in memory, while a new one is compiled into the file
// *parts are taken from "PE_Patcher"

#include "image.h"
#include <windows.h>
#include "error.h"

// no need to dive into GetImageConfigInformation() and similar


bool Image_ReadTimestamp (u32& timestamp, byte* image, u32 len)
{
	IMAGE_DOS_HEADER* image_dos_header;
	IMAGE_NT_HEADERS* image_nt_headers;
	u32 offset;

	image_dos_header = (IMAGE_DOS_HEADER*)image;
	offset = image_dos_header->e_lfanew;
// is not outside of area
	if (offset >= len)
		return 0;
// and fits in it
	if (offset + sizeof(IMAGE_NT_HEADERS) > len)
		return 0;
	image_nt_headers = (IMAGE_NT_HEADERS*)(image + offset);

	timestamp = image_nt_headers->FileHeader.TimeDateStamp;

	return 1;
}

bool Image_CheckTimestamp (byte* image, u32 len, const char* filename)
{
	bool r;
	BOOL B_r;
	u32 filesize;
	DWORD bytes_transfered;
	byte* image_file;
	HANDLE hf;
	u32 timestamp_mem;
	u32 timestamp_file;

	hf = ::CreateFile (filename, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
	if (hf == INVALID_HANDLE_VALUE)
	{
//		printf ("CreateFile () failed\n");
		Error ("Image_CheckTimestamp() :  CreateFile() failed\n");
	}

	filesize = GetFileSize (hf, 0);
	Assert(filesize);
	image_file = (byte*)malloc (filesize);
	Assert(image_file);

	B_r = ::ReadFile (hf, image_file, filesize, &bytes_transfered, 0);
	if (! B_r)
	{
//		printf ("ReadFile () failed\n");
		Error ("Image_CheckTimestamp() :  ReadFile() failed\n");
	}

	B_r = ::CloseHandle (hf);
	if (! B_r)
	{
//		printf ("CloseHandle () failed\n");
		Error ("Image_CheckTimestamp() :  CloseHandle() failed\n");
	}

	r = Image_ReadTimestamp (timestamp_mem, image, len);
	if (! r)
		Error ("Image_CheckTimestamp() :  Image_ReadTimestamp() for memory has failed\n");
	r = Image_ReadTimestamp (timestamp_file, image_file, filesize);
	if (! r)
		Error ("Image_CheckTimestamp() :  Image_ReadTimestamp() for file has failed\n");

	free (image_file);

	if (timestamp_mem != timestamp_file)
		Error ("Image_CheckTimestamp() :  memory timestamp %08x and file timestamp %08x are different; exiting\n", timestamp_mem, timestamp_file);

	return 1;
}


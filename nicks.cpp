//

#include "nicks.h"
#include <assert.h>
#include <stdio.h>


#define NICK_SIZE_MAX 32

int nicks_letters_min = 2;
int nicks_spaces_max = 2;


bool Nicks_Check (char* result_info_string, char* nick)
{
	bool result = 0;
	int letters = 0;
	int spaces = 0;
	int size = NICK_SIZE_MAX;
	char symbol;

	assert(result_info_string);

	while (size-- && (symbol = *nick))
	{
		switch (symbol)
		{
		case ' ':
			spaces++;
			break;
		default:
			if (symbol >= 'A' && symbol <= 'Z')
				letters++;
			else if (symbol >= 'a' && symbol <= 'z')
				letters++;
		}

		nick++;
	}

	if (letters < nicks_letters_min)
	{
		if (nicks_letters_min > 0)
			sprintf (result_info_string, "nick has %d letters, %d is min", letters, nicks_letters_min);
		else
			sprintf (result_info_string, "nick has no letters");
	}
	else if (spaces > nicks_spaces_max)
	{
		if (nicks_spaces_max > 0)
			sprintf (result_info_string, "nick has %d spaces, %d is max", spaces, nicks_spaces_max);
		else
			sprintf (result_info_string, "nick has spaces");
	}
	else
// :)
	{
		sprintf (result_info_string, "nick is valid");
		result = 1;
	}

	return result;
}


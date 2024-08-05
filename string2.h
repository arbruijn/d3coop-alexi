#pragma once

// original belongs at Windows Resident Tools project

#include <string>
#include <sstream>
using namespace std;


// lets enhance :)
// bad, even operator = is not working
#if 0
class string2 : public string
{
public:
	bool cmpi (string str)
	{
		int i_result;
		i_result = stricmp (c_str (), str.c_str ());
		return ! i_result;
	}
	inline void __cdecl makeupper ()
	{
		int i;
		int len;
		len = length ();
// *<string> uses 'for' as iterator, so will do I
		for (i = 0; i < len; i++)
// *'ctype' also requires function from std lib anyway
			(*this)[i] = toupper ((*this)[i]);
	}
	inline void __cdecl mkupper ()
	{
		makeupper ();
	}
	inline void __cdecl makelower ()
	{
		int i;
		int len;
		len = length ();
		for (i = 0; i < len; i++)
			(*this)[i] = tolower ((*this)[i]);
	}
	inline void __cdecl mklower ()
	{
		makelower ();
	}
};
#endif

// ok, if the cart isn't moving, then lets mount a rocket engine at it :/

// *as a hacky way, for the such function often also is used 'tolower()'
// offered also is "International Components for Unicode" to use instead of this ASCII subr.
inline bool cmpi (string str0, string str1)
{
	int i_result;
	i_result = _stricmp (str0.c_str (), str1.c_str ());
	return ! i_result;
}
inline bool cmpni (string str0, string str1, int len)
{
	int i_result;
	i_result = _strnicmp (str0.c_str (), str1.c_str (), len);
	return ! i_result;
}
inline void __cdecl mkupper (string& str)
{
	int i;
	int len;
	len = str.length ();
// *<string> uses 'for' as iterator, so will do I
	for (i = 0; i < len; i++)
// *'ctype' also requires function from std lib anyway
		str[i] = toupper (str[i]);
}
inline string __cdecl mkupper (const char* str)
{
#if 1
	string s;
	s = str;
	mkupper(s);
	return s;
#else
	mkupper(str);
	return str;
#endif
}
inline void __cdecl mklower (string& str)
{
	int i;
	int len;
	len = str.length ();
	for (i = 0; i < len; i++)
		str[i] = tolower (str[i]);
}
inline string __cdecl mklower (const char* str)
{
	string s;
	s = str;
	mklower(s);
	return s;
}
// I couldn't say earlier what is better - '\r' or '\n';
// now I can say :  if look from ASCII symbols purposes - both of them are wrong, and proper combination for the new-line symbol is "\r\n";
// ASCII does not contain new-line or line-end symbol, only if IBM's 0xb5;
// just to support all three cases (and even sometimes happened buggied "\r\r\n" and "\r\n\n").
// but definitely will not support backward combination "\n\r" ! - only three of four ("\n", "\r", "\r\n", "\n\r") combinations is possible to support.
inline void getline (stringstream& strm, string& str)
{
	char c;
	bool got_some_output = 0;
	str.erase();
loop:
	c = strm.peek();
// *if some
	if (strm.fail())
		return;
	if (strm.eof())
		goto finish;
	if (c == '\r' || c == '\n')
		goto skip_returns;
//	strm >> c;
	strm.ignore();
// produce an exception when some size is reached :/
	str += c;
	got_some_output = 1;
	goto loop;
// now must separately skip '\r' and '\n'; not by single loop !
// so if next will be an empty line, then it must be returned at the next request
skip_returns:
	c = strm.peek();
	if (strm.fail())
		return;
	if (strm.eof())
		goto finish;
	if (c != '\r')
		goto skip_newlines;
	strm.ignore();
	goto skip_returns;
skip_newlines:
	c = strm.peek();
	if (strm.fail())
		return;
	if (strm.eof())
		goto finish;
	if (c != '\n')
		goto finish;
	strm.ignore();
	goto skip_newlines;
finish:
	if (! got_some_output)
		strm.setstate(ios_base::failbit);
	return;
}
inline void trimbegin (string& str, const char* soilers_list = "\t\n\r ")
{
	int i;
	i = str.find_first_not_of(soilers_list);
	str.erase(0, i);
}
inline void trimend (string& str, const char* soilers_list = "\t\n\r ")
{
	int i;
	i = str.find_last_not_of(soilers_list);
	str.erase(i + 1);
}
inline void trim (string& str, const char* soilers_list = "\t\n\r ")
{
	trimbegin (str, soilers_list);
	trimend (str, soilers_list);
}


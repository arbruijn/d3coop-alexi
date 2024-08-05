//
// atm it only is intended to be like as an advanced 'printf()'

#include "script.h"
#include "macros.h"
//#include <vector>
#include <list>
//using namespace std;
#include "error.h"


struct script_var_t
{
	string name;
// *a variable also must have a type, but we don't care about it now
	string value;
};

#if 0
typedef vector<script_var_t> vec_script_var_t;
//vector<script_var_t> script_vars;
vec_script_var_t script_vars;
//typedef vector<script_var_t>::iterator vec_script_var_it_t;
typedef vec_script_var_t::iterator vec_script_var_it_t;
#endif
typedef list<script_var_t> list_script_var_t;
//list<script_var_t> script_vars;
list_script_var_t script_vars;
//typedef list<script_var_t>::iterator list_script_var_it_t;
typedef list_script_var_t::iterator list_script_var_it_t;

// config
#define VAR_MUST_EXIST 0
//#define VAR_MUST_EXIST 1
#define SCRIPT_VAR_CHAR '%'


// 'is_same'

// some simplier version
// fail :  "error C2989: 'is_sam<T,T>' : template class has already been defined as a non-template class"
#if 0
template<typename T, typename U>
struct is_sam
{
	enum { value = 0 };
//	T t;
//	U u;
};
template<typename T>
struct is_sam<T, T>
{
	enum { value = 1 };
};
#endif

// GCC version
#if 0
#if 0
// copy/paste from GCC
/// integral_constant
template<typename _Tp, _Tp __v>
struct integral_constant
{
	static constexpr _Tp                  value = __v;
	typedef _Tp                           value_type;
	typedef integral_constant<_Tp, __v>   type;
	constexpr operator value_type() { return value; }
};
/// typedef for true_type
typedef integral_constant<bool, true>     true_type;
/// typedef for false_type
typedef integral_constant<bool, false>    false_type;
#endif
//template<typename, typename>
template<typename T, typename U>
struct is_sam : public false_type { };
//template<typename _Tp>
//struct is_sam<_Tp, _Tp> : public true_type { };
#endif


// returns ptr to var, or var ix
//static bool Script_FindVar (const char*& value, const char* name)
static bool Script_FindVar (list_script_var_it_t& var, const char* name)
// uhoh, MSVC 6.0 can't take this (can't conditionally do a different return results)
//template<typename T>
//bool Script_FindVar (T& var, const char* name)
{
	bool result;
//	list_script_var_it_t it;
	int i;

//	SA(typeid(T) == typeid(const char*));
//// *note, that with the current "typeof.h" it may rise some problems on MSVC other than 6.0
// "boost/type_traits/is_same.hpp" can be used for that
//	SA(decltype(T) == decltype(const char*));
// this one is for C++0x
//	SA(is_same<T, const char*>::value);
	result = 0;

//	FOREACH(it, script_vars)
	FOREACH_EXT(it, script_vars)
	{
		i = it->name.compare(name);
#if 0
		if (! i)
		{
//			var = &*it;
//			var = it;
//			return 1;
			result = 1;
			break;
		}
		else if (i < 0)
// we have gone too far
//			return 0;
			break;
#else
//		if (i <= 0)
		if (i >= 0)
// we have skipped all previous names; now we are or on the necessary position, or on the one which will have to be next
		{
//			if (! i)
//				result = 1;
			result = ! i;
			break;
		}
#endif
	}

// if has not found, then this is the position where the var should be inserted in the list and be sorted (if will be necessary)
	var = it;

//	return 0;
	return result;
}

void Script_ClearVars ()
{
	script_vars.clear();
}

// *should maybe take 'script_var_t' instead of 'name'+'var', not sure
bool Script_Var_Insert (list_script_var_it_t& place, const char* name, const char* value)
{
	script_var_t var;
	
	var.name = name;
	var.value = value;

//	script_vars.insert(place, {name, value});
//	script_vars.insert(place, script_var_t(name, value));
	script_vars.insert(place, var);
	return 1;
}

#if 0
// find where to insert and then do insert
bool Script_Var_Insert (const char* name, const char* value)
{
	return 0;
}
#endif

#if VAR_MUST_EXIST
bool Script_Var_Add (const char* name, const char* value)
{
	return 1;
}
#endif

bool Script_Var_Update (const char* name, const char* value)
{
	bool r;
//	script_var_t* var;
	list_script_var_it_t it;

//	r = Script_FindVar (var, name);
	r = Script_FindVar (it, name);
	if (! r)
	{
#if VAR_MUST_EXIST
		return 0;
#else
		r = Script_Var_Insert (it, name, value);
		if (! r)
			return 0;
#endif
	}
	else
	{
		it->value = value;
	}
	return 1;
}

bool Script_Var_GetVal (const char*& value, const char* name)
{
	bool r;
	list_script_var_it_t it;

	r = Script_FindVar (it, name);
	if (! r)
		return 0;
	value = it->value.c_str();
	return 1;
}

//bool Script_String_InplaceIn (string& out, string& in, const char* name)
bool Script_String_InplaceIn (string& out, string& in)
{
	bool r;
	string::iterator it0;
	string::iterator it1;
	string::iterator it2;
	string::iterator it_end;
	string name;
//	const char* value;
	string* value;
	list_script_var_it_t it;

	out.erase();
	it0 = in.begin();
	it_end = in.end();

//	FOREACH(it1, in)
//loop:
	while (it0 != it_end)
//	do
	{
		value = 0;
// search for a script character from here
//		it1 = it0;
//		while (it1 != it_end)
		for (it1 = it0; it1 != it_end; it1++)
		{
// *opening and closing could also be by some different characters, like '{}', but I don't care atm
			if (*it1 != SCRIPT_VAR_CHAR)
				continue;
			goto find_end;
		}
// no more variable found in the remainder
// set it to 'it_end' ('it1' is at it now)
		it2 = it1;
		goto append_string;

find_end:
		for (it2 = it1 + 1; it2 != it_end; it2++)
		{
			if (*it2 != SCRIPT_VAR_CHAR)
				continue;
			name.assign(it1 + 1, it2);
// skip the closing character
			it2++;
			r = Script_FindVar (it, name.c_str());
			if (! r)
//				return 0;
				Error ("no variable \"%s\" exist", name.c_str());
			value = &it->value;
			goto append_string;
		}
		Error ("'%c' is opened but is not closed", SCRIPT_VAR_CHAR);

append_string:
		out.append(it0, it1);
		if (value)
			out.append(*value);
//		out += *value;
		it0 = it2;
	}
//	while (it0 != it_end)

	return 1;
}


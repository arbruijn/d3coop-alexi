#pragma once

#include <string>
using namespace std;


void Script_ClearVars ();
//bool Script_Var_Insert (const char* name, const char* value);
//bool Script_Var_Add (const char* name, const char* value);
bool Script_Var_Update (const char* name, const char* value);
bool Script_Var_GetVal (const char*& value, const char* name);
bool Script_String_InplaceIn (string& out, string& in);


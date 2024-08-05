#pragma once

#include "types.h"


// +++ config +++
extern bool exceptions_catch;
extern bool exceptions_avoid_repeats;
// --- config ---


void Exceptions_Init ();
void Exceptions_Quit ();
void Exceptions_Frame ();


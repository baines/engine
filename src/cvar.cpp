#include "cvar.h"

template<> CVar* CVar::get<CVar>(void){ return this; }

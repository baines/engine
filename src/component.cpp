#include "component.h"

unsigned getNextComponentID(){
	static unsigned id = 0;
	return id++;
}


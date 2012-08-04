#ifndef _GAMESTATE_H_
#define _GAMESTATE_H_
#include "input.h"
#include "sprite.h"

class StateMgr;

class Gamestate {
public:
	Gamestate(StateMgr& mgr) : statemgr(mgr){};
	virtual ~Gamestate(){};
	virtual void update(Input& input, Uint32 delta){};
	virtual void draw(std::vector<uint32_t>& indices){};
protected:
	StateMgr& statemgr;
};

#endif

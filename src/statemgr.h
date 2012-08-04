#ifndef _STATEMGR_H_
#define _STATEMGR_H_
#include <vector>
#include <queue>
#include <stdint.h>
#include <memory>
#include "gamestate.h"

class Input;

class StateMgr {
	typedef std::shared_ptr<Gamestate> state_ptr;
public:
	StateMgr();
	~StateMgr();
	void update(Input& input, uint32_t delta);
	void draw(std::vector<uint32_t>& indices);
	void pop(int amount);
	void push(const state_ptr& state);
	void push(Gamestate* state);
private:
	typedef std::vector<state_ptr>::const_iterator state_itr;
	std::vector<state_ptr> states;
	std::queue<state_ptr> nextStates;
	int popAmount;
};

#endif
	

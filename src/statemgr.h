#ifndef _STATEMGR_H_
#define _STATEMGR_H_
#include <vector>
#include <queue>
#include <stdint.h>

class Gamestate;
class Input;

class StateMgr {
public:
	StateMgr();
	~StateMgr();
	void update(Input& input, uint32_t delta);
	void draw(std::vector<uint32_t>& indices);
	void pop(int amount);
	void push(Gamestate* state);
private:
	typedef std::vector<Gamestate*>::const_iterator state_itr;
	std::vector<Gamestate*> states;
	std::queue<Gamestate*> nextStates;
	int popAmount;
};

#endif
	

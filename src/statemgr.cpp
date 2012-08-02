#include "statemgr.h"
#include "gamestate.h"
#include "input.h"
#include "util.h"

StateMgr::StateMgr() : states(), nextStates(), popAmount(0){}

StateMgr::~StateMgr(){
	while (!states.empty()){
		delete states.back();
    	states.pop_back();
	}
}

void StateMgr::update(Input& input, Uint32 delta){

	while(popAmount > 0){
		delete states.back();
		states.pop_back();
		--popAmount;
	}

	while(!nextStates.empty()){
		states.push_back(nextStates.front());
		nextStates.pop();
	}
	states.back()->update(input, delta);
}

void StateMgr::draw(std::vector<uint32_t>& indices){
	for(state_itr i = states.begin(), j = states.end(); i != j; ++i){
			(*i)->draw(indices);
	}
}

void StateMgr::push(Gamestate* state){
	nextStates.push(state);
}

void StateMgr::pop(int amount){
	popAmount = util::min(amount, (int)states.size());
}


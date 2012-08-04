#include "statemgr.h"
#include "gamestate.h"
#include "input.h"
#include "util.h"
#include <algorithm>

StateMgr::StateMgr() : states(), nextStates(), popAmount(0){}

StateMgr::~StateMgr(){
	std::for_each(states.begin(), states.end(), [](state_ptr& p){
		printf("%ld\n", p.use_count());
	});
	states.clear();
}

void StateMgr::update(Input& input, Uint32 delta){
	for(; popAmount > 0; --popAmount){
		states.pop_back();
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

void StateMgr::push(const std::shared_ptr<Gamestate>& state){
	nextStates.push(state);
}

void StateMgr::push(Gamestate* state){
	nextStates.push(std::move(std::shared_ptr<Gamestate>(state)));
}

void StateMgr::pop(int amount){
	popAmount = util::min(amount, (int)states.size());
}


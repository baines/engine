#include "state_system.h"
#include "game_state.h"
#include "engine.h"
#include "input.h"

StateSystem::StateSystem()
: states()
, new_states()
, pop_num(0) {

}

void StateSystem::push(GameState* s){
	new_states.push_back(s);
}

void StateSystem::pop(int amount){
	pop_num = std::min<int>(states.size(), pop_num + amount);
}
		
void StateSystem::onInput(Engine& e, SDL_Event& ev){
	int action_id = 0;
	
	if(ev.type == SDL_KEYDOWN || SDL_KEYUP){
		for(auto i = states.rbegin(), j = states.rend(); i != j; ++i){
			if(e.input.getKeyAction(*i, ev.key.keysym, action_id)
			&& (*i)->onInput(e, action_id, ev.key.state)){
				break;
			}
		}
	} else if(ev.type == SDL_CONTROLLERBUTTONDOWN || ev.type == SDL_CONTROLLERBUTTONUP){
		for(auto i = states.rbegin(), j = states.rend(); i != j; ++i){
			if(e.input.getPadAction(*i, ev.cbutton.which, ev.cbutton.button, action_id)
			&& (*i)->onInput(e, action_id, ev.cbutton.state)){
				break;
			}
		}
	} else if(ev.type == SDL_MOUSEWHEEL){
		//TODO
	}
}

void StateSystem::onMotion(Engine& e, SDL_Event& ev){
	//TODO

}

void StateSystem::onText(Engine& e, SDL_TextInputEvent& ev){
	assert(!states.empty());

	states.back()->onText(e, ev.text);
}

void StateSystem::update(Engine& e, uint32_t delta){
	GameState* current_state = states.empty() ? nullptr : states.back();
	
	while(pop_num > 0 && !states.empty()){
		pop_num--;
		states.back()->onQuit(e);
		states.pop_back();
	}
	
	for(auto* s : new_states){
		states.push_back(s);
		states.back()->onInit(e);
	}
	
	new_states.clear();
	
	assert(!states.empty());
	
	if(current_state != states.back()){
		if(current_state){
			current_state->onStateChange(e, false);
		}
		current_state = states.back();
		current_state->onStateChange(e, true);
		e.input.onStateChange(current_state);
	}

	current_state->update(e, delta);
}

void StateSystem::draw(Renderer& r){
	for(auto* s : states){
		s->draw(r);
	}
}

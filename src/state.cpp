#include "state_system.h"
#include "game_state.h"
#include "engine.h"
#include "input.h"
#include "root_state.h"
#include "cli.h"
#include "util.h"
#include <assert.h>

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
	bool pressed = true;
	InputKey key;
	
	if(ev.type == SDL_KEYDOWN || ev.type == SDL_KEYUP){
		key = InputKey(ev.key.keysym);
		pressed = ev.key.state;
	} else if(ev.type == SDL_CONTROLLERBUTTONDOWN || ev.type == SDL_CONTROLLERBUTTONUP){
		key = InputKey(pad_button_tag, ev.cbutton.button);
		pressed = ev.cbutton.state;
	} else if(ev.type == SDL_MOUSEBUTTONDOWN || ev.type == SDL_MOUSEBUTTONUP){
		key = InputKey(mouse_button_tag, ev.button.button);
		pressed = ev.button.state;
	} else if(ev.type == SDL_MOUSEWHEEL){
		key = InputKey(mouse_wheel_tag, ev.wheel.y);
		//XXX: no released event for mouse wheel.
	}
	
	for(auto i = states.rbegin(), j = states.rend(); i != j; ++i){
		if(e.input->getKeyAction(*i, key, action_id) && (*i)->onInput(e, action_id, pressed)){
			break;
		}
	}
}

void StateSystem::onMotion(Engine& e, SDL_Event& ev){
	
	auto send_motion_fn = [&](const InputAxis& axis, int val){
		int action_id = -1;
		bool rel = false;
		float scale = 1.0f;
		
		for(auto i = states.rbegin(), j = states.rend(); i != j; ++i){
			if(e.input->getAxisAction(*i, axis, action_id, rel, scale)
			&& (*i)->onMotion(e, action_id, val * scale, rel)){
				break;
			}
		}
	};

	if(ev.type == SDL_MOUSEMOTION){
		send_motion_fn(InputAxis(mouse_tag, 0), ev.motion.x);
		send_motion_fn(InputAxis(mouse_tag, 1), ev.motion.y);
	} else {
		//TODO
	}
}

void StateSystem::onText(Engine& e, SDL_TextInputEvent& ev){
	assert(!states.empty());

	states.back()->onText(e, ev.text);
}

void StateSystem::onResize(Engine& e, int w, int h){
	for(auto* s : states){
		s->onResize(e, w, h);
	}
}

void StateSystem::processStateChanges(Engine& e){
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
		e.input->onStateChange(current_state);
	}
}

void StateSystem::update(Engine& e, uint32_t delta){
	assert(!states.empty());
	states.back()->update(e, delta);
}

void StateSystem::draw(IRenderer& r){
	for(auto* s : states){
		s->draw(r);
	}
}

/* root state */

enum {
	ACTION_QUIT,
	ACTION_TOGGLE_CONSOLE
};

RootState::RootState(Engine& e) {
	e.input->subscribe(this, "menu", ACTION_QUIT);
	e.input->subscribe(this, "console", ACTION_TOGGLE_CONSOLE);
}

void RootState::update(Engine& e, uint32_t delta){

}

void RootState::draw(IRenderer& r){

}

bool RootState::onInput(Engine& e, int action_id, bool pressed){
	bool handled = false;
	
	if(action_id == ACTION_QUIT){
		e.quit();
		handled = true;
	} else if(pressed && action_id == ACTION_TOGGLE_CONSOLE){
		e.cli->toggle();
		handled = true;
	}
	
	return handled;
}


#include "state_system.h"
#include "game_state.h"
#include "engine.h"
#include "input.h"
#include <cassert>

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
	Input::Key key;
	
	if(ev.type == SDL_KEYDOWN || ev.type == SDL_KEYUP){
		key = Input::Key(ev.key.keysym);
		pressed = ev.key.state;
	} else if(ev.type == SDL_CONTROLLERBUTTONDOWN || ev.type == SDL_CONTROLLERBUTTONUP){
		key = Input::Key(pad_button_tag, ev.cbutton.button);
		pressed = ev.cbutton.state;
	} else if(ev.type == SDL_MOUSEBUTTONDOWN || ev.type == SDL_MOUSEBUTTONUP){
		key = Input::Key(mouse_button_tag, ev.button.button);
		pressed = ev.button.state;
	} else if(ev.type == SDL_MOUSEWHEEL){
		key = Input::Key(mouse_wheel_tag, ev.wheel.y);
		//XXX: no released event for mouse wheel.
	}
	
	for(auto i = states.rbegin(), j = states.rend(); i != j; ++i){
		if(e.input->getKeyAction(*i, key, action_id)	&& (*i)->onInput(e, action_id, pressed)){
			break;
		}
	}
}

void StateSystem::onMotion(Engine& e, SDL_Event& ev){
	
	auto send_motion_fn = [&](const Input::Axis& axis, int val){
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
		send_motion_fn(Input::Axis(mouse_tag, 0), ev.motion.x);
		send_motion_fn(Input::Axis(mouse_tag, 1), ev.motion.y);
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

void StateSystem::draw(Renderer& r){
	for(auto* s : states){
		s->draw(r);
	}
}

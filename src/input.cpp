#include "input.h"
#include "engine.h"

using namespace std;

Input::Input(Engine& e){
	//XXX: consolidate these two?
	e.cfg.addVar("bind", CVarFunc([&](std::string&& s){
		char* state = nullptr;
		char* key = strtok_r(&s[0]  , " \t", &state);
		char* act = strtok_r(nullptr, " \t", &state);
		
		if(key && act){
			this->bind(key, act);
		}
	}));
	e.cfg.addVar("bind_raw", CVarFunc([&](std::string&& s){
		char* state = nullptr;
		char* key = strtok_r(&s[0]  , " \t", &state);
		char* act = strtok_r(nullptr, " \t", &state);
		
		if(key && act){
			auto code = static_cast<SDL_Scancode>(strtol(key, nullptr, 0));
			this->bindRaw(code, act);
		}
	}));
}

void Input::bind(const char* input_name, const char* action){
	SDL_Scancode key = SDL_GetScancodeFromName(input_name);
	bindRaw(key, action);
}

void Input::bindRaw(SDL_Scancode key, const char* action){
	if(key != SDL_SCANCODE_UNKNOWN){
		binds.emplace(string(action), key);
		
		for(auto states = watches.find(action); states != watches.end(); ++states){
			StateAction& action = states->second;
			
			active_binds.emplace(StateKey{action.state, key}, action.id);
		}
	}
}

void Input::unbind(const char* input_name){
	SDL_Scancode key = SDL_GetScancodeFromName(input_name);
	unbindRaw(key);
}

void Input::unbindRaw(SDL_Scancode key){
	if(key != SDL_SCANCODE_UNKNOWN){
		for(auto it = binds.begin(); it != binds.end(); /**/){
			if(it->second == key){
				binds.erase(it++);
			} else {
				++it;
			}
		}
		for(auto it = active_binds.begin(); it != active_binds.end(); /**/){
			if(it->first.key == key){
				active_binds.erase(it++);
			} else {
				++it;
			}
		}
	}
}

void Input::onDeviceChange(SDL_ControllerDeviceEvent& event){
//TODO

}

void Input::watchAction(GameState* s, const char* action, int action_id){

	watches.emplace(action, StateAction{s, action_id});
	
	auto it = binds.find(action);
	if(it != binds.end()){
		active_binds.emplace(StateKey{s, it->second}, action_id);
	}
}

bool Input::getKeyAction(GameState* s, SDL_Scancode key, int& action_id){

	auto it = active_binds.find(StateKey{s, key});
	
	if(it != active_binds.end()){
		action_id = it->second;
		return true;
	} else {
		return false;
	}
}


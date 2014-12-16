#include "input.h"
#include "engine.h"

using namespace std;

static bool parse_bind(char* buff, char*& key, char*& act){
	char* state = nullptr;
	key = strtok_r(buff   , " \t", &state);
	act = strtok_r(nullptr, " \t", &state);
	
	return key && act;
}

Input::Input(Engine& e)
: binds()
, active_binds()
, bound_actions()
, current_state(nullptr){

	e.cfg.addVar("bind", CVarFunc([&](const string_view& str){
		char buff[str.size()+1] = {}, *key = nullptr, *act = nullptr;
		str.copy(buff, str.size());
		
		if(parse_bind(buff, key, act)){
			this->bind(key, str_hash(act));
		}
	}));
	e.cfg.addVar("bind_raw", CVarFunc([&](const string_view& str){
		char buff[str.size()+1] = {}, *key = nullptr, *act = nullptr;
		str.copy(buff, str.size());
		
		if(parse_bind(buff, key, act)){
			auto code = static_cast<SDL_Scancode>(strtol(key, nullptr, 0));
			this->bindRaw(code, str_hash(act));
		}
	}));

	SDL_StopTextInput();
}

void Input::bind(const char* input_name, uint32_t action){
	SDL_Scancode key = SDL_GetScancodeFromName(input_name);
	bindRaw(key, action);
}

void Input::bindRaw(SDL_Scancode key, uint32_t action){
	if(key != SDL_SCANCODE_UNKNOWN){
		binds.emplace(action, key);
		
		auto pair = bound_actions.equal_range(action);
		for(auto i = pair.first, j = pair.second; i != j; ++i){
			StateAction& sa =i->second;
			active_binds.emplace(StateKey{sa.state, key}, sa.id);
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

void Input::onStateChange(GameState* s){
	auto it = text_states.find(s);

	if(it != text_states.end()){
		SDL_StartTextInput();
		SDL_SetTextInputRect(&it->second);
	} else {
		SDL_StopTextInput();
	}

	current_state = s;
}

void Input::watchAction(GameState* s, const str_const& action, int action_id){

	bound_actions.emplace(action.hash, StateAction{s, action_id});
	
	auto it = binds.find(action.hash);
	if(it != binds.end()){
		active_binds.emplace(StateKey{s, it->second}, action_id);
	}
}

void Input::enableText(GameState* gs, bool enable, const SDL_Rect& pos){
	if(enable){
		text_states[gs] = pos;
	} else {
		text_states.erase(gs);
	}

	if(gs == current_state){
		onStateChange(current_state);
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

bool Input::getPadAction(GameState* s, SDL_JoystickID id, int button, int& action_id){
	// TODO
	return false;
}


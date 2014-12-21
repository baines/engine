#include "input.h"
#include "engine.h"
#include <tuple>

using namespace std;

Input::Key::Key() : code(SDL_SCANCODE_UNKNOWN), shift(false), ctrl(false), alt(false){}

Input::Key::Key(SDL_Scancode code) : code(code), shift(false), ctrl(false), alt(false){}

Input::Key::Key(const SDL_Keysym& k)
: code  (k.scancode)
, shift (k.mod & (KMOD_LSHIFT | KMOD_RSHIFT))
, ctrl  (k.mod & (KMOD_LCTRL  | KMOD_RCTRL))
, alt   (k.mod & (KMOD_LALT   | KMOD_RALT)){

}

Input::Key::Key(const char* str, bool raw_scancode){
	const char* mod_separator = nullptr;

	if(str[0] != '-' && (mod_separator = strchr(str, '-'))){
		for(const char* p = str; p < mod_separator; ++p){
			if(*p == 's' || *p == 'S') shift = true;
			if(*p == 'c' || *p == 'C') ctrl  = true;
			if(*p == 'a' || *p == 'A') alt   = true;
		}
		str = mod_separator+1;
	}

	if(raw_scancode){
		code = static_cast<SDL_Scancode>(strtol(str, nullptr, 0));
	} else {
		code = SDL_GetScancodeFromName(str);
	}
}

bool Input::Key::operator<(const Key& rhs) const {
	return tie(code, shift, ctrl, alt) < tie(rhs.code, rhs.shift, rhs.ctrl, rhs.alt);	
}

bool Input::Key::operator==(const Key& rhs) const {
	return tie(code, shift, ctrl, alt) == tie(rhs.code, rhs.shift, rhs.ctrl, rhs.alt);
}

bool Input::StateKey::operator<(const StateKey& rhs) const {
	return tie(state, key) < tie(rhs.state, rhs.key);
}

static void bind_key_fn(Input* const input, const string_view& str, bool raw){
	char buff[str.size()+1] = {};
	str.copy(buff, str.size());
	
	char* state = nullptr;
	char* key = strtok_r(buff   , " \t", &state);
	char* act = strtok_r(nullptr, " \t", &state);
	
	if(key && act){
		input->bind(Input::Key(key, raw), str_hash(act));
	}
}

Input::Input(Engine& e)
: binds()
, active_binds()
, bound_actions()
, current_state(nullptr){

	using namespace std::placeholders;

	e.cfg.addVar("bind",     CVarFunc(std::bind(&bind_key_fn, this, _1, false)));
	e.cfg.addVar("bind_raw", CVarFunc(std::bind(&bind_key_fn, this, _1, true)));

	SDL_StopTextInput();
}

void Input::bind(Key key, uint32_t action){
	if(key.code != SDL_SCANCODE_UNKNOWN){
		binds.emplace(action, key);
		
		auto pair = bound_actions.equal_range(action);
		for(auto i = pair.first, j = pair.second; i != j; ++i){
			StateAction& sa =i->second;
			active_binds.emplace(StateKey{sa.state, key}, sa.id);
		}
	}
}

void Input::unbind(Key key){
	if(key.code != SDL_SCANCODE_UNKNOWN){
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

bool Input::getKeyAction(GameState* s, const Key& key, int& action_id){

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


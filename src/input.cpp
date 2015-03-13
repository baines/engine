#include "input.h"
#include "engine.h"
#include <tuple>

mouse_button_tag_t mouse_button_tag;
mouse_wheel_tag_t mouse_wheel_tag;
pad_button_tag_t pad_button_tag;
mouse_tag_t mouse_tag;

using namespace std;

Input::Key::Key(const SDL_Keysym& k)
: code  (k.scancode)
, shift (k.mod & (KMOD_LSHIFT | KMOD_RSHIFT))
, ctrl  (k.mod & (KMOD_LCTRL  | KMOD_RCTRL))
, alt   (k.mod & (KMOD_LALT   | KMOD_RALT))
, ignore_mods(false) {

}

static const char* pad_buttons[] = {
	"a",
	"b",
	"x",
	"y",
	"back",
	"guide",
	"start",
	"lsb",
	"rsb",
	"lb",
	"rb",
	"up",
	"down",
	"left",
	"right"
};

static const uint32_t KEY_MOUSEBUTTON_BIT = 0x10000;
static const uint32_t KEY_MOUSEWHEEL_BIT  = 0x20000;
static const uint32_t KEY_GAMEPAD_BIT     = 0x40000;

Input::Key::Key(mouse_button_tag_t, uint8_t button)
: code(KEY_MOUSEBUTTON_BIT | button)
, shift(false)
, ctrl(false)
, alt(false)
, ignore_mods(true) {

}

Input::Key::Key(mouse_wheel_tag_t, int dir)
: code(dir > 0 ? KEY_MOUSEWHEEL_BIT : dir < 0 ? KEY_MOUSEWHEEL_BIT | 1 : 0)
, shift(false) //TODO: get the actual current keyboard mods
, ctrl(false)
, alt(false)
, ignore_mods(true) {
	
}

Input::Key::Key(pad_button_tag_t, uint8_t button)
: code(KEY_GAMEPAD_BIT | button)
, shift(false)
, ctrl(false)
, alt(false)
, ignore_mods(true) {
	
}

Input::Key::Key(const char* str, bool raw_scancode)
: code(SDL_SCANCODE_UNKNOWN)
, shift(false)
, ctrl(false)
, alt(false)
, ignore_mods(false){

	const char* mod_separator = nullptr;

	if(str[0] != '-' && (mod_separator = strchr(str, '-'))){
		for(const char* p = str; p < mod_separator; ++p){
			if(*p == 's' || *p == 'S') shift = true;
			if(*p == 'c' || *p == 'C') ctrl  = true;
			if(*p == 'a' || *p == 'A') alt   = true;
			if(*p == '*') ignore_mods = true;
		}
		str = mod_separator+1;
	}

	if(raw_scancode){
		code = strtol(str, nullptr, 0);
	} else {
		if(strncasecmp(str, "mb", 2) == 0){
			code = KEY_MOUSEBUTTON_BIT | strtoul(str + 2, nullptr, 10);
		} else if(strncasecmp(str, "mwheel", 6) == 0) {
			code = str[6] == 'u'
			     ? KEY_MOUSEWHEEL_BIT
			     : str[6] == 'd'
			     ? KEY_MOUSEWHEEL_BIT | 1
			     : 0
			     ;
		} else if(strncasecmp(str, "pad_", 4) == 0){
				
			for(size_t i = 0; i < SDL_arraysize(pad_buttons); ++i){
				if(strcasecmp(str + 4, pad_buttons[i]) == 0){
					code = KEY_GAMEPAD_BIT | i;
					break;
				}
			}
		} else {
			code = SDL_GetScancodeFromName(str);
		}
	}
}

bool Input::Key::operator<(const Key& rhs) const {
	if(ignore_mods || rhs.ignore_mods){
		return code < rhs.code;
	} else {
		return tie(code, shift, ctrl, alt) < tie(rhs.code, rhs.shift, rhs.ctrl, rhs.alt);
	}
}

bool Input::Key::operator==(const Key& rhs) const {
	if(ignore_mods || rhs.ignore_mods){
		return code == rhs.code;
	} else {
		return tie(code, shift, ctrl, alt) == tie(rhs.code, rhs.shift, rhs.ctrl, rhs.alt);
	}
}

Input::Axis::Axis(mouse_tag_t, int axis)
: type(AXIS_MOUSE)
, device_index(0)
, axis_index(axis) {

}

Input::Axis::Axis(const char* str)
: type(AXIS_INVALID)
, device_index()
, axis_index() {

	constexpr struct axis_map {
		AxisType type;
		const char* name;
	} axes[] = {
		{ AXIS_MOUSE, "mouse_x" },
		{ AXIS_MOUSE, "mouse_y" },
		{ AXIS_PAD,   "pad_ls_x" },
		{ AXIS_PAD,   "pad_ls_y" },
		{ AXIS_PAD,   "pad_rs_x" },
		{ AXIS_PAD,   "pad_rs_y" },
		{ AXIS_PAD,   "pad_lt" },
		{ AXIS_PAD,   "pad_rt" }
	};
	
	for(size_t i = 0; i < SDL_arraysize(axes); ++i){
		if(strcasecmp(str, axes[i].name) == 0){
			type = axes[i].type;
			device_index = 0;   //TODO: multiple pads / mice.
			axis_index = i;
			break;
		}
	}
}

bool Input::Axis::operator<(const Axis& rhs) const {
	return tie(type, device_index, axis_index)
	     < tie(rhs.type, rhs.device_index, rhs.axis_index);
}

bool Input::Axis::operator==(const Axis& rhs) const {
	return tie(type, device_index, axis_index)
	    == tie(rhs.type, rhs.device_index, rhs.axis_index);
}

Input::Binding::Binding(Key k)
: type(BINDING_KEY) {
	data.key = k;
}

Input::Binding::Binding(Axis a, bool rel, float scale)
: type(BINDING_AXIS) {
	data.axis = { a, rel, scale };
}

bool Input::Binding::operator<(const Binding& rhs) const {
	if(type != rhs.type) return type < rhs.type;

	if(type == BINDING_KEY){
		return data.key < rhs.data.key;
	} else {
		return data.axis.axis < rhs.data.axis.axis;
	}
}

bool Input::Binding::operator==(const Key& k) const {
	return type == BINDING_KEY && data.key == k;
}

bool Input::Binding::operator==(const Axis& a) const {
	return type == BINDING_AXIS && data.axis.axis == a;
}

bool Input::StateBind::operator<(const StateBind& rhs) const {
	return tie(state, bind) < tie(rhs.state, rhs.bind);
}

static bool bind_key_fn(Input* const input, const string_view& str, bool raw){
	char buff[str.size()+1]; //XXX: allow VLAs?
	memset(buff, 0, str.size()+1);
	str.copy(buff, str.size());
	
	char* state = nullptr;
	char* key = strtok_r(buff   , " \t", &state);
	char* act = strtok_r(nullptr, " \t", &state);
	
	if(key && act){
		input->bind(Input::Key(key, raw), str_hash(act));
		return true;
	} else {
		return false;
	}
}

static bool bind_axis_fn(Input* const input, const string_view& str){
	char buff[str.size()+1];
	memset(buff, 0, str.size()+1);
	str.copy(buff, str.size());

	char* state = nullptr;
	char* str_axis  = strtok_r(buff   , " \t", &state);
	char* str_act   = strtok_r(nullptr, " \t", &state);
	char* str_rel   = strtok_r(nullptr, " \t", &state);
	char* str_scale = strtok_r(nullptr, " \t", &state);

	if(!str_axis || !str_act){
		return false;
	}

	Input::Axis axis(str_axis);
	if(axis.type == Input::Axis::AXIS_INVALID){
		return false;
	}

	bool rel = true;
	if(str_rel){
		rel = str_to_bool(str_rel);
	}

	float scale = 1.0f;
	if(str_scale){
		scale = strtof(str_scale, nullptr);
		if(abs(scale) < FLT_EPSILON || errno == ERANGE){
			scale = 1.0f;
		}
	}

	input->bind(axis, str_hash(str_act), rel, scale);

	return true;
}

Input::Input(Engine& e)
: binds()
, active_binds()
, bound_actions()
, current_state(nullptr){

	using namespace std::placeholders;

	//TODO: can all these cvar functions be merged into one?
	e.cfg.addVar<CVarFunc>(
		"bind",
		std::bind(&bind_key_fn, this, _1, false),
		"Usage: bind <key_name> <action>"
	);

	e.cfg.addVar<CVarFunc>(
		"bind_raw",
		std::bind(&bind_key_fn, this, _1, true),
		"Usage: bind_raw <scancode> <action>"
	);

	e.cfg.addVar<CVarFunc>(
		"bind_axis",
		std::bind(&bind_axis_fn, this, _1),
		"Usage: bind_axis <axis_name> <action> [relative?] [scale]"
	);

	SDL_StopTextInput();
}

void Input::bind(Key key, strhash_t action){
	if(key.code != SDL_SCANCODE_UNKNOWN){
		binds.emplace(action, key);
		
		auto pair = bound_actions.equal_range(action);
		for(auto i = pair.first, j = pair.second; i != j; ++i){
			StateAction& sa = i->second;
			active_binds.emplace(StateBind{sa.state, key}, sa.id);
		}
	}
}

//TODO: merge this function with the other bind?
void Input::bind(Axis axis, strhash_t action, bool rel, float scale){
	if(axis.type != Axis::AXIS_INVALID){
		binds.emplace(action, Binding{ axis, rel, scale });

		auto pair = bound_actions.equal_range(action);
		for(auto i = pair.first, j = pair.second; i != j; ++i){
			StateAction& sa = i->second;
			active_binds.emplace(StateBind{sa.state, Binding{ axis, rel, scale}}, sa.id);
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
			if(it->first.bind == key){
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
	} else {
		SDL_StopTextInput();
	}

	current_state = s;
}

void Input::watchAction(GameState* s, const str_const& action, int action_id){

	bound_actions.emplace(action.hash, StateAction{s, action_id});

	auto it = binds.find(action.hash);
	if(it != binds.end()){
		active_binds.emplace(StateBind{s, it->second}, action_id);
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

bool Input::getKeyAction(GameState* s, const Key& key, int& act_id){

	auto it = active_binds.find(StateBind{s, key});
	
	if(it != active_binds.end()){
		act_id = it->second;
		return true;
	} else {
		return false;
	}
}

bool Input::getAxisAction(GameState* s, const Axis& a, int& act_id, bool& rel, float& scale){

	auto it = active_binds.find(StateBind{s, Binding{a, 0, 0}});
	
	if(it != active_binds.end()){
		act_id = it->second;
		rel = it->first.bind.data.axis.relative;
		scale = it->first.bind.data.axis.scale;
		return true;
	} else {
		return false;
	}
}

/*bool Input::getPadAction(GameState* s, SDL_JoystickID id, int button, int& action_id){
	// TODO
	return false;
}*/


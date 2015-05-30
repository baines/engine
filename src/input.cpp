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
			if(*p == '*'){
				ignore_mods = true;
				shift = ctrl = alt = false;
				break;
			}
			if(*p == 's' || *p == 'S') shift = true;
			if(*p == 'c' || *p == 'C') ctrl  = true;
			if(*p == 'a' || *p == 'A') alt   = true;
		}
		str = mod_separator+1;
	}

	if(raw_scancode){
		code = strtol(str, nullptr, 0);
	} else {
		if(strncasecmp(str, "mb", 2) == 0){
			code = KEY_MOUSEBUTTON_BIT | strtoul(str + 2, nullptr, 10);
		} else if(strncasecmp(str, "mwheel", 6) == 0) {
			switch(str[6]){
				case 'u':
				case 'U':
					code = KEY_MOUSEWHEEL_BIT;
					break;
				case 'd':
				case 'D':
					code = KEY_MOUSEWHEEL_BIT | 1;
					break;
			}
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

size_t Input::Key::toString(char* buf, size_t len){
	if(!buf || len < 8) return 0;
	const char* orig_buf = buf;

	if(ignore_mods) *buf++ = '*';
	if(shift)       *buf++ = 'S';
	if(ctrl)        *buf++ = 'C';
	if(alt)         *buf++ = 'A';
	if(buf != orig_buf) *buf++ = '-';

	len -= (buf - orig_buf);

	if(code & KEY_MOUSEBUTTON_BIT){
		buf += max<int>(0, snprintf(buf, len, "mb%d", code - KEY_MOUSEBUTTON_BIT));
	} else if(code & KEY_MOUSEWHEEL_BIT){
		buf += max<int>(0, snprintf(buf, len, "mwheel%s", code & 1 ? "down" : "up"));
	} else if(code & KEY_GAMEPAD_BIT){
		size_t i = code - KEY_GAMEPAD_BIT;

		if(i < SDL_arraysize(pad_buttons)){
			buf += max<int>(0, snprintf(buf, len, "pad_%s", pad_buttons[i]));
		} else {
			buf += SDL_strlcpy(buf, "[unknown pad btn]", len);	
		}
	} else {
		buf += SDL_strlcpy(buf, SDL_GetScancodeName(static_cast<SDL_Scancode>(code)), len);
	}

	return buf - orig_buf;
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

constexpr struct axis_map {
	Input::Axis::AxisType type;
	const char* name;
} axes[] = {
	{ Input::Axis::AXIS_MOUSE, "mouse_x" },
	{ Input::Axis::AXIS_MOUSE, "mouse_y" },
	{ Input::Axis::AXIS_PAD,   "pad_ls_x" },
	{ Input::Axis::AXIS_PAD,   "pad_ls_y" },
	{ Input::Axis::AXIS_PAD,   "pad_rs_x" },
	{ Input::Axis::AXIS_PAD,   "pad_rs_y" },
	{ Input::Axis::AXIS_PAD,   "pad_lt" },
	{ Input::Axis::AXIS_PAD,   "pad_rt" }
};

Input::Axis::Axis(const char* str)
: type(AXIS_INVALID)
, device_index()
, axis_index() {
	for(size_t i = 0; i < SDL_arraysize(axes); ++i){
		if(strcasecmp(str, axes[i].name) == 0){
			type = axes[i].type;
			device_index = 0;   //TODO: multiple pads / mice.
			axis_index = i;
			break;
		}
	}
}

size_t Input::Axis::toString(char* buf, size_t len){
	for(size_t i = 0; i < SDL_arraysize(axes); ++i){
		if(type == axes[i].type){
			return SDL_strlcpy(buf, axes[i].name, len);
		}
	}
	return SDL_strlcpy(buf, "[unknown axis]", len);
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

bool Input::Binding::operator==(const Binding& b) const {
	if(type != b.type) return false;
	if(type == BINDING_KEY){
		return data.key == b.data.key;
	} else {
		return data.axis.axis == b.data.axis.axis;
	}
}

bool Input::Binding::operator==(const Key& k) const {
	return type == BINDING_KEY && data.key == k;
}

bool Input::Binding::operator==(const Axis& a) const {
	return type == BINDING_AXIS && data.axis.axis == a;
}

size_t Input::Binding::toString(char* buf, size_t len){
	if(type == BINDING_KEY){
		return data.key.toString(buf, len);
	} else {
		return data.axis.axis.toString(buf, len);
	}
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
		input->bind(Input::Key(key, raw), act);
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

	input->bind(axis, str_act, rel, scale);

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

	e.cfg.addVar<CVarFunc>("bindlist", [&](const string_view&){
		char bind_buf[32] = {};
		size_t bind_len = sizeof(bind_buf);

		for(auto& bp : binds){
			bp.second.toString(bind_buf, bind_len);

			const char* action = "[unknown action]";
			auto it = action_names.find(bp.first);
			if(it != action_names.end()){
				action = it->second.c_str();
			}

			e.cli.printf("%12s: %s\n", bind_buf, action);
		}

		return true;
	}, "Lists the current set of keybindings.");

	e.cfg.addVar<CVarFunc>("unbind", [&](const string_view& arg){
		for(auto& bp : binds){
			char bind_buf[32] = {};
			bp.second.toString(bind_buf, sizeof(bind_buf));

			if(strncasecmp(arg.data(), bind_buf, arg.size()) == 0){
				unbind(bp.second);
				return true;
			}
		}
		e.cli.printf("\"%.*s\" isn't bound.\n", (int)arg.size(), arg.data());
		return true;
	}, "Usage: unbind <key/axis>.");

	SDL_StopTextInput();
}

void Input::bind(const Key& key, const string_view& action){
	strhash_t act_hash = str_hash_len(action.data(), action.size());
	action_names.emplace(
		piecewise_construct,
		forward_as_tuple(act_hash),
		forward_as_tuple(action.data(), action.size())
	);

	if(key.code != SDL_SCANCODE_UNKNOWN){
		binds.emplace(act_hash, key);
		
		auto pair = bound_actions.equal_range(act_hash);
		for(auto i = pair.first, j = pair.second; i != j; ++i){
			StateAction& sa = i->second;
			active_binds.emplace(StateBind{sa.state, key}, sa.id);
		}
	}
}

//TODO: merge this function with the other bind?
void Input::bind(const Axis& axis, const string_view& action, bool rel, float scale){
	strhash_t act_hash = str_hash_len(action.data(), action.size());
	action_names.emplace(
		piecewise_construct,
		forward_as_tuple(act_hash),
		forward_as_tuple(action.data(), action.size())
	);

	if(axis.type != Axis::AXIS_INVALID){
		binds.emplace(act_hash, Binding{ axis, rel, scale });

		auto pair = bound_actions.equal_range(act_hash);
		for(auto i = pair.first, j = pair.second; i != j; ++i){
			StateAction& sa = i->second;
			active_binds.emplace(StateBind{sa.state, Binding{ axis, rel, scale}}, sa.id);
		}
	}
}

void Input::unbind(const Binding& b){
	for(auto it = binds.begin(); it != binds.end(); /**/){
		if(it->second == b){
			binds.erase(it++);
		} else {
			++it;
		}
	}
	for(auto it = active_binds.begin(); it != active_binds.end(); /**/){
		if(it->first.bind == b){
			active_binds.erase(it++);
		} else {
			++it;
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

void Input::subscribe(GameState* s, const str_const& action, int action_id){

	bound_actions.emplace(action.hash, StateAction{s, action_id});

	auto it_pair = binds.equal_range(action.hash);
	for(; it_pair.first != it_pair.second; ++it_pair.first){
		active_binds.emplace(StateBind{s, it_pair.first->second}, action_id);
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


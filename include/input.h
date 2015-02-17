#ifndef INPUT_H_
#define INPUT_H_
#include "common.h"
#include "util.h"
#include <SDL2/SDL.h>
#include "game_state.h"
#include <map>
#include <unordered_map>
#include <string>

static constexpr struct mouse_button_tag_t {} mouse_button_tag;
static constexpr struct mouse_wheel_tag_t {} mouse_wheel_tag;
static constexpr struct pad_button_tag_t {} pad_button_tag;
static constexpr struct mouse_tag_t {} mouse_tag;

struct Input {
	
	struct Key {
		Key() = default;
		
		Key(SDL_Scancode code);
		Key(const SDL_Keysym& key);
		
		Key(mouse_button_tag_t, uint8_t button); //TODO: multiple mice.
		Key(mouse_wheel_tag_t, int dir);

		Key(pad_button_tag_t, uint8_t button);
		
		Key(const char* str, bool raw_scancode = false);
		
		bool operator<(const Key& k) const;
		bool operator==(const Key& k) const;

		uint32_t code;
		bool shift, ctrl, alt, ignore_mods;
	};

	struct Axis {
		Axis() = default;
		
		Axis(SDL_GameControllerAxis pad_axis);
		
		Axis(mouse_tag_t, int axis);
		
		Axis(const char* str);
		
		bool operator<(const Axis& a) const;
		bool operator==(const Axis& a) const;

		enum AxisType {
			AXIS_INVALID,
			AXIS_MOUSE,
			AXIS_PAD
		} type;
		int device_index;
		int axis_index;
	};

	Input(Engine& e);

	void bind(Key key, strhash_t action);
	void bind(Axis axis, strhash_t action, bool rel, float scale = 1.0f);
	void unbind(Key key);

	void watchAction(GameState* s, const str_const& action, int action_id);
	void enableText(GameState* s, bool enable, const SDL_Rect& pos = {0, 0, 0, 0});

	void onDeviceChange(SDL_ControllerDeviceEvent& event);
	void onStateChange(GameState* new_state);

	bool getKeyAction(GameState* s, const Key& key, int& act_id);
	//bool getPadAction(GameState* s, SDL_JoystickID id, int button, int& action_id);
	bool getAxisAction(GameState* s, const Axis& a, int& act_id, bool& rel, float& scale);	

private:
	struct Binding {
		Binding(Key k);
		Binding(Axis a, bool rel, float scale);

		enum {
			BINDING_KEY,
			BINDING_AXIS
		} type;
		union {
			Key key;
			struct {
				Axis axis;
				bool relative;
				float scale;
			} axis;
		} data;
		bool operator<(const Binding& rhs) const;
		bool operator==(const Key& k) const;
		bool operator==(const Axis& a) const;
	};
	struct StateAction {
		GameState* state;
		int id;
	};
	struct StateBind {
		GameState* state;
		Binding bind;
		bool operator<(const StateBind& rhs) const;
	};
	
	std::map<strhash_t, Binding> binds;
	std::map<StateBind, int> active_binds;
	std::multimap<strhash_t, StateAction> bound_actions;

	std::unordered_map<GameState*, SDL_Rect> text_states;
		
	GameState* current_state;
};

#endif

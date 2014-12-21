#ifndef INPUT_H_
#define INPUT_H_
#include "common.h"
#include "util.h"
#include <SDL2/SDL.h>
#include "game_state.h"
#include <map>
#include <unordered_map>
#include <string>

struct Input {
	
	struct Key {
		Key();
		Key(SDL_Scancode code);
		Key(const SDL_Keysym& key);
		Key(const char* str, bool raw_scancode = false);
		bool operator<(const Key& k) const;
		bool operator==(const Key& k) const;

		SDL_Scancode code;
		bool shift, ctrl, alt;
	};

	Input(Engine& e);

	void bind(Key key, uint32_t action);
	void unbind(Key key);

	void watchAction(GameState* s, const str_const& action, int action_id);
	void enableText(GameState* s, bool enable, const SDL_Rect& pos = {0, 0, 0, 0});

	void onDeviceChange(SDL_ControllerDeviceEvent& event);
	void onStateChange(GameState* new_state);

	bool getKeyAction(GameState* s, const Key& key, int& action_id);
	bool getPadAction(GameState* s, SDL_JoystickID id, int button, int& action_id);

private:
	struct StateAction {
		GameState* state;
		int id;
	};
	struct StateKey {
		GameState* state;
		Key key;
		bool operator<(const StateKey& rhs) const;
	};

	std::map<uint32_t, Key> binds;
	std::map<StateKey, int> active_binds;
	std::multimap<uint32_t, StateAction> bound_actions;

	std::unordered_map<GameState*, SDL_Rect> text_states;
		
	GameState* current_state;
};

#endif

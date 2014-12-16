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
	
	Input(Engine& e);

	void bind(const char* input_name, uint32_t action);
	void bindRaw(SDL_Scancode key, uint32_t action);
	
	void unbind(const char* input_name);
	void unbindRaw(SDL_Scancode key);

	void watchAction(GameState* s, const str_const& action, int action_id);
	void enableText(GameState* s, bool enable, const SDL_Rect& pos = {0, 0, 0, 0});

	void onDeviceChange(SDL_ControllerDeviceEvent& event);
	void onStateChange(GameState* new_state);

	bool getKeyAction(GameState* s, SDL_Scancode key, int& action_id);
	bool getPadAction(GameState* s, SDL_JoystickID id, int button, int& action_id);

private:

	struct StateAction {
		GameState* state;
		int id;
	};
	struct StateKey {
		GameState* state;
		SDL_Scancode key;
		bool operator<(const StateKey& rhs) const {
			if(state == rhs.state){
				return key < rhs.key;
			} else {
				return state < rhs.state;
			}
		}
	};

	std::map<uint32_t, SDL_Scancode> binds;
	std::map<StateKey, int> active_binds;
	std::multimap<uint32_t, StateAction> bound_actions;

	std::unordered_map<GameState*, SDL_Rect> text_states;
		
	GameState* current_state;
};

#endif

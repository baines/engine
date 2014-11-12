#ifndef INPUT_H_
#define INPUT_H_
#include <SDL2/SDL.h>
#include "game_state.h"
#include <map>
#include <string>

struct Input {

	void bind(const char* input_name, const char* action);
	void bindRaw(SDL_Scancode key, const char* action);
	
	void unbind(const char* input_name);
	void unbindRaw(SDL_Scancode key);

	void watchAction(GameState* s, const char* action, int action_id);

	void addDevice(int id);
	void delDevice(int id);

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

	std::map<std::string, SDL_Scancode> binds;
	std::map<StateKey, int> active_binds;
	std::multimap<std::string, StateAction> watches;
};

#endif

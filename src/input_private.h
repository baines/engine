#ifndef INPUT_PRIVATE_H_
#define INPUT_PRIVATE_H_
#include "input.h"
#include <SDL_events.h>
#include <SDL_keyboard.h>
#include <SDL_gamecontroller.h>
#include <map>
#include <unordered_map>

struct Input : public IInput {
	Input(Engine& e);

	void bind(const InputKey& key, const StrRef& action);
	void bind(const InputAxis& axis, const StrRef& action, bool rel, float scale = 1.0f);

	void subscribe(GameState* s, const str_const& action, int action_id);
	void subscribe(GameState* s, const str_const& action, int action_id, const InputKey& dflt);
	void enableText(GameState* s, bool enable, const SDL_Rect& pos = {0, 0, 0, 0});

	void onDeviceChange(SDL_ControllerDeviceEvent& event);
	void onStateChange(GameState* new_state);

	bool getKeyAction(GameState* s, const InputKey& key, int& act_id);
	//bool getPadAction(GameState* s, SDL_JoystickID id, int button, int& action_id);
	bool getAxisAction(GameState* s, const InputAxis& a, int& act_id, bool& rel, float& scale);	
private:
	struct Binding {
		Binding(InputKey k);
		Binding(InputAxis a, bool rel, float scale);

		enum {
			BINDING_KEY,
			BINDING_AXIS
		} type;
		union {
			InputKey key;
			struct {
				InputAxis axis;
				bool relative;
				float scale;
			} axis;
		} data;
		bool operator<(const Binding& rhs) const;
		bool operator==(const Binding& b) const;
		bool operator==(const InputKey& k) const;
		bool operator==(const InputAxis& a) const;
		size_t toString(char* buf, size_t len);
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

	void unbind(const Binding& b);
	
	std::multimap<strhash_t, Binding> binds;
	std::map<StateBind, int> active_binds;
	std::multimap<strhash_t, StateAction> bound_actions;

	std::map<strhash_t, StrMut> action_names;
	
	std::unordered_map<GameState*, SDL_Rect> text_states;
	GameState* current_state;
};

#endif


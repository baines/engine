#ifndef STATE_SYSTEM_H_
#define STATE_SYSTEM_H_
#include "common.h"
#include <SDL.h>
#include <vector>

struct StateSystem {
	StateSystem();
	
	void push(GameState* s);
	void pop(int amount);
	
	void onInput(Engine& e, SDL_Event& event);
	void onMotion(Engine& e, SDL_Event& event);
	void onText(Engine& e, SDL_TextInputEvent& event);

	void onResize(Engine& e, int w, int h);

	void processStateChanges(Engine& e);
	void update(Engine& e, uint32_t delta);
	void draw(Renderer& r);
private:
	std::vector<GameState*> states, new_states;
	int pop_num;
};

#endif

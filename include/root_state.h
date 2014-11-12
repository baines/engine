#ifndef ROOT_STATE_H_
#define ROOT_STATE_H_
#include "game_state.h"

struct RootState : public GameState {
	RootState(Engine& e);
	void update(Engine& e);
	void draw(Renderer& r);
	bool onInput(int action_id, bool pressed);
private:
	Engine& engine;
};

#endif

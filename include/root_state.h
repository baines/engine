#ifndef ROOT_STATE_H_
#define ROOT_STATE_H_
#include "common.h"
#include "game_state.h"

struct RootState : public GameState {
	RootState(Engine& e);
	void update(Engine& e, uint32_t delta);
	void draw(Renderer& r);
	bool onInput(Engine& e, int action_id, bool pressed);
};

#endif

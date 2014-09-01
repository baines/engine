#ifndef GAMESTATE_H_
#define GAMESTATE_H_

struct Engine;

struct GameState {
	GameState(Engine& e);
	virtual void update(Engine& e) = 0;
	virtual bool onInput(int action_id, bool pressed) = 0;
	virtual ~GameState(){};
};

#endif

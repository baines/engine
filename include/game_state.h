#ifndef GAME_STATE_H_
#define GAME_STATE_H_

struct Engine;
struct Renderer;

struct GameState {
	GameState(Engine& e);
	virtual void update(Engine& e) = 0;
	virtual void draw(Renderer& r) = 0;
	virtual bool onInput(int action_id, bool pressed) = 0;
	virtual ~GameState(){};
};

#endif

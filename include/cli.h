#ifndef CLI_H_
#define CLI_H_
#include "common.h"
#include "game_state.h"

struct CLI : public GameState {
	CLI(Engine& e);
	
	virtual bool onInput(Engine& e, int action, bool pressed);
	virtual void update(Engine& e, uint32_t delta);
	virtual void draw(Renderer& r);
	
	void toggle(void);
	bool execute(const char* line);
	void printf(const char* fmt, ...);
	
	~CLI();
private:
	Engine& engine;
	bool active;
};

#endif


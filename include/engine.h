#ifndef ENGINE_H_
#define ENGINE_H_
#include "common.h"
#include "config.h"
#include "cli.h"
#include "input.h"
#include "renderer.h"
#include "resource_system.h"
#include "state_system.h"
#include "game_state.h"
#include "root_state.h"

struct Engine {

	Engine(int argc, char** argv, const char* name);
	void addState(GameState* s);
	bool run(void);
	
	void quit(void);
//	void log(const char* text);
	void showError(const char* text);
	~Engine();

	Config         cfg;
	Input          input;
	Renderer       renderer;
	ResourceSystem res;
	StateSystem    state;
	CLI            cli;
	
private:
	std::vector<GameState*> states;
	CVarInt* max_fps;
	bool running;
	uint32_t prev_ticks;
	RootState root_state;
};

#endif

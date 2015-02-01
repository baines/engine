#ifndef ENGINE_H_
#define ENGINE_H_
#include "common.h"
#include "config.h"
#include "cli.h"
#include "input.h"
#include "renderer.h"
#include "resource_system.h"
#include "text_system.h"
#include "collision_system.h"
#include "state_system.h"
#include "game_state.h"
#include "root_state.h"

struct Engine {

	Engine(int argc, char** argv, const char* name);
	void addState(GameState* s);
	bool run(void);
	void quit(void);
	~Engine();

	ResourceSystem   res;
	Config           cfg;
	Input            input;
	Renderer         renderer;
	TextSystem       text;
	CollisionSystem  collision;
	StateSystem      state;
	CLI              cli;
	
private:
	std::vector<GameState*> states;
	CVarInt* max_fps;
	bool running;
	uint32_t prev_ticks;
	RootState root_state;
};

#include "resource_impl.h"

#endif

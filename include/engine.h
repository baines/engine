#ifndef ENGINE_H_
#define ENGINE_H_
#include "config.h"
#include "cli.h"
#include "input.h"
#include "renderer.h"
#include "resource_system.h"

struct GameState;

struct Engine {

	Engine(int argc, char** argv, const char* name);
	void addState(GameState* s);
	bool run(void);
//	void log(const char* text);
	void showError(const char* text);
	~Engine();

	Config         cfg;
	CLI            cli;
	Input          input;
	Renderer       renderer;
	ResourceSystem res;
//	StateSystem    gamestate;
	
private:
//	RootState root_state;
	CVarInt* max_fps;
	bool running;
};

#endif

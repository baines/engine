#ifndef ENGINE_H_
#define ENGINE_H_
#include "common.h"
#include "cvar.h"

struct Engine {
	Engine(int argc, char** argv, const char* name);
	void addState(GameState* s);
	bool run(void);
	void quit(void);
	~Engine();

	std::unique_ptr<ResourceSystem>  res;
	std::unique_ptr<Config>          cfg;
	std::unique_ptr<Input>           input;
	std::unique_ptr<Renderer>        renderer;
	std::unique_ptr<TextSystem>      text;
	std::unique_ptr<CollisionSystem> collision;
	std::unique_ptr<StateSystem>     state;
	std::unique_ptr<CLI>             cli;
	
private:

	std::vector<GameState*> states;
	CVarInt* max_fps;
	bool running;
	uint32_t prev_ticks;
	std::unique_ptr<RootState> root_state;
};

#endif

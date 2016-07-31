#ifndef ENGINE_H_
#define ENGINE_H_
#include "common.h"
#include <vector>

struct Engine {
	Engine(int argc, char** argv, const char* name);
	void addState(GameState* s);
	bool run(void);
	void quit(void);
	~Engine();

	UniquePtr<ResourceSystem>  res;
	UniquePtr<Config>          cfg;
	UniquePtr<IInput>          input;
	UniquePtr<IRenderer>       renderer;
	UniquePtr<ITextSystem>     text;
	UniquePtr<CollisionSystem> collision;
	UniquePtr<StateSystem>     state;
	UniquePtr<ICLI>            cli;
	
private:
	std::vector<GameState*> states;
	CVarInt* max_fps;
	bool running;
	uint32_t prev_ticks;
	UniquePtr<RootState> root_state;
};

#endif

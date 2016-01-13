#ifndef ENGINE_H_
#define ENGINE_H_
#include "common.h"
#include <memory>
#include <vector>

struct Engine {
	Engine(int argc, char** argv, const char* name);
	void addState(GameState* s);
	bool run(void);
	void quit(void);
	~Engine();

	std::unique_ptr<ResourceSystem>  res;
	std::unique_ptr<Config>          cfg;
	std::unique_ptr<IInput>          input;
	std::unique_ptr<IRenderer>       renderer;
	std::unique_ptr<ITextSystem>     text;
	std::unique_ptr<CollisionSystem> collision;
	std::unique_ptr<StateSystem>     state;
	std::unique_ptr<ICLI>            cli;
	
private:

	std::vector<GameState*> states;
	CVarInt* max_fps;
	bool running;
	uint32_t prev_ticks;
	std::unique_ptr<RootState> root_state;
};

#endif

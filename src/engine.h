#ifndef _ENGINE_H_
#define _ENGINE_H_
#include "SDL/SDL.h"
#include "statemgr.h"
#include "renderer.h"
#include "input.h"
#include "console.h"
#include <vector>
	
class Engine {
public:
	Engine();
	bool run();
private:
	void handleKeyPress(Uint8 toggle);
	Renderer& renderer;
	StateMgr statemgr;
	Input input;
	Console console;
	
	bool running;
	SDL_Event e;
	uint32_t time, delta;
	std::vector<uint32_t> indices;
};

#endif

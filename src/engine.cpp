#include "engine.h"
#include "game_state.h"

Engine::Engine(int argc, char** argv, const char* name)
: res        (argv[0])
, cfg        (*this, argc, argv)
, input      (*this)
, renderer   (*this, name)
, text       (*this)
, state      ()
, cli        (*this)
, max_fps    (cfg.addVar("max_fps", CVarInt(200, 1, 1000)))
, running    (true)
, prev_ticks (0)
, root_state (*this) {
	state.push(&root_state);
}

void Engine::addState(GameState* s){
	state.push(s);
}

bool Engine::run(void){

	const int min_delta = 1000 / std::max(1, max_fps->val);
	int delta = SDL_GetTicks() - prev_ticks; //XXX: high res timer?
	
	if(delta < min_delta){
		SDL_Delay(min_delta - delta);
		delta = min_delta;
	}
	prev_ticks += delta;
	delta = std::min(delta, 100);
	
	SDL_Event e;
	
	while(SDL_PollEvent(&e)){
		switch(e.type){
			case SDL_KEYDOWN:
			case SDL_KEYUP:
			case SDL_CONTROLLERBUTTONDOWN:
			case SDL_CONTROLLERBUTTONUP:
			case SDL_MOUSEWHEEL:
				state.onInput(*this, e);
				break;
			case SDL_MOUSEMOTION:
			case SDL_CONTROLLERAXISMOTION:
				state.onMotion(*this, e);
				break;
			case SDL_CONTROLLERDEVICEADDED:
			case SDL_CONTROLLERDEVICEREMOVED:
			case SDL_CONTROLLERDEVICEREMAPPED:
				input.onDeviceChange(e.cdevice);
				break;
			case SDL_WINDOWEVENT: {
				if(e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED){
					renderer.handleResize(e.window.data1, e.window.data2);
				}
				break;
			}
			case SDL_QUIT:
				running = false;
			default:
				break;
		}
	}
	
	state.update(*this, delta);
	state.draw(renderer);
		
	renderer.drawFrame();

	return running;
}

void Engine::quit(){
	running = false;
}

Engine::~Engine(){
	SDL_Quit();
}


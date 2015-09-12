#include "engine.h"
#include "resource_system.h"
#include "config.h"
#include "input.h"
#include "renderer.h"
#include "text_system.h"
#include "collision_system.h"
#include "state_system.h"
#include "root_state.h"
#include "cli.h"
#include <algorithm>

namespace {
	static int get_ticks(){
#ifdef __EMSCRIPTEN__
		return std::round(emscripten_get_now());
#else
		return SDL_GetTicks();
#endif
	}
}

using std::make_unique;

Engine::Engine(int argc, char** argv, const char* name)
: res        (make_unique<ResourceSystem>(argv[0]))
, cfg        (make_unique<Config>(*this, argc, argv))
, input      (make_unique<Input>(*this))
, renderer   (make_unique<Renderer>(*this, name))
, text       (make_unique<TextSystem>(*this))
, collision  (make_unique<CollisionSystem>())
, state      (make_unique<StateSystem>())
, cli        (make_unique<CLI>(*this))
, max_fps    (cfg->addVar<CVarInt>("max_fps", 200, 1, 1000))
, running    (true)
, prev_ticks (0)
, root_state (make_unique<RootState>(*this)) {
	addState(root_state.get());
}

void Engine::addState(GameState* s){
	state->push(s);
}

bool Engine::run(void){

	TRACEF("---------- Frame Begin ----------");

	const int min_delta = 1000 / std::max(1, max_fps->val);
	int delta = std::max<int>(0, get_ticks() - prev_ticks);

	if(delta < min_delta){
		SDL_Delay(min_delta - delta);
		delta = min_delta;
	}
	prev_ticks += delta;

	delta = std::min(delta, 100);

	state->processStateChanges(*this);

	SDL_Event e;
	
	while(SDL_PollEvent(&e)){
		switch(e.type){
			case SDL_KEYDOWN:
			case SDL_KEYUP:
			case SDL_CONTROLLERBUTTONDOWN:
			case SDL_CONTROLLERBUTTONUP:
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
			case SDL_MOUSEWHEEL:
				state->onInput(*this, e);
				break;
			case SDL_TEXTINPUT:
				state->onText(*this, e.text);
				break;
			case SDL_MOUSEMOTION:
			case SDL_CONTROLLERAXISMOTION:
				state->onMotion(*this, e);
				break;
			case SDL_CONTROLLERDEVICEADDED:
			case SDL_CONTROLLERDEVICEREMOVED:
			case SDL_CONTROLLERDEVICEREMAPPED:
				input->onDeviceChange(e.cdevice);
				break;
			case SDL_WINDOWEVENT: {
				if(e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED){
					renderer->handleResize(e.window.data1, e.window.data2);
					state->onResize(*this, e.window.data1, e.window.data2);
				}
				break;
			}
			case SDL_QUIT:
				running = false;
			default:
				break;
		}
	}
	
	state->update(*this, delta);
	collision->update(delta);
	state->draw(*renderer);
		
	renderer->drawFrame();

	TRACEF("---------- Frame End ----------");

	return running;
}

void Engine::quit(){
	running = false;
}

Engine::~Engine(){
	SDL_Quit();
}


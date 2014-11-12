#include "engine.h"
#include "game_state.h"

Engine::Engine(int argc, char** argv, const char* name)
: cfg()
, cli()
, input()
, renderer(*this, name)
, res()
, max_fps(cfg.addVar("max_fps", CVarInt(200, 1, 1000)))
, running(true)
, prev_ticks(0)
, root_state(*this) {
	cfg.load(argc, argv);
	input.bindRaw(SDL_SCANCODE_ESCAPE, "QUIT");
	
	states.push_back(&root_state);
}

void Engine::addState(GameState* s){
	states.push_back(s);
}

bool Engine::run(void){

	const int min_delta = 1000 / std::max(1, max_fps->val);
	if(int delta = (SDL_GetTicks() - prev_ticks) < min_delta){
		SDL_Delay(min_delta - delta);
		prev_ticks += delta;
	}
	
	SDL_Event e;
	
	while(SDL_PollEvent(&e)){
		switch(e.type){
			case SDL_KEYDOWN:
			case SDL_KEYUP: {
				int action = 0;
				for(size_t i = states.size()-1; i >= 0; --i){
					if(input.getKeyAction(states[i], e.key.keysym.scancode, action)){
						if(states[i]->onInput(action, e.key.state)){
							break;
						}
					}
				}
				break;
			}
			case SDL_MOUSEMOTION:
				//TODO
				break;
			case SDL_WINDOWEVENT:
				renderer.onWindowEvent(e.window);
				break;
			case SDL_QUIT:
				running = false;
			default:
				break;
		}	
	}

	return running;
}

void Engine::quit(){
	running = false;
}

void Engine::showError(const char* text){
	SDL_MessageBoxColorScheme colors = {{
		{ 27 , 24 , 37  }, // bg
		{ 200, 200, 200 }, // text
		{ 200, 200, 200 }, // button border
		{ 27 , 24 , 37  }, // button bg
		{ 255, 255, 255 }  // button selected
	}};
	
	SDL_MessageBoxButtonData button = { 3, 0, "Ok :(" };
	
	SDL_MessageBoxData msg = {
		SDL_MESSAGEBOX_ERROR,
		renderer.getWindow(),
		"Fatal Error!",
		text,
		1,
		&button,
		&colors
	};
	
	SDL_ShowMessageBox(&msg, nullptr);
}

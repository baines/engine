#include "engine.h"
#include "gamestate.h"

Engine::Engine(int argc, char** argv, const char* name)
: cfg()
, cli()
, input()
, renderer(*this, name)
, res()
, max_fps(cfg.addVar("max_fps", CVarInt(200, 1, 1000)))
, running(true) {
	cfg.load(argc, argv);
//	state.add(&root_state);
}

void Engine::addState(GameState* s){
//	gamestate.add(s);
}

bool Engine::run(void){
/*
	SDL_Event e;
	
	//if(delta < max_fps e.t.c. delay
	
	while(SDL_PollEvent(&e)){
		switch(e.type){
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				state.onKeyEvent(e.key);
				break;
			case SDL_MOUSEMOTION:
				state.onMouseMotion(e.motion);
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
*/
	return running;
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

#include <ctime>
#include "engine.h"
#include "util.h"
#include "title.h"

Engine::Engine() : running(true), time(SDL_GetTicks()), delta(0),
	renderer(Renderer::get()) {
	indices.reserve(1024);	
	statemgr.push(new Title(statemgr));
}

bool Engine::run(){	
	while (SDL_PollEvent(&e)){
		switch (e.type){
			case SDL_QUIT: 
				running = false; 
			break;
			case SDL_KEYUP:
			case SDL_KEYDOWN:
				input.mods = e.key.keysym.mod;
				handleKeyPress(e.key.state);
			break;
			case SDL_MOUSEBUTTONUP:
			case SDL_MOUSEBUTTONDOWN:
				input.mouseClick(e.button.button, e.button.state);
			break;
			case SDL_MOUSEMOTION:
				input.mouseMove(e.motion.x, e.motion.y);
			break;				
		}
	}
	
	while((delta = SDL_GetTicks() - time) < (renderer.vsync ? 1 : 16))
		SDL_Delay(1);
	time += delta;
	
	statemgr.update(input, delta);
	statemgr.draw(indices);
	renderer.draw(indices);
	
	return running;
}

void Engine::handleKeyPress(Uint8 toggle){
	switch(e.key.keysym.sym) {
		case SDLK_ESCAPE:
			running = false;
			break;
		case SDLK_RETURN: 
			if(toggle && (e.key.keysym.mod & KMOD_ALT)) {
			    SDL_ShowCursor(SDL_ShowCursor(-1) ^ 1);
				renderer.toggleFullscreen();
			}
			break;
		default:
			break;
	}
	if(input.text && toggle) input.addText(e.key.keysym);
}

int main(int argc, char **argv){
	srand(time(0));
	Engine engine;
	while(engine.run());
	
	return 0;
}

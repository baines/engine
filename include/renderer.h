#ifndef RENDERER_H_
#define RENDERER_H_
#include <SDL2/SDL.h>
#include <vector>
#include "renderer/shader.h"
#include "renderer/vertex_state.h"
#include "renderer/texture.h"
#include "renderer/sampler.h"
#include "renderer/renderable.h"
#include "renderer/render_state.h"
#include "cvar.h"

struct Engine;
struct Renderable;

struct Renderer {
	Renderer(Engine& e, const char* name);
	void reload(Engine& e);
	void onWindowEvent(SDL_WindowEvent& ev);
	void drawFrame();
	
	void addRenderable(Renderable& r);
	void delRenderable(Renderable& r);
	
	SDL_Window* getWindow() const {
		return window;
	}
	
	~Renderer();
private:
	std::vector<Renderable*> renderables;
	RenderState render_state;
		
	CVarEnum* buff_orphan_mode;
	CVarString* libgl;
	CVarInt* window_width;
	CVarInt* window_height;
	
	const char* window_title;
	SDL_Window* window;
};

#endif

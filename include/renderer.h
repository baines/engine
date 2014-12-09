#ifndef RENDERER_H_
#define RENDERER_H_
#include "common.h"
#include <SDL2/SDL.h>
#include <vector>
#include "renderer/shader.h"
#include "renderer/vertex_state.h"
#include "renderer/texture.h"
#include "renderer/sampler.h"
#include "renderer/renderable.h"
#include "renderer/render_state.h"
#include "cvar.h"

struct Renderer {
	Renderer(Engine& e, const char* name);
	void reload(Engine& e);
	void handleResize(float w, float h);

	void drawFrame();
	
	void addRenderable(Renderable& r);
	
	SDL_Window* getWindow() const {
		return window;
	}
		
	~Renderer();
private:
	std::vector<Renderable*> renderables;
	std::vector<GLint> multi_offs;
	std::vector<GLsizei> multi_counts;
	
	RenderState render_state;
		
	CVarBool* gl_debug;
	CVarBool* gl_fwd_compat;
	CVarBool* gl_core_profile;
	CVarBool* gl_multi_draw;
	CVarString* libgl;
	CVarInt* window_width;
	CVarInt* window_height;
	CVarInt* vsync;
	CVarInt* fov;
	CVarBool* fullscreen;
	CVarBool* resizable;
	
	const char* window_title;
	SDL_Window* window;
	
	ShaderUniforms main_uniforms;
public:
	const int &window_w, &window_h;
};

#endif

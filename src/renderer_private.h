#ifndef RENDERER_PRIVATE_H_
#define RENDERER_PRIVATE_H_
#include "renderer.h"
#include <vector>
#include "shader_uniforms.h"
#include "render_state.h"

struct Renderer : public IRenderer {
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
	
	RenderState render_state;
		
	CVarBool* gl_debug;
	CVarBool* gl_fwd_compat;
	CVarBool* gl_core_profile;
	CVarString* libgl;
	CVarInt* window_width;
	CVarInt* window_height;
	CVarInt* vsync;
	CVarInt* fov;
	CVarInt* display_index;
	CVarBool* fullscreen;
	CVarBool* resizable;
	
	const char* window_title;
	SDL_Window* window;
	
	ShaderUniforms main_uniforms;
};

#endif

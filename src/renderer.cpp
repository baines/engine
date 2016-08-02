#include "renderer_private.h"
#include "engine.h"
#include "enums.h"
#include "config.h"
#include "cli.h"
#include "texture.h"
#include "sampler.h"
#include "renderable.h"
#include "shader.h"
#include "vertex_state.h"
#include "index_buffer.h"
#include <math.h>
#include <limits.h>
#include <SDL.h>

Renderer::Renderer(Engine& e, const char* name)
: renderables      ()
, render_state     ()
, gl_debug         (e.cfg->addVar<CVarBool>   ("gl_debug",          true))
, gl_fwd_compat    (e.cfg->addVar<CVarBool>   ("gl_fwd_compat",     true))
, gl_core_profile  (e.cfg->addVar<CVarBool>   ("gl_core_profile",   true))
, libgl            (e.cfg->addVar<CVarString> ("gl_library",        ""))
, window_width     (e.cfg->addVar<CVarInt>    ("vid_width" ,        640, 320, INT_MAX))
, window_height    (e.cfg->addVar<CVarInt>    ("vid_height",        480, 240, INT_MAX))
, vsync            (e.cfg->addVar<CVarInt>    ("vid_vsync",         1, -2, 2))
, fov              (e.cfg->addVar<CVarInt>    ("vid_fov",           90, 45, 135))
, display_index    (e.cfg->addVar<CVarInt>    ("vid_display_index", 0, 0, 100))
, fullscreen       (e.cfg->addVar<CVarBool>   ("vid_fullscreen",    false))
, resizable        (e.cfg->addVar<CVarBool>   ("vid_resizable",     true))
, window_title     (name)
, window           (nullptr)
, main_uniforms    () {

	SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");

	if(SDL_InitSubSystem(SDL_INIT_VIDEO) != 0){
		log(logging::fatal, "Couldn't initialize SDL video subsystem (%s).", SDL_GetError());
	}
	
	e.cfg->addVar<CVarFunc>("vid_reload", [&](const StrRef&){
		reload(e);
		return true;
	});

	std::initializer_list<CVar*> reload_vars = {
		gl_debug,      gl_fwd_compat, gl_core_profile, libgl,
		window_width,  window_height, vsync,           fov,
		display_index, fullscreen,    resizable
	};

	for(auto* v : reload_vars){
		v->setReloadVar("vid_reload");
	}

	e.cfg->addVar<CVarFunc>("vid_display_info", [&](const StrRef&){
		char buf[80] = {};
		SDL_Rect r;
		e.cli->echo("Displays:");

		int num_disp = SDL_GetNumVideoDisplays();
		for(int i = 0; i < num_disp; ++i){
			SDL_GetDisplayBounds(i, &r);
			const char* name = SDL_GetDisplayName(i);

			snprintf(buf, sizeof(buf), "  %d: [%dx%d+%d+%d] '%s'",
				i, r.w, r.h, r.x, r.y, name ? name : "(no name)"
			);
			e.cli->echo(reinterpret_cast<const char*>(buf));
		}
		return true;
	}, "Show info about available displays / monitors");

	reload(e);
}

void Renderer::reload(Engine& e){
	//TODO: check if we can get away with just using SDL_SetWindow{Size, Position} e.t.c.
	//      instead of destroying the window & GL context.

	gl.deleteContext();

	if(window){
		SDL_DestroyWindow(window);
		window = nullptr;
	}

	SDL_GL_UnloadLibrary();

	if(SDL_GL_LoadLibrary(libgl->str.empty() ? nullptr : libgl->str.c_str()) < 0){
		log(logging::fatal, "Couldn't load OpenGL library! (%s).", SDL_GetError());
	}

	render_state = {};

	SDL_GL_SetAttribute(SDL_GL_RED_SIZE     , 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE   , 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE    , 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE   , 8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER , 1);

	constexpr struct glversion {
		int maj, min;
	} ctx_versions[] = {
		{ 4, 5 }, { 4, 4 }, { 4, 3 }, { 4, 2 }, { 4, 1 }, { 4, 0 },
		{ 3, 3 }, { 3, 2 }, { 3, 1 }, { 3, 0 }, { 2, 1 }, { 2, 0 }
	};

	bool created = false;

	for(auto& v : ctx_versions){
#ifndef __EMSCRIPTEN__
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, v.maj);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, v.min);
		
		if(gl_core_profile->val && v.maj >= 3){
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		} else {
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_COMPATIBILITY);
		}
		
		int ctx_flags = 0;
		if(gl_debug->val){
			ctx_flags |= SDL_GL_CONTEXT_DEBUG_FLAG;
		}
		if(gl_fwd_compat->val && v.maj >= 3){
			ctx_flags |= SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG;
		}
		
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, ctx_flags);
#endif		
		int window_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN;
		if(fullscreen->val){
			window_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
		}
		if(resizable->val){
			window_flags |= SDL_WINDOW_RESIZABLE;
		}

		display_index->val = std::min(display_index->val, SDL_GetNumVideoDisplays()-1);

		window = SDL_CreateWindow(
			window_title,
			SDL_WINDOWPOS_CENTERED_DISPLAY(display_index->val),
			SDL_WINDOWPOS_CENTERED_DISPLAY(display_index->val),
			window_width->val,
			window_height->val,
			window_flags
		);

		if((created = gl.createContext(e, window))){
			break;
		} else {
			if(window) SDL_DestroyWindow(window);
		}
	}

	if(!created){
		log(logging::fatal, "Couldn't get an OpenGL context of any version!");
	}

	SDL_SetWindowMinimumSize(window, window_width->min, window_height->min);
	SDL_ShowWindow(window);

	gl.Enable(GL_BLEND);
	SDL_GL_SetSwapInterval(vsync->val);
	
	handleResize(window_width->val, window_height->val);
}

void Renderer::handleResize(float w, float h){
	window_width->val = w;
	window_height->val = h;
	
	if(gl.initialized()){
		gl.Viewport(0, 0, w, h);

		main_uniforms.setUniform("u_ortho", {
			alt::ortho(0.f, w, h, 0.f, -1.0f, 1.0f)
		});
		
		const float half_angle = (fov->val / 360.f) * M_PI;
		const float x_dist = tan(half_angle);
		const float y_dist = x_dist * (h/w);
		
		main_uniforms.setUniform("u_perspective", {
			alt::frustum(-x_dist, x_dist, -y_dist, y_dist, 1.0f, 1000.0f)
		});
	}
}

void Renderer::drawFrame(){

	gl.Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for(auto* r : renderables){
		VertexState* v = r->vertex_state;
		if(!v) continue;
	
		if(r->clip.x > 0 && r->clip.y > 0 && r->clip.w > 0 && r->clip.h > 0){
			//printf("scissor %d %d %d %d\n", r->clip.x, r->clip.y, r->clip.w, r->clip.h);
			gl.Scissor(r->clip.x, window_height->val - (r->clip.y+r->clip.h), r->clip.w, r->clip.h);
		} else {
			gl.Scissor(0, 0, window_width->val, window_height->val);
		}

		r->blend_mode.bind(render_state);
		
		if(ShaderProgram* s = r->shader){
			s->bind(render_state);
			s->setAttribs(render_state, *v);
			s->setUniforms(main_uniforms);
			
			if(ShaderUniforms* u = r->uniforms){
				s->setUniforms(*u);
			}
		}
		
		for(size_t i = 0; i < r->textures.size(); ++i){
			if(const Texture* t = r->textures[i]){
				t->bind(i, render_state);
			}
			if(const Sampler* s = r->samplers[i]){
				s->bind(i, render_state);
			}
		}
		
		v->bind(render_state);
				
		if(IndexBuffer* ib = v->getIndexBuffer()){
			gl.DrawElements(r->prim_type, r->count, ib->getType(), reinterpret_cast<GLvoid*>(r->offset));
		} else {
			gl.DrawArrays(r->prim_type, r->offset, r->count);
		}
	}

	SDL_GL_SwapWindow(window);

	renderables.clear();
}

void Renderer::addRenderable(Renderable& r){
	renderables.push_back(&r);
}

Renderer::~Renderer(){
	gl.deleteContext();
	
	if(window){
		SDL_DestroyWindow(window);
	}

	SDL_GL_UnloadLibrary();
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
}


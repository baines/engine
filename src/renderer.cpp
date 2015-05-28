#include "renderer.h"
#include "engine.h"
#include <climits>
#include "enums.h"
#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

Renderer::Renderer(Engine& e, const char* name)
: renderables      ()
, render_state     ()
, gl_debug         (e.cfg.addVar<CVarBool>   ("gl_debug",          true))
, gl_fwd_compat    (e.cfg.addVar<CVarBool>   ("gl_fwd_compat",     true))
, gl_core_profile  (e.cfg.addVar<CVarBool>   ("gl_core_profile",   true))
, gl_multi_draw    (e.cfg.addVar<CVarBool>   ("gl_multi_draw",     true))
, libgl            (e.cfg.addVar<CVarString> ("gl_library",        ""))
, window_width     (e.cfg.addVar<CVarInt>    ("vid_width" ,        640, 320, INT_MAX))
, window_height    (e.cfg.addVar<CVarInt>    ("vid_height",        480, 240, INT_MAX))
, vsync            (e.cfg.addVar<CVarInt>    ("vid_vsync",         1, -2, 2))
, fov              (e.cfg.addVar<CVarInt>    ("vid_fov",           90, 45, 135))
, display_index    (e.cfg.addVar<CVarInt>    ("vid_display_index", 0, 0, 100))
, fullscreen       (e.cfg.addVar<CVarBool>   ("vid_fullscreen",    false))
, resizable        (e.cfg.addVar<CVarBool>   ("vid_resizable",     true))
, window_title     (name)
, window           (nullptr)
, main_uniforms    ()
, window_w         (window_width->val)
, window_h         (window_height->val) {

	SDL_SetHint(SDL_HINT_VIDEO_MINIMIZE_ON_FOCUS_LOSS, "0");

	if(SDL_InitSubSystem(SDL_INIT_VIDEO) != 0){
		log(logging::fatal, "Couldn't initialize SDL video subsystem (%s).", SDL_GetError());
	}
	
	e.cfg.addVar<CVarFunc>("vid_reload", [&](const string_view&){
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

	e.cfg.addVar<CVarFunc>("vid_display_info", [&](const string_view&){
		char buf[80] = {};
		SDL_Rect r;
		e.cli.echo("Displays:");

		int num_disp = SDL_GetNumVideoDisplays();
		for(int i = 0; i < num_disp; ++i){
			SDL_GetDisplayBounds(i, &r);
			const char* name = SDL_GetDisplayName(i);

			snprintf(buf, sizeof(buf), "  %d: [%dx%d+%d+%d] '%s'",
				i, r.w, r.h, r.x, r.y, name ? name : "(no name)"
			);
			e.cli.echo(buf);
		}
		return true;
	}, "Show info about available displays / monitors");

	reload(e);
}

void Renderer::reload(Engine& e){
	//TODO: check if we can get away with just using SDL_SetWindow{Size, Position} e.t.c.
	//      instead of destroying the window & GL context.

	if(gl.initialized()){
		gl.deleteContext();
	}

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

	bool created = false;
		
	int window_flags = SDL_WINDOW_OPENGL;
	if(fullscreen->val){
		window_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	}
	if(resizable->val){
		window_flags |= SDL_WINDOW_RESIZABLE;
	}

	display_index->val = std::min(display_index->val, SDL_GetNumVideoDisplays()-1);

	window = SDL_CreateWindow(
		window_title,
		SDL_WINDOWPOS_UNDEFINED_DISPLAY(display_index->val),
		SDL_WINDOWPOS_UNDEFINED_DISPLAY(display_index->val),
		window_width->val,
		window_height->val,
		window_flags
	);
	
	if(!(created = gl.createContext(e, window))){
		log(logging::fatal, "Couldn't get an OpenGL context of any version!");
	}

	SDL_SetWindowMinimumSize(window, window_width->min, window_height->min);

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
			glm::ortho(0.f, w, h, 0.f)
		});
		
		const float half_angle = (fov->val / 360.f) * M_PI;
		const float x_dist = tan(half_angle);
		const float y_dist = x_dist * (h/w);
		
		main_uniforms.setUniform("u_perspective", {
			glm::frustum(-x_dist, x_dist, -y_dist, y_dist, 1.0f, 1000.0f)
		});
	}
}

void Renderer::drawFrame(){

	gl.Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for(auto i = renderables.begin(), j = renderables.end(); i != j; ++i){
		auto* r = *i;
		
		VertexState* v = r->vertex_state;
		if(!v) continue;
		
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


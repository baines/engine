#include "renderer.h"
#include "engine.h"
#include <climits>
#include "enums.h"
#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

namespace {

static const char* gl_dbgsrc2str(GLenum src){
	switch(src){
		case GL_DEBUG_SOURCE_API:             return "API";
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   return "WM";
		case GL_DEBUG_SOURCE_SHADER_COMPILER: return "SHDR";
		case GL_DEBUG_SOURCE_THIRD_PARTY:     return "3RD";
		case GL_DEBUG_SOURCE_APPLICATION:     return "APP";
		default:
		case GL_DEBUG_SOURCE_OTHER_ARB:       return "OTHER";
	}
}

static const char* gl_dbgtype2str(GLenum type){
	switch(type){
		case GL_DEBUG_TYPE_ERROR:               return "ERR";
		case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "DEPREC";
		case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  return "UNDEF";
		case GL_DEBUG_TYPE_PORTABILITY:         return "PORT";
		case GL_DEBUG_TYPE_PERFORMANCE:         return "PERF";
		default:
		case GL_DEBUG_TYPE_OTHER:               return "OTHER";
	}
}

static const char* gl_dbgsev2str(GLenum sev){
	switch(sev){
		case GL_DEBUG_SEVERITY_HIGH:   return "HIGH";
		case GL_DEBUG_SEVERITY_MEDIUM: return "MED";
		default:
		case GL_DEBUG_SEVERITY_LOW:    return "LOW";
	}
}

static void gl_dbg_callback(GLenum src, GLenum type, GLuint id, GLenum sev, 
GLsizei len, const char* msg, const void* p){
	log(logging::debug,
	    "[GL-%s-%s-%s] [%u] %s\n",
	    gl_dbgsrc2str(src),
	    gl_dbgtype2str(type),
	    gl_dbgsev2str(sev),
	    id,
	    msg
	);
}

}

Renderer::Renderer(Engine& e, const char* name)
: renderables      ()
, render_state     ()
, gl_debug         (e.cfg.addVar("gl_debug",        CVarBool(true)))
, gl_fwd_compat    (e.cfg.addVar("gl_fwd_compat",   CVarBool(true)))
, gl_core_profile  (e.cfg.addVar("gl_core_profile", CVarBool(true)))
, gl_multi_draw    (e.cfg.addVar("gl_multi_draw",   CVarBool(true)))
, libgl            (e.cfg.addVar("gl_library",      CVarString("")))
, window_width     (e.cfg.addVar("vid_width" ,      CVarInt(640, 320, INT_MAX)))
, window_height    (e.cfg.addVar("vid_height",      CVarInt(480, 240, INT_MAX)))
, vsync            (e.cfg.addVar("vid_vsync",       CVarInt(1, -2, 2)))
, fov              (e.cfg.addVar("vid_fov",         CVarInt(90, 45, 135)))
, fullscreen       (e.cfg.addVar("vid_fullscreen",  CVarBool(false)))
, resizable        (e.cfg.addVar("vid_resizable",   CVarBool(true)))
, window_title     (name)
, window           (nullptr)
, main_uniforms    ()
, window_w         (window_width->val)
, window_h         (window_height->val) {
	SDL_InitSubSystem(SDL_INIT_VIDEO);
	
	if(SDL_GL_LoadLibrary(libgl->str.empty() ? nullptr : libgl->str.c_str()) < 0){
		log(logging::fatal, "Couldn't load OpenGL library! (%s).", SDL_GetError());
	}
	
	reload(e);
}

void Renderer::reload(Engine& e){

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
		
	for(size_t i = 0; i < SDL_arraysize(ctx_versions); ++i){
		const glversion& v = ctx_versions[i];
		
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
		
		int window_flags = SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN;
		if(fullscreen->val){
			window_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
		}
		if(resizable->val){
			window_flags |= SDL_WINDOW_RESIZABLE;
		}
	
		window = SDL_CreateWindow(
			window_title,
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			window_width->val,
			window_height->val,
			window_flags
		);
		
		if((created = gl.createContext(e, window))){
			break;
		} else {
			SDL_DestroyWindow(window);
		}
	}
	
	if(!created){
		log(logging::fatal, "Couldn't get an OpenGL context of any version!");
	}
	
	SDL_ShowWindow(window);
	
	if(gl.DebugMessageCallback){
		gl.DebugMessageCallback(&gl_dbg_callback, nullptr);
		gl.Enable(GL_DEBUG_OUTPUT);
	}
	
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
				
		if(gl_multi_draw->val){
			int num_calls = 1;
			multi_counts.push_back(r->count);
			multi_offs.push_back(r->offset);
		
			for(auto k = i+1; k != j; ++k){
				if((*k)->usesSameState(*r)){
					multi_counts.push_back((*k)->count);
					multi_offs.push_back((*k)->offset);
					++num_calls;
				} else {
					break;
				}
			}
		
			if(IndexBuffer* ib = v->getIndexBuffer()){
				auto offs = reinterpret_cast<const void* const*>(multi_offs.data());
				gl.MultiDrawElements(r->prim_type, multi_counts.data(), ib->getType(), offs, num_calls);
			} else {
				gl.MultiDrawArrays(r->prim_type, multi_offs.data(), multi_counts.data(), num_calls);
			}
			
			multi_counts.clear();
			multi_offs.clear();
			
		} else {
			if(IndexBuffer* ib = v->getIndexBuffer()){
				gl.DrawElements(r->prim_type, r->count, ib->getType(), reinterpret_cast<GLvoid*>(r->offset));
			} else {
				gl.DrawArrays(r->prim_type, r->offset, r->count);
			}
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
	SDL_GL_UnloadLibrary();
	
	if(window){
		SDL_DestroyWindow(window);
	}
	
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
}


#include "renderer.h"
#include "engine.h"
#include <climits>
#include "enums.h"

Renderer::Renderer(Engine& e, const char* name)
: renderables      ()
, render_state     ()
, libgl            (e.cfg.addVar("vid_libgl", CVarString("")))
, window_width     (e.cfg.addVar("vid_width" , CVarInt(640, 320, INT_MAX)))
, window_height    (e.cfg.addVar("vid_height", CVarInt(480, 240, INT_MAX)))
, window_title     (name)
, window           (nullptr) {
	SDL_InitSubSystem(SDL_INIT_VIDEO);
	reload(e);
}

void Renderer::reload(Engine& e){
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE             , 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE           , 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE            , 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE           , 8);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER         , 1);
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	
	int ctx_flags = SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG | SDL_GL_CONTEXT_DEBUG_FLAG;
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, ctx_flags);
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	
	window = SDL_CreateWindow(
		window_title,
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		window_width->val,
		window_height->val,
		SDL_WINDOW_OPENGL
	);
	
	if(SDL_GL_LoadLibrary(libgl->str.empty() ? nullptr : libgl->str.c_str()) < 0){
		log(logging::fatal, "Couldn't load OpenGL library! (%s).", SDL_GetError());
	}
	
	gl.createContext(e, window);
	gl.Viewport(0, 0, window_width->val, window_height->val);
}

void Renderer::onWindowEvent(SDL_WindowEvent& ev){
	//TODO
}

void Renderer::drawFrame(){

	gl.Clear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for(auto& r : renderables){
		VertexState* v = r->vertex_state;
		if(!v) continue;
		
		r->blend_mode.set(render_state);
		
		if(ShaderProgram* s = r->shader){
			s->bind(render_state);
			s->setAttribs(render_state, *v);
			
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

void Renderer::delRenderable(Renderable& r){

}

Renderer::~Renderer(){
	gl.deleteContext();
	SDL_GL_UnloadLibrary();
	
	if(window){
		SDL_DestroyWindow(window);
	}
	
	SDL_QuitSubSystem(SDL_INIT_VIDEO);
}


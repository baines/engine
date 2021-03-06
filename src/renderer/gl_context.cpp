#include "gl_context.h"
#include "engine.h"
#include "config.h"
#include "enums.h"
#include "util.h"

#ifndef CALLBACK
	#define CALLBACK
#endif

#define GL_STRINGIFY(name) STRINGIFY(gl##name)
#undef OPTIONAL
GLContext gl;

using namespace logging;

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
			case GL_DEBUG_SEVERITY_HIGH:         return "HIGH";
			case GL_DEBUG_SEVERITY_MEDIUM:       return "MED";
			case GL_DEBUG_SEVERITY_LOW:          return "LOW";
			default:
			case GL_DEBUG_SEVERITY_NOTIFICATION: return "INFO";
		}
	}

	void CALLBACK gl_dbg_callback(GLenum src, GLenum type, GLuint id, GLenum sev, 
	GLsizei len, const char* msg, const void* p) {
		logging::level lvl = 
			(sev == GL_DEBUG_SEVERITY_HIGH)   ? logging::error :
			(sev == GL_DEBUG_SEVERITY_MEDIUM) ? logging::warn  :
			(sev == GL_DEBUG_SEVERITY_LOW)    ? logging::debug :
			logging::trace;

		log(lvl,
			"[GL-%s-%s-%s] [%u] %s",
			gl_dbgsrc2str(src),
			gl_dbgtype2str(type),
			gl_dbgsev2str(sev),
			id,
			msg
		);
	}

	constexpr str_const prefix_names[] = { "ARB_", "ARB_", "EXT_", "AMD_", "NV_" };
	
	enum {
		ARBCORE  = 1 << 16,
		ARB      = 1 << 17,
		EXT      = 1 << 18,
		AMD      = 1 << 19,
		NV       = 1 << 20,
		
		OPTIONAL = 1 << 31
	};
	
	inline bool check_result(const char* name, uint32_t flags, bool loaded){
		if(!(flags & OPTIONAL) && !loaded){
			log(fatal, "Required OpenGL function %s is not available.", name);
			abort();
		}
		return loaded;
	}

	bool do_load(const char* name, void** ptr){
		log(info, "Loading Func: %-32s [%p]", name, (*ptr = SDL_GL_GetProcAddress(name)));
		return *ptr != nullptr;
	}

	bool load_func(const char* name, void** ptr){
		return check_result(name, 0, do_load(name, ptr));
	}
	
	bool load_func(const char* name, void** ptr, uint32_t flags){
		uint32_t req_vers = flags & 0xFFFF;
		bool result = gl.version >= req_vers && do_load(name, ptr);
		return check_result(name, flags, result);
	}

	bool load_func(const char* name, size_t nsz, void** ptr, uint32_t flags, const char* ext, size_t esz){
		uint32_t req_vers = flags & 0xFFFF;
		
		if(req_vers && gl.version >= req_vers){
			return do_load(name, ptr);
		} else {
			for(size_t i = 0; i < SDL_arraysize(prefix_names) && !*ptr; ++i){
				if(!(flags & (1 << (16+i)))) continue;
				
				size_t sz = prefix_names[i].size;
				char* ebuf = SDL_stack_alloc(char, esz + sz);
				memcpy(ebuf, prefix_names[i].str, sz);
				memcpy(ebuf + sz, ext, esz);
			
				if(gl.hasExtension(ebuf)){
					if(i == 0){ // ARBCORE doesn't append to func names
						do_load(name, ptr);
					} else {
						char* nbuf = SDL_stack_alloc(char, nsz + sz - 1);
				
						memcpy(nbuf, name, nsz - 1);
						memcpy(nbuf + nsz - 1, prefix_names[i].str, sz - 1);
						nbuf[nsz+sz-2] = 0;
				
						do_load(nbuf, ptr);
				
						SDL_stack_free(nbuf);
					}
				}
			
				SDL_stack_free(ebuf);
			}

			return check_result(name, flags, *ptr != nullptr);
		}
	}
		
	template<size_t A, size_t B>
	bool load_func(const char (&name)[A], void** ptr, uint32_t v, const char (&ext)[B]){
		return load_func(name, A, ptr, v, ext, B);
	}
}

GLContext::GLContext()
: version(0)
, streaming_mode(nullptr)
#define GLFUNC(type, name, ...) \
	, name(0)
#include "gl_functions.h"
#undef GLFUNC
, sdl_context(nullptr)
{

}

bool GLContext::createContext(Engine& e, SDL_Window* w){
	if(sdl_context) return true;
	
	if(!streaming_mode){
		streaming_mode = e.cfg->addVar<CVarEnum>("gl_streaming_mode", gl_streaming_enum, 0);
	}
	
	int maj = 0, min = 0;
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, &maj);
	SDL_GL_GetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, &min);
	
	SDL_GLContext ctx = nullptr;
	if((ctx = SDL_GL_CreateContext(w))){
		log(logging::info, "Got OpenGL %d.%d context.", maj, min);
		loadAllFuncs();

		for(auto i = objects.begin(); i != objects.end(); /**/){
			if(i->second == DELETED){
				objects.erase(i++);
			} else {
				++i;
			}
		}

		for(auto& pair : objects){
			if(pair.second == INVALID){
				pair.second = VALID;
				pair.first->onGLContextRecreate();
			}
		}

#if defined(DEBUG) && !defined(_WIN32)
// this is bugged on WINE: https://bugs.winehq.org/show_bug.cgi?id=38402
		if(DebugMessageCallback){
			DebugMessageCallback(&gl_dbg_callback, nullptr);
			Enable(GL_DEBUG_OUTPUT);
		}
#endif

		sdl_context = ctx;

		return true;
	} else {
		log(logging::debug, "OpenGL %d.%d context unavailable.", maj, min);
		return false;
	}
}

void GLContext::deleteContext(void){
	if(sdl_context){
		SDL_GL_DeleteContext(sdl_context);
		sdl_context = nullptr;
	}

	// invalidate objects
	for(auto& pair : objects){
		if(pair.second == VALID){
			pair.second = INVALID;
		}
	}

	extensions.clear();

	#define GLFUNC(type, name, ...) \
		name = nullptr;
	#include "gl_functions.h"
	#undef GLFUNC
}

bool GLContext::hasExtension(const char* ext){
	if(ext[0] == 'G' && ext[1] == 'L' && ext[2] == '_') ext += 3;
	bool ok = extensions.find(str_hash(ext)) != extensions.end();
	log(info, "Checking Ext: %-32s [%s]", ext, ok ? "Available." : "Unavailable.");
	return ok;
}

bool GLContext::initialized(){
	return sdl_context != nullptr;
}

void GLContext::registerObject(GLObject& obj){
	objects[&obj] = VALID;
}

void GLContext::validateObject(const GLObject& obj){
	for(auto& pair : objects){
		if(pair.first == &obj && pair.second == INVALID){
			pair.second = VALID;
			pair.first->onGLContextRecreate();
			break;
		}
	}
}

void GLContext::unregisterObject(GLObject& obj){
	auto it = objects.find(&obj);
	if(it != objects.end()){
		it->second = DELETED;
	}
}

bool GLContext::loadAllFuncs(void){
	GetError    = (decltype(GetError))   SDL_GL_GetProcAddress("glGetError");
	GetString   = (decltype(GetString))  SDL_GL_GetProcAddress("glGetString");
	GetIntegerv = (decltype(GetIntegerv))SDL_GL_GetProcAddress("glGetIntegerv");
	
	const char* vs = (const char*)GetString(GL_VERSION);
	int v_maj = 0, v_min = 0;
	
	log(info, "OpenGL version: %s", vs);
	
	sscanf(vs, "%d.%d", &v_maj, &v_min);
	version = v_maj * 10 + v_min;
	
	if(version >= 30){
		GetStringi = (decltype(GetStringi)) SDL_GL_GetProcAddress("glGetStringi");
	}
	
	loadExtensions();

	size_t total = 0, loaded = 0;
	#define GLFUNC(type, name, args, ...) \
		total++; \
		if(load_func(GL_STRINGIFY(name), reinterpret_cast<void**>(&name), ##__VA_ARGS__ )){ \
			loaded++; \
		} else { \
			log(info, "Func %s is not available.", GL_STRINGIFY(name)); \
		}
	#include "gl_functions.h"
	#undef GLFUNC
	log(info, "Loaded %zu/%zu OpenGL functions.", loaded, total);

	return loaded == total;
}

void GLContext::loadExtensions(){
	if(version >= 30){
		GLint num_ext = 0;
		GetIntegerv(GL_NUM_EXTENSIONS, &num_ext);
	
		for(GLint i = 0; i < num_ext; ++i){
			const char* ext = reinterpret_cast<const char*>(GetStringi(GL_EXTENSIONS, i));
			if(ext){
				if(!extensions.insert(str_hash(ext+3)).second){
					//TODO: better collision recovery
					log(logging::warn, "GL Extension hash collision :/ (%s).", ext);
				}
			}
		}
	} else {
		const char* exts = reinterpret_cast<const char*>(GetString(GL_EXTENSIONS));
		if(exts){
			const char* c = nullptr, *prev_c = exts+3;

			while((c = strchr(prev_c, ' '))){
				int len = c - prev_c;
				if(!extensions.insert(str_hash_len(prev_c, len)).second){
					log(logging::warn, "GL Extension hash collision :/ (%.*s).", len, prev_c);
				}
				if(!c[1] || !c[2] || !c[3]) break;
				prev_c = c+4;
			}
		}
	}
}


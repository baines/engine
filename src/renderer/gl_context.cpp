#include "gl_context.h"

#define STRINGIFY(name) #name
#define GL_STRINGIFY(name) STRINGIFY(gl##name)

GLContext gl;

namespace {

	constexpr str_const prefix_names[] = { "ARB_", "ARB_", "EXT_", "AMD_", "NV_" };

	enum {
		ARBCORE = 1 << 16,
		ARB     = 1 << 17,
		EXT     = 1 << 18,
		AMD     = 1 << 19,
		NV      = 1 << 20
	};

	bool loadFunc(const char* name, void*& ptr){
		printf("Loading Func: %s = %p\n", name, (ptr = SDL_GL_GetProcAddress(name)));
		return ptr != nullptr;
	}
	
	bool loadFunc(const char* name, void*& ptr, uint32_t v){
		return gl.version >= v ? loadFunc(name, ptr) : false;
	}

	bool loadFunc(const char* name, size_t nsz, void*& ptr, uint32_t req_vers, const char* ext, size_t esz){
		uint32_t v = req_vers & 0xFFFF;
	
		if(v && gl.version >= v){
			return loadFunc(name, ptr);
		} else {
			for(int i = 0; i < SDL_arraysize(prefix_names) && !ptr; ++i){
				if(!(req_vers & (1 << (16+i)))) continue;
				
				size_t sz = prefix_names[i].size;
				char* ebuf = SDL_stack_alloc(char, esz + sz);
				memcpy(ebuf, prefix_names[i].str, sz);
				memcpy(ebuf + sz, ext, esz);
			
				if(gl.hasExtension(ebuf)){
					if(i == 0){ // ARBCORE doesn't prepend to func names
						loadFunc(name, ptr);
					} else {
						char* nbuf = SDL_stack_alloc(char, nsz + sz - 1);
				
						memcpy(nbuf, name, nsz - 1);
						memcpy(nbuf + nsz - 1, prefix_names[i].str, sz - 1);
						nbuf[nsz+sz-2] = 0;
				
						loadFunc(nbuf, ptr);
				
						SDL_stack_free(nbuf);
					}
				}
			
				SDL_stack_free(ebuf);
			}
					
			return ptr != nullptr;
		}
	}
		
	template<size_t A, size_t B>
	bool loadFunc(const char (&name)[A], void*& ptr, uint32_t v, const char (&ext)[B]){
		return loadFunc(name, A, ptr, v, ext, B);
	}
}

GLContext::GLContext()
: base_w(0)
, base_h(0)
, version(0)
, sdl_context(nullptr)
#define GLFUNC(type, name, args, ...) \
	, name(0)
#include "gl_functions.h"
#undef GLFUNC
{

}

bool GLContext::createContext(SDL_Window* w){
	sdl_context = SDL_GL_CreateContext(w);
	if(!sdl_context) puts(SDL_GetError());
	return sdl_context != nullptr && loadAllFuncs();
}

void GLContext::deleteContext(void){
	if(sdl_context) SDL_GL_DeleteContext(sdl_context);
	#define GLFUNC(type, name, args, ...) \
		name = nullptr;
	#include "gl_functions.h"
	#undef GLFUNC
}

bool GLContext::hasExtension(const char* ext){
	if(ext[0] == 'G' && ext[1] == 'L' && ext[2] == '_') ext += 3;
	bool ok = extensions.find(ext) != extensions.end();
	printf("Checking GL Extension \"%s\" : %s.\n", ext, ok ? "Yes" : "No");
	return ok;
}
bool GLContext::loadAllFuncs(void){
	GetString   = (decltype(GetString))  SDL_GL_GetProcAddress("glGetString");
	GetIntegerv = (decltype(GetIntegerv))SDL_GL_GetProcAddress("glGetIntegerv");
	
	const char* vs = (const char*)GetString(GL_VERSION);
	int v_maj = 0, v_min = 0;
	
	printf("OpenGL version: %s\n", vs);
	
	sscanf(vs, "%d.%d", &v_maj, &v_min);
	version = v_maj * 10 + v_min;
	
	if(version >= 30){
		GetStringi = (decltype(GetStringi)) SDL_GL_GetProcAddress("glGetStringi");
	}
	
	loadExtensions();

	bool ret = true;
	#define GLFUNC(type, name, args, ...) \
		ret = ret && loadFunc(GL_STRINGIFY(name), (void*&)name, ##__VA_ARGS__);
	#include "gl_functions.h"
	#undef GLFUNC
	if(ret){
		puts("All OpenGL functions loaded successfully!");
	}
	return ret;
}

void GLContext::loadExtensions(){
	if(version >= 30){
		GLint num_ext = 0;
		GetIntegerv(GL_NUM_EXTENSIONS, &num_ext);
	
		for(GLint i = 0; i < num_ext; ++i){
			const char* ext = reinterpret_cast<const char*>(GetStringi(GL_EXTENSIONS, i));
			if(ext){
				extensions.emplace(ext+3);
			}
		}
	} else {
		const char* exts = reinterpret_cast<const char*>(GetString(GL_EXTENSIONS));
		if(exts){
			const char* c, *prev_c = exts+3;
			
			while((c = strchrnul(prev_c, ','), *c)){
				extensions.emplace(prev_c, c);
				prev_c = c+3;
			}
		}
	}
}


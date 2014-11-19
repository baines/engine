#ifndef GL_CONTEXT_H
#define GL_CONTEXT_H
#include <GL/gl.h>
#include <GL/glext.h>
#include <SDL2/SDL.h>
#include <unordered_set>
#include "gl_functions.h"
#include "util.h"
#include "cvar.h"

struct GLContext {
	GLContext();
	bool createContext(SDL_Window* w);
	void deleteContext(void);
	bool hasExtension(const char* ext);
	bool initialized();
	
	int base_w, base_h, version;

	GLenum         (APIENTRY* GetError)(void);
	const GLubyte* (APIENTRY* GetString)(GLenum);
	const GLubyte* (APIENTRY* GetStringi)(GLenum, GLuint);
	void           (APIENTRY* GetIntegerv)(GLenum, GLint*);

	#define GLFUNC(type, name, args, ...) \
		name##_p name;
	#include "gl_functions.h"
	#undef GLFUNC
	
private:
	bool loadAllFuncs(void);
	void loadExtensions(void);
	std::unordered_set<uint32_t> extensions;
	SDL_GLContext sdl_context;
};

extern GLContext gl;

#endif

#ifndef GL_CONTEXT_H
#define GL_CONTEXT_H
#include <GL/gl.h>
#include <SDL2/SDL_rect.h>
#include "common.h"
#include "gl_functions.h"
#include "shader_attribs.h"
#include "blend_mode.h"

struct SDL_Window;

struct GLContext {
	GLContext();
	bool createContext(Engine& e, SDL_Window* w);
	void deleteContext(void);
	bool hasExtension(const char* ext);
	bool initialized();

	void registerObject(GLObject& obj);
	void validateObject(const GLObject& obj);
	void unregisterObject(GLObject& obj);

	uint32_t version;
	
	CVarEnum* streaming_mode;

	GLenum         (APIENTRY* GetError)(void);
	const GLubyte* (APIENTRY* GetString)(GLenum);
	const GLubyte* (APIENTRY* GetStringi)(GLenum, GLuint);
	void           (APIENTRY* GetIntegerv)(GLenum, GLint*);

	struct State {
		GLuint program;
		GLuint active_tex_index;
		Array<GLuint, 8> tex; //TODO: lookup GL_MAX_TEXTURE_IMAGE_UNITS
		Array<GLuint, 8> samp;

		GLuint vao;
		GLuint vbo;
		GLuint ibo;

		uint16_t enabled_attrib_arrays; // used when VAOs aren't supported.
		ShaderAttribs active_attribs;   // used when VAOs aren't supported.

		BlendMode blend_mode{{{ GL_ONE, GL_ZERO, GL_ONE, GL_ZERO }}};

		SDL_Rect clip;
	} state;

	#define GLFUNC(type, name, ...) \
		name##_p name;
	#include "gl_functions.h"
	#undef GLFUNC
	
private:
	bool loadAllFuncs(void);
	void loadExtensions(void);
	void* sdl_context;
};

extern GLContext gl;

struct GLObject {
	GLObject(){
		gl.registerObject(*this);
	}
	virtual ~GLObject(){
		gl.unregisterObject(*this);
	}
	virtual void onGLContextRecreate(){}
};

#endif

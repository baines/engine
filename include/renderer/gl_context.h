#ifndef GL_CONTEXT_H
#define GL_CONTEXT_H
#include <GL/gl.h>
#include <GL/glext.h>
#include <map>
#include <unordered_set>
#include "common.h"
#include "gl_functions.h"

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

	#define GLFUNC(type, name, ...) \
		name##_p name;
	#include "gl_functions.h"
	#undef GLFUNC
	
private:
	bool loadAllFuncs(void);
	void loadExtensions(void);
	std::unordered_set<uint32_t> extensions;
	enum GLObjStatus { VALID, INVALID, DELETED };
	std::map<GLObject*, GLObjStatus> objects;
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

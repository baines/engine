#ifndef _GL_CONTEXT_H_
#define _GL_CONTEXT_H_
#define _GL_STRING(name) #name
#define _GL(name) _GL_STRING(gl##name)
#include <SDL/SDL_opengl.h>
#include "opengl.h"
#include "util.h"
#include <SDL/SDL.h>

class GLContext {
	int initial_w, initial_h;
public:
	GLContext() : initial_w(320), initial_h(240)
		#define _GL_FN(type, name, args) \
			, name(0)
		#include "opengl.h"
		#undef _GL_FN
	{}
	void createContext(int w, int h){
		loadAllFuncs();			
		MatrixMode(GL_PROJECTION);
        int scale = util::min(h / initial_h, w / initial_w);
        //float scale = (float)h / (float)initial_h; //non pixel-perfect scale
        //int xOff = (w - (initial_w * scale)) / 2;
        //int yOff = (h - (initial_h * scale)) / 2;
        //Viewport(xOff, yOff, initial_w * scale, initial_h * scale);
        Viewport(0, 0, w, h);
		LoadIdentity();
		Ortho(0, initial_w, initial_h, 0, -1, 1);
		MatrixMode(GL_MODELVIEW);
		Enable(GL_TEXTURE_2D);
		//Enable(GL_BLEND);
		FrontFace(GL_CW);
		Enable(GL_CULL_FACE);
		//BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		Enable(GL_FRAGMENT_PROGRAM_ARB);
		Enable(GL_VERTEX_PROGRAM_ARB);
		EnableVertexAttribArrayARB(0);
		EnableVertexAttribArrayARB(1);
		EnableVertexAttribArrayARB(8);
	}
	bool loadFunction(const char* name, void** ptr){
		*ptr = SDL_GL_GetProcAddress(name);
		if(*ptr){
			return true;
		} else {
			printf("OpenGL function %s not available.\n", name);
			return false;
		}
	}
			
	bool loadAllFuncs(){
		bool retval = true;
		#define _GL_FN(type, name, args) \
			retval = retval && loadFunction(_GL(name), (void**)&name);
		#include "opengl.h"
		#undef _GL_FN
		return retval;
	}

	#define _GL_FN(type, name, args) \
		name##_p name;
	#include "opengl.h"
	#undef _GL_FN
};

extern GLContext gl;

#endif

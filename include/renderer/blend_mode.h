#ifndef BLEND_MODE_H_
#define BLEND_MODE_H_
#include "gl_context.h"

struct RenderState;

struct BlendMode {
	BlendMode()
	: funcs { GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA }
	, equations { GL_FUNC_ADD, GL_FUNC_ADD } {
	
	}
	
	void set(RenderState& state); /*{
		GLenum* fns = state.blend_mode.funcs;
		GLenum* eqs = state.blend_mode.equations;
		
		for(size_t i = 0; i < sizeof(funcs); ++i){
			if(funcs[i] != fns[i]){
				gl.BlendFuncSeparate(fns[0], fns[1], fns[2], fns[3]);
				break;
			}
		}
		
		for(size_t i = 0; i < sizeof(equations); ++i){
			if(equations[i] != eqs[i]){
				gl.BlendEquationSeparate(eq[0], eq[1]);
				break;
			}
		}
		
		state.blend_mode = *this;
	}*/
	
	GLenum funcs[4];
	GLenum equations[2];
};

#endif


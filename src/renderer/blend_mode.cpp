#include "blend_mode.h"
#include "render_state.h"

BlendMode::BlendMode()
: funcs {{ GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA }}
, equations {{ GL_FUNC_ADD, GL_FUNC_ADD }} {

}

BlendMode::BlendMode(const Array<GLenum, 4>& fns, const Array<GLenum, 2>& eqs)
: funcs(fns)
, equations(eqs) {
	if(funcs[2] == 0 && funcs[3] == 0){
		funcs[2] = funcs[0];
		funcs[3] = funcs[1];
	}
	if(equations[1] == 0){
		equations[1] = equations[0];
	}
}
	
void BlendMode::bind(RenderState& state) const {
	if(funcs != state.blend_mode.funcs){
		gl.BlendFuncSeparate(funcs[0], funcs[1], funcs[2], funcs[3]);
	}
	
	if(equations != state.blend_mode.equations){
		gl.BlendEquationSeparate(equations[0], equations[1]);
	}
	
	state.blend_mode = *this;
}


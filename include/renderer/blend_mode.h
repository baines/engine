#ifndef BLEND_MODE_H_
#define BLEND_MODE_H_
#include "gl_context.h"

struct RenderState;

struct BlendMode {
	BlendMode();
	BlendMode(const std::array<GLenum, 4>& fns, 
	          const std::array<GLenum, 2>& eqs = { GL_FUNC_ADD, GL_FUNC_ADD });
	          	
	void bind(RenderState& state);
private:
	std::array<GLenum, 4> funcs;
	std::array<GLenum, 2> equations;
};

#endif


#ifndef BLEND_MODE_H_
#define BLEND_MODE_H_
#include "gl_context.h"

struct RenderState;

struct BlendMode {
	BlendMode();
	void set(RenderState& state);
private:
	std::array<GLenum, 4> funcs;
	std::array<GLenum, 2> equations;
};

#endif


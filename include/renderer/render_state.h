#ifndef RENDER_STATE_H_
#define RENDER_STATE_H_
#include "common.h"
#include "gl_context.h"
#include "blend_mode.h"

struct RenderState {
	GLuint program;
	std::array<GLuint, 8> tex;
	std::array<GLuint, 8> samp;
	GLuint vao;
	
	BlendMode blend_mode = BlendMode({ GL_ONE, GL_ZERO, GL_ONE, GL_ZERO});
	
};

#endif


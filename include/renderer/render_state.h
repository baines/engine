#ifndef RENDER_STATE_H_
#define RENDER_STATE_H_
#include "gl_context.h"
#include "blend_mode.h"

struct RenderState {
	GLuint program;
	GLuint tex[8];
	GLuint samp[8];
	GLuint vao;
	
	BlendMode blend_mode;
	
};

#endif


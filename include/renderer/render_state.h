#ifndef RENDER_STATE_H_
#define RENDER_STATE_H_
#include "common.h"
#include "gl_context.h"
#include "blend_mode.h"
#include "shader_attribs.h"
#include <SDL2/SDL_rect.h>

struct RenderState {
	GLuint program;
	GLuint active_tex;
	Array<GLuint, 8> tex; //TODO: lookup GL_MAX_TEXTURE_IMAGE_UNITS
	Array<GLuint, 8> samp;

	GLuint vao;
	GLuint vbo;
	GLuint ibo;

	uint16_t enabled_attrib_arrays; // used when VAOs aren't supported.
	ShaderAttribs active_attribs;   // used when VAOs aren't supported.

	BlendMode blend_mode{{{ GL_ONE, GL_ZERO, GL_ONE, GL_ZERO }}};

	SDL_Rect clip;
};

#endif


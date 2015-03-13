#ifndef RENDER_STATE_H_
#define RENDER_STATE_H_
#include "common.h"
#include "gl_context.h"
#include "blend_mode.h"
#include "shader_attribs.h"
#include <bitset>

struct RenderState {
	GLuint program;
	GLuint active_tex;
	std::array<GLuint, 8> tex; //TODO: lookup GL_MAX_TEXTURE_IMAGE_UNITS
	std::array<GLuint, 8> samp;

	GLuint vao;
	GLuint vbo;
	GLuint ibo;

	std::bitset<16> enabled_attrib_arrays; // used when VAOs aren't supported.
	ShaderAttribs active_attribs;          // used when VAOs aren't supported.

	BlendMode blend_mode{{{ GL_ONE, GL_ZERO, GL_ONE, GL_ZERO }}};
};

#endif


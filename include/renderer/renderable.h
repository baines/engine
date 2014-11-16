#ifndef RENDERABLE_H_
#define RENDERABLE_H_
#include "texture.h"
#include "sampler.h"
#include "vertex_state.h"
#include "shader.h"
#include "shader_uniforms.h"
#include "blend_mode.h"

struct Renderable {

	Renderable()
	: textures()
	, samplers()
	, vertex_state(nullptr)
	, shader(nullptr)
	, uniforms(nullptr)
	, blend_mode()
	, prim_type(GL_TRIANGLES)
	, count(0)
	, offset(0) {
	
	}
	
	std::array<Texture*, 8> textures;
	std::array<Sampler*, 8> samplers;
	
	VertexState*    vertex_state;
	ShaderProgram*  shader;
	ShaderUniforms* uniforms;
	
	BlendMode blend_mode;
	GLenum    prim_type;
	GLsizei   count;
	GLint     offset;
	
};

#endif


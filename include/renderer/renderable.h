#ifndef RENDERABLE_H_
#define RENDERABLE_H_
#include "common.h"
#include <GL/gl.h>
#include "blend_mode.h"
#include "shader_uniforms.h"

struct RCount { GLsizei value; };
struct RType  { GLenum  value; };
struct ROff   { GLint   value; };

struct Renderable {

	Renderable()
	: textures({})
	, samplers({})
	, vertex_state(nullptr)
	, shader(nullptr)
	, uniforms(nullptr)
	, blend_mode()
	, prim_type(GL_TRIANGLES)
	, count(0)
	, offset(0) {

	}

	bool usesSameState(const Renderable& o);/*{

		bool uniforms_compat = uniforms && o.uniforms
			? *uniforms == *o.uniforms
			: !uniforms && !o.uniforms;

		return prim_type == o.prim_type
		    && blend_mode == o.blend_mode
		    && vertex_state == o.vertex_state
		    && shader == o.shader
		    && textures == o.textures
		    && samplers == o.samplers
			&& uniforms_compat;
	}*/
	
	Array<const Texture*, 8> textures;
	Array<const Sampler*, 8> samplers;
	
	VertexState*    vertex_state;
	ShaderProgram*  shader;
	ShaderUniforms* uniforms;
	
	BlendMode blend_mode;
	GLenum    prim_type;
	GLsizei   count;
	GLint     offset;
	
};

#endif


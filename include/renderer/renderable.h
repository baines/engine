#ifndef RENDERABLE_H_
#define RENDERABLE_H_
#include "texture.h"
#include "sampler.h"
#include "vertex_state.h"
#include "shader.h"
#include "shader_uniforms.h"
#include "blend_mode.h"

struct Renderable {

	Renderable();

	bool isValid(){
		// num tex == num samp, has v state e.t.c.
	}
	
	size_t          getNumTextures();
	Texture*        getTexture(size_t index);
	Sampler*        getSampler(size_t index);
	VertexState*    getVertexState();
	ShaderProgram*  getShader();
	ShaderUniforms* getUniforms();
	
	void setTexture(Texture* t, size_t index);
	void setSampler(Sampler* s, size_t index);
	void setVertexState(VertexState* state);
	void setShader(ShaderProgram* shader);
	void setUniforms(ShaderUniforms* uniforms);
	
	void bind(Renderable* current);
	
	BlendMode blend_mode;
	
	GLenum prim_type;
	GLsizei count;
	GLint offset;
	
};

#endif


#ifndef SHADER_H_
#define SHADER_H_
#include "gl_context.h"
#include "resource_system.h"
#include "shader_uniforms.h"
#include "shader_attribs.h"
#include "render_state.h"
#include <vector>

struct VertexState;

struct ShaderBase {
	ShaderBase(GLenum type);
	bool load(const ResourceHandle& data);
	GLuint getID() const;
	virtual ~ShaderBase();
private:
	GLenum type;
	GLuint id;
};

struct VertShader : ShaderBase {
	VertShader() : ShaderBase(GL_VERTEX_SHADER){}
};

struct FragShader : ShaderBase {
	FragShader() : ShaderBase(GL_FRAGMENT_SHADER){}
};

struct ShaderProgram {
	ShaderProgram(const VertShader& v, const FragShader& f);
	bool link(void);
	bool bind(RenderState& rs);
	void setUniforms(const ShaderUniforms& uniforms);
	void setAttribs(RenderState& rs, VertexState& vstate);

	~ShaderProgram();
private:
	const VertShader& vs;
	const FragShader& fs;
	GLuint program_id;

	ShaderUniforms uniforms;
	ShaderAttribs  attribs;
};

#endif

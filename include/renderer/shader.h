#ifndef SHADER_H_
#define SHADER_H_
#include "gl_context.h"
#include "resource_system.h"
#include "shader_uniforms.h"
#include "shader_attribs.h"
#include "render_state.h"
#include "proxy.h"
#include <vector>

struct VertexState;

struct ShaderBase : public GLObject {
	ShaderBase(GLenum type);
	bool loadFromResource(Engine& e, const ResourceHandle& data);
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

struct ShaderProgram : public GLObject {
	ShaderProgram(Proxy<VertShader> v, Proxy<FragShader> f);
	bool link(void);
	bool bind(RenderState& rs);
	void setUniforms(const ShaderUniforms& uniforms);
	void setAttribs(RenderState& rs, VertexState& vstate);
	virtual void onGLContextRecreate();

	~ShaderProgram();
private:
	Proxy<VertShader> vs;
	Proxy<FragShader> fs;
	GLuint program_id;

	ShaderUniforms uniforms;
	ShaderAttribs  attribs;
};

#endif

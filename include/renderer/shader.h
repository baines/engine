#ifndef SHADER_H_
#define SHADER_H_
#include "gl_context.h"
#include "shader_uniforms.h"
#include "shader_attribs.h"
#include "proxy.h"
#include <vector>

struct VertexState;

struct ShaderBase : public GLObject {
	ShaderBase(GLenum type, MemBlock mem);
	GLuint getID() const;
	virtual ~ShaderBase();
protected:
	GLenum type;
	GLuint id;
};

struct VertShader : ShaderBase {
	VertShader(MemBlock mem) : ShaderBase(GL_VERTEX_SHADER, mem){}
};

struct FragShader : ShaderBase {
	FragShader(MemBlock mem) : ShaderBase(GL_FRAGMENT_SHADER, mem){}
};

struct ShaderProgram : public GLObject {
	ShaderProgram(Proxy<VertShader> v, Proxy<FragShader> f);
	bool link(void);
	bool bind();
	void setUniforms(const ShaderUniforms& uniforms);
	void setAttribs(VertexState& vstate);
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

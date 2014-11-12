#ifndef VERTEX_BUFFER_H_
#define VERTEX_BUFFER_H_
#include "gl_context.h"
#include "resource_system.h"
#include "shader_attribs.h"

struct VertexBuffer {
	virtual const ShaderAttribs& getShaderAttribs(void) const = 0;
	virtual GLuint getID(void) const = 0;
	virtual GLint getStride(void) const = 0;
};

struct StaticVertexBuffer : VertexBuffer {
	StaticVertexBuffer();
	StaticVertexBuffer(ResourceHandle& data, const char* fmt);
	virtual const ShaderAttribs& getShaderAttribs(void) const;
	virtual void invalidate(void);
	virtual GLuint getID(void) const;
	virtual GLint getStride(void) const;
private:
	void parseAttribs(const char* fmt);
	ShaderAttribs attrs;
	ResourceHandle data;
	GLint stride;
	GLuint id;
};

struct DynamicVertexBuffer : VertexBuffer {

	const ShaderAttribs& getShaderAttribs(void) const;
	GLuint getID(void) const;
	GLint getStride(void) const;
};

#endif

#ifndef VERTEX_STATE_H_
#define VERTEX_STATE_H_
#include "common.h"
#include "gl_context.h"
#include "shader_attribs.h"

struct VertexState : public GLObject {
	VertexState();
	VertexState(std::initializer_list<VertexBuffer*> vbs, IndexBuffer* ib);

	void setVertexBuffers(std::initializer_list<VertexBuffer*> buffers);
	void setIndexBuffer(IndexBuffer* buff);
	IndexBuffer* getIndexBuffer(void);
	void setAttribArrays(RenderState& rs, const ShaderAttribs& attrs);
	void bind(RenderState& rs);
	void onGLContextRecreate();
	~VertexState();
private:
	uint16_t enabled_arrays; //TODO: use vector<bool> + lookup GL_MAX_VERTEX_ATTRIBS
	ShaderAttribs active_attribs;
	std::vector<VertexBuffer*> vertex_buffers;
	IndexBuffer* index_buffer;
	GLuint id;
	bool using_vao;
};

#endif

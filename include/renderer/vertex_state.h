#ifndef VERTEX_STATE_H_
#define VERTEX_STATE_H_
#include "gl_context.h"
#include "vertex_buffer.h"
#include "index_buffer.h"
#include "render_state.h"
#include <bitset>

struct VertexState : public GLObject {
	VertexState();
	void setVertexBuffers(std::initializer_list<VertexBuffer*> buffers);
	void setIndexBuffer(IndexBuffer* buff);
	IndexBuffer* getIndexBuffer(void);
	void setAttribArrays(RenderState& rs, const ShaderAttribs& attrs);
	void bind(RenderState& rs);
	void onGLContextRecreate();
	~VertexState();
private:
	std::bitset<16> enabled_arrays; //TODO: use vector<bool> + lookup GL_MAX_VERTEX_ATTRIBS
	ShaderAttribs active_attrs;
	std::vector<VertexBuffer*> vertex_buffers;
	IndexBuffer* index_buffer;
	GLuint id;
};

#endif

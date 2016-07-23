#ifndef INDEX_BUFFER_H_
#define INDEX_BUFFER_H_
#include "common.h"
#include "gl_context.h"
#include "buffer_common.h"
#include "resource_system.h"

struct IndexBuffer : public GLObject {
	virtual void   bind(RenderState&)  = 0;
	virtual GLenum getType() const     = 0;
	virtual GLuint getID()   const     = 0;
	virtual void update(RenderState&)  = 0;
	virtual void onGLContextRecreate() = 0;
	virtual ~IndexBuffer(){}
};

struct StaticIndexBuffer : public IndexBuffer {
	StaticIndexBuffer();
	StaticIndexBuffer(const ResourceHandle& data, GLenum type);
	void bind(RenderState&);
	GLenum getType() const;
	GLuint getID() const;
	void update(RenderState&){};
	void onGLContextRecreate();
	~StaticIndexBuffer();
private:
	ResourceHandle data;
	GLenum type;
	GLuint id;
};

//extern template class std::vector<uint8_t>;

template<class T>
struct DynamicIndexBuffer : public IndexBuffer {
	DynamicIndexBuffer();
	void replace(size_t index, T val);
	void push(T val);
	void clear();
	GLenum getType() const ;
	GLuint getID() const ;
	size_t getSize() const ;
	void bind(RenderState& rs);
	void update(RenderState& rs);
	virtual void onGLContextRecreate();
private:
	std::vector<uint8_t> indices;
	StreamingBuffer stream_buf;
};

#endif

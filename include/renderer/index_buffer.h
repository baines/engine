#ifndef INDEX_BUFFER_H_
#define INDEX_BUFFER_H_
#include "common.h"
#include "gl_context.h"
#include "buffer_common.h"
#include "resource_system.h"

struct IndexBuffer : public GLObject {
	virtual void   bind()              = 0;
	virtual GLenum getType() const     = 0;
	virtual GLuint getID()   const     = 0;
	virtual void update()              = 0;
	virtual void onGLContextRecreate() = 0;
	virtual ~IndexBuffer(){}
};

struct StaticIndexBuffer : public IndexBuffer {
	StaticIndexBuffer();
	StaticIndexBuffer(const MemBlock& data, GLenum type);
	void bind();
	GLenum getType() const;
	GLuint getID() const;
	void update(){};
	void onGLContextRecreate();
	~StaticIndexBuffer();
private:
	MemBlock data;
	GLenum type;
	GLuint id;
};

template<class T>
struct DynamicIndexBuffer : public IndexBuffer {
	DynamicIndexBuffer();
	void replace(size_t index, T val);
	void push(T val);
	void clear();

	uint8_t* beginWrite (size_t upper_bound);
	void     endWrite   (size_t bytes_written);

	GLenum getType() const ;
	GLuint getID() const ;
	size_t getSize() const ;
	void bind();
	void update();
	virtual void onGLContextRecreate();
private:
	std::vector<uint8_t> indices;
	StreamingBuffer stream_buf;
};

#endif

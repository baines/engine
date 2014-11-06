#ifndef INDEX_BUFFER_H_
#define INDEX_BUFFER_H_
#include "gl_context.h"
#include "resource_system.h"

struct IndexBuffer {
	virtual void   bind()          = 0;
	virtual GLenum getType() const = 0;
	virtual GLuint getID()   const = 0;
	virtual ~IndexBuffer(){}
};

struct StaticIndexBuffer : public IndexBuffer {
	StaticIndexBuffer();
	StaticIndexBuffer(const ResourceHandle& data, GLenum type);
	void bind();
	GLenum getType() const;
	GLuint getID() const;
	~StaticIndexBuffer();
private:
	ResourceHandle data;
	GLenum type;
	GLuint id;
};

#endif

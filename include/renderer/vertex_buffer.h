#ifndef VERTEX_BUFFER_H_
#define VERTEX_BUFFER_H_
#include "gl_context.h"
#include "buffer_common.h"
#include "resource_system.h"
#include "shader_attribs.h"

struct VertexBuffer {
	virtual const ShaderAttribs& getShaderAttribs() const = 0;
	virtual GLint  getStride() const = 0;
	virtual size_t getSize() const = 0;
	virtual GLuint getID() const = 0;
	virtual void update() = 0;

	virtual ~VertexBuffer(){};
};

struct StaticVertexBuffer : VertexBuffer {
	StaticVertexBuffer();
	StaticVertexBuffer(const ResourceHandle& data, const char* fmt);
	virtual const ShaderAttribs& getShaderAttribs() const;
	virtual GLint  getStride() const;
	virtual size_t getSize() const;
	virtual GLuint getID() const;
	virtual void update();
	~StaticVertexBuffer();
private:
	void parseAttribs(const char* fmt);
	ResourceHandle data;
	ShaderAttribs attrs;
	GLint stride;
	GLuint id;
};

struct DynamicVertexBuffer : VertexBuffer {
	DynamicVertexBuffer();
	DynamicVertexBuffer(const char* fmt, size_t initial_capacity);
	
	template<class T>
	void push(const T& vertex_data){
		assert(sizeof(T) == stride);
		
		const char* p = reinterpret_cast<const char*>(&vertex_data);
		
		for(size_t i = 0; i < sizeof(T); ++i){
			data.push_back(p[i]);
		}
		
		stream_buf.mark();	
	}
	
	void invalidate(BufferRange&& range);
	
	void clear();

	virtual const ShaderAttribs& getShaderAttribs() const;
	virtual GLint getStride() const;
	virtual size_t getSize() const;
	virtual GLuint getID() const;
	virtual void update();
private:
	std::vector<uint8_t> data;
	ShaderAttribs attrs;
	GLint stride;
	StreamingBuffer stream_buf;
};

#endif

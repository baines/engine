#ifndef VERTEX_BUFFER_H_
#define VERTEX_BUFFER_H_
#include "gl_context.h"
#include "resource_system.h"
#include "shader_attribs.h"

struct VertexBuffer {
	virtual const ShaderAttribs& getShaderAttribs() const = 0;
	virtual GLint getStride() const = 0;
	virtual GLuint getID() const = 0;
	virtual void update() = 0;

	virtual ~VertexBuffer(){};
};

struct StaticVertexBuffer : VertexBuffer {
	StaticVertexBuffer();
	StaticVertexBuffer(const ResourceHandle& data, const char* fmt);
	virtual const ShaderAttribs& getShaderAttribs() const;
	virtual GLint getStride() const;
	virtual GLuint getID() const;
	virtual void update();
	~StaticVertexBuffer();
private:
	void parseAttribs(const char* fmt);
	ShaderAttribs attrs;
	ResourceHandle data;
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
		
		dirty = true;
	}
	
	void clear();

	virtual const ShaderAttribs& getShaderAttribs() const;
	virtual GLuint getID() const;
	virtual GLint getStride() const;
	virtual void update();
	~DynamicVertexBuffer();
private:
	std::vector<uint8_t> data;
	size_t prev_capacity, prev_size;
	ShaderAttribs attrs;
	bool dirty;
	GLint stride;
	GLuint id;
};

#endif

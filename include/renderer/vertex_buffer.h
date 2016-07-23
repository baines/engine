#ifndef VERTEX_BUFFER_H_
#define VERTEX_BUFFER_H_
#include "common.h"
#include "gl_context.h"
#include "buffer_common.h"
#include "shader_attribs.h"
#include "util.h"

struct VertexBuffer : public GLObject {
	virtual void bind(RenderState& rs);
	virtual const ShaderAttribs& getShaderAttribs() const = 0;
	virtual GLint  getStride() const = 0;
	virtual size_t getSize() const = 0;
	virtual GLuint getID() const = 0;
	virtual void update(RenderState&) = 0;
	virtual void onGLContextRecreate(){};
	virtual ~VertexBuffer(){};
};

struct StaticVertexBuffer : VertexBuffer {
	StaticVertexBuffer();
	StaticVertexBuffer(const MemBlock& data, const char* fmt);
	virtual const ShaderAttribs& getShaderAttribs() const;
	virtual GLint  getStride() const;
	virtual size_t getSize() const;
	virtual GLuint getID() const;
	virtual void update(RenderState&);
	virtual void onGLContextRecreate();
	~StaticVertexBuffer();
private:
	void parseAttribs(const char* fmt);
	MemBlock data;
	ShaderAttribs attrs;
	GLint stride;
	GLuint id;
};

//extern template class std::vector<uint8_t>;

struct DynamicVertexBuffer : VertexBuffer {
	DynamicVertexBuffer();
	DynamicVertexBuffer(const char* fmt, size_t initial_capacity);
	
	template<class T>
	void push(const T& vertex_data){
		const char* p = reinterpret_cast<const char*>(&vertex_data);
		
		for(size_t i = 0; i < sizeof(T); ++i){
			data.push_back(p[i]);
		}
		
		stream_buf.mark();	
	}

	void invalidate(BufferRange&& range);
	
	void clear();
	
	virtual void onGLContextRecreate();
	virtual const ShaderAttribs& getShaderAttribs() const;
	virtual GLint getStride() const;
	virtual size_t getSize() const;
	virtual GLuint getID() const;
	virtual void update(RenderState&);

	~DynamicVertexBuffer(){};
private:
	std::vector<uint8_t> data;
	ShaderAttribs attrs;
	GLint stride;
	StreamingBuffer stream_buf;
};

#endif

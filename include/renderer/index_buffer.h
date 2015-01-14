#ifndef INDEX_BUFFER_H_
#define INDEX_BUFFER_H_
#include "gl_context.h"
#include "buffer_common.h"
#include "resource_system.h"

struct IndexBuffer : public GLObject {
	virtual void   bind()          = 0;
	virtual GLenum getType() const = 0;
	virtual GLuint getID()   const = 0;
	virtual void onGLContextRecreate(){}
	virtual ~IndexBuffer(){}
};

struct StaticIndexBuffer : public IndexBuffer {
	StaticIndexBuffer();
	StaticIndexBuffer(const ResourceHandle& data, GLenum type);
	void bind();
	GLenum getType() const;
	GLuint getID() const;
	void onGLContextRecreate();
	~StaticIndexBuffer();
private:
	ResourceHandle data;
	GLenum type;
	GLuint id;
};

template<class T> struct index_type {};
template<> struct index_type<uint8_t>{ static const GLenum value = GL_UNSIGNED_BYTE; };
template<> struct index_type<uint16_t>{ static const GLenum value = GL_UNSIGNED_SHORT; };
template<> struct index_type<uint32_t>{ static const GLenum value = GL_UNSIGNED_INT; };

template<class T>
struct DynamicIndexBuffer : public IndexBuffer {
	DynamicIndexBuffer()
	: indices()
	, stream_buf(GL_ELEMENT_ARRAY_BUFFER, indices){

	}

	void replace(size_t index, T val){
		if((index + 1) * sizeof(T) > indices.size()) return;

		stream_buf.invalidateAll();
		auto* p = reinterpret_cast<const uint8_t*>(&val);
		for(size_t i = 0; i < sizeof(T); ++i){
			indices[index * sizeof(T) + i] = p[i];
		}
	}
	
	void push(T val){
		auto* p = reinterpret_cast<const uint8_t*>(&val);
		for(size_t i = 0; i < sizeof(T); ++i){
			indices.push_back(p[i]);
		}
		stream_buf.mark();
	}

	void clear(){
		indices.clear();
		stream_buf.mark();
	}

	GLenum getType() const {
		return index_type<T>::value;
	}

	GLuint getID() const {
		return stream_buf.getID();
	}

	size_t getSize() const {
		return indices.size() / sizeof(T);
	}

	void bind(){
		gl.BindBuffer(GL_ELEMENT_ARRAY_BUFFER, stream_buf.getID());
	}	

	virtual void onGLContextRecreate(){
		gl.validateObject(stream_buf);
	}

private:
	std::vector<uint8_t> indices;
	StreamingBuffer stream_buf;
	static_assert(
		std::is_same<T, uint8_t>::value ||
		std::is_same<T, uint16_t>::value ||
		std::is_same<T, uint32_t>::value,
		"Invalid index type"
	);
};

#endif

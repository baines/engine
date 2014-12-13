#ifndef INDEX_BUFFER_H_
#define INDEX_BUFFER_H_
#include "gl_context.h"
#include "buffer_common.h"
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

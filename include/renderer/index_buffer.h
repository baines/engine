#ifndef INDEX_BUFFER_H_
#define INDEX_BUFFER_H_
#include "common.h"
#include "gl_context.h"
#include "buffer_common.h"
#include "resource_system.h"
#include "render_state.h"

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

template<class T> struct index_type {};
template<> struct index_type<uint8_t>{ static const GLenum value = GL_UNSIGNED_BYTE; };
template<> struct index_type<uint16_t>{ static const GLenum value = GL_UNSIGNED_SHORT; };
template<> struct index_type<uint32_t>{ static const GLenum value = GL_UNSIGNED_INT; };

template<class T>
struct DynamicIndexBuffer : public IndexBuffer {
	DynamicIndexBuffer()
	: indices()
	, stream_buf(GL_ELEMENT_ARRAY_BUFFER, indices, false){

	}

	void replace(size_t index, T val){
		if((index + 1) * sizeof(T) > indices.size()) return;

		auto* p = reinterpret_cast<const uint8_t*>(&val);
		for(size_t i = 0; i < sizeof(T); ++i){
			indices[index * sizeof(T) + i] = p[i];
		}
		stream_buf.invalidateAll();
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

	void bind(RenderState& rs){
		auto id = stream_buf.getID();
		if(rs.ibo != id){
			gl.BindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
			rs.ibo = id;
		}
	}

	void update(RenderState& rs){
		stream_buf.update(rs);
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

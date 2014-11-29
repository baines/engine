#include "vertex_buffer.h"
#include <numeric>
#include <algorithm>
#include "enums.h"

namespace {

static GLenum lookup_type(const char c){
	switch(c){
		case 'b': return GL_BYTE;
		case 'B': return GL_UNSIGNED_BYTE;
		case 's': return GL_SHORT;
		case 'S': return GL_UNSIGNED_SHORT;
		case 'i': return GL_INT;
		case 'I': return GL_UNSIGNED_INT;
		case 'f': return GL_FLOAT;
	/*	case 'F': return GL_FIXED; */
		case 'r': return GL_INT_2_10_10_10_REV;
		case 'R': return GL_UNSIGNED_INT_2_10_10_10_REV;
		case 'h': return GL_HALF_FLOAT;
		case 'd': return GL_DOUBLE;
		default:  return 0;
	}
}

static int lookup_size(GLenum e){
	switch(e){
		case GL_BYTE:
		case GL_UNSIGNED_BYTE: 
			return 1;
		case GL_SHORT:
		case GL_UNSIGNED_SHORT:
		case GL_HALF_FLOAT:
			return 2;
		case GL_INT:
		case GL_UNSIGNED_INT:
		case GL_FLOAT:
		case GL_INT_2_10_10_10_REV:
		case GL_UNSIGNED_INT_2_10_10_10_REV:
			return 4;
		case GL_DOUBLE:
			return 8;
		default:
			return 0;
	}
}

// format should be (<name>':'<nelem><type>'|')+ e.g. "a_pos:4S|a_tex:2f"

static void parse_attribs(const char* fmt, ShaderAttribs& attrs, GLint& stride) {

	enum { GET_NAME, GET_NELEM, GET_TYPE, GET_EXTRA } state = GET_NAME;
	
	ShaderAttribs::Attrib current_attr{ };
	stride = 0;
	
	DEBUGF("Starting to parse VBO format...");
	
	auto add_attr = [&](ShaderAttribs::Attrib& at){
		if(at.nelem != 0 && at.type != 0){
			int aligned_sz = (
				at.nelem
				* lookup_size(at.type) 
				+ 3
			) & ~3;
	
			at.off = stride;
			stride += aligned_sz;
		
			attrs.setAttribFormat(at.name_hash, at.type, at.nelem, at.off, at.flags);
			DEBUGF("---- Attrib added.");
		}
	};
	
	const char* name_start = fmt;
	
	for(const char* p = fmt; *p; ++p){
		if(*p == '|'){
			add_attr(current_attr);
			current_attr = ShaderAttribs::Attrib();
			name_start = p + 1;
			state = GET_NAME;
		} else {
			if(state == GET_NAME && *p == ':'){
				const int len = p - name_start;
				DEBUGF("\tAttrib name: %.*s", len, name_start);
				current_attr.name_hash = djb2(name_start, len);
				state = GET_NELEM;
			} else
			if(state == GET_NELEM){
				const int n = *p - 48;
				if(n > 0 && n <= 4){
					current_attr.nelem = n;
					DEBUGF("\tAttrib no. elements: %d", n);
					state = GET_TYPE;
				}
			} else
			if(state == GET_TYPE){
				if(GLenum t = lookup_type(*p)){
					current_attr.type = t;
					DEBUGF("\tAttrib type: %#x", t);
					state = GET_EXTRA;
				}
			} else
			if(state == GET_EXTRA){
				if(*p == 'N') current_attr.flags |= ATR_NORM;
				if(*p == 'I') current_attr.flags |= ATR_INT;
				if(*p == '/') current_attr.flags |= ATR_INSTANCED;
				DEBUGF("\tAttrib flags: %#x", current_attr.flags);
			}
		}
	}
	add_attr(current_attr);
}

}

StaticVertexBuffer::StaticVertexBuffer()
: data()
, attrs()
, stride(0)
, id(0) {

}

StaticVertexBuffer::StaticVertexBuffer(const ResourceHandle& data, const char* fmt)
: data(data)
, attrs()
, stride(0)
, id(0) {
	parse_attribs(fmt, attrs, stride);
	
	gl.GenBuffers(1, &id);
	gl.BindBuffer(GL_ARRAY_BUFFER, id);
	gl.BufferData(GL_ARRAY_BUFFER, data.size(), data.data(), GL_STATIC_DRAW);
}

const ShaderAttribs& StaticVertexBuffer::getShaderAttribs(void) const {
	return attrs;
}

GLuint StaticVertexBuffer::getID(void) const {
	return id;
}

GLint StaticVertexBuffer::getStride(void) const {
	return stride;
}

void StaticVertexBuffer::update(){

}

StaticVertexBuffer::~StaticVertexBuffer(){
	if(id && gl.initialized()){
		gl.DeleteBuffers(1, &id);
	}
}

DynamicVertexBuffer::DynamicVertexBuffer()
: data()
, prev_capacity(0)
, prev_size(0)
, attrs()
, dirty(false)
, stride(0)
, id(0) {

}

DynamicVertexBuffer::DynamicVertexBuffer(const char* fmt, size_t initial_capacity)
: data()
, prev_capacity(0)
, prev_size(0)
, attrs()
, dirty(false)
, stride(0)
, id(0) {

	data.reserve(initial_capacity);
	prev_capacity = data.capacity();

	parse_attribs(fmt, attrs, stride);
	
	gl.GenBuffers(1, &id);
	gl.BindBuffer(GL_ARRAY_BUFFER, id);
	gl.BufferData(GL_ARRAY_BUFFER, data.capacity(), nullptr, GL_STREAM_DRAW);
}

void DynamicVertexBuffer::clear(){
	data.clear();
	prev_size = 0;
	dirty = true;
}

const ShaderAttribs& DynamicVertexBuffer::getShaderAttribs() const {
	return attrs;
}

GLuint DynamicVertexBuffer::getID() const {
	return id;
}

GLint DynamicVertexBuffer::getStride() const {
	return stride;
}

void DynamicVertexBuffer::update(){
/* TODO: more efficient buffer streaming
	https://www.opengl.org/wiki/Buffer_Object_Streaming
	http://www.seas.upenn.edu/~pcozzi/OpenGLInsights/OpenGLInsights-AsynchronousBufferTransfers.pdf
*/
	if(!dirty) return;

	gl.BindBuffer(GL_ARRAY_BUFFER, id);
	
	if(gl.streaming_mode->get() == BUFFER_INVALIDATE){
	
		if(gl.InvalidateBufferData){
			gl.InvalidateBufferData(id);
		} else {
			log(logging::warn, "Streaming mode is BUFFER_INVALIDATE, but glInvalidateBufferData unsupported!");
		}
		
		if(prev_capacity != data.capacity()){
			gl.BufferData(GL_ARRAY_BUFFER, data.capacity(), data.data(), GL_STREAM_DRAW);
		} else {
			gl.BufferSubData(GL_ARRAY_BUFFER, 0, data.size(), data.data());
		}
	}
	
	if(gl.streaming_mode->get() == BUFFER_DATA_NULL){
		gl.BufferData(GL_ARRAY_BUFFER, data.capacity(), nullptr, GL_STREAM_DRAW);
		gl.BufferSubData(GL_ARRAY_BUFFER, 0, data.size(), data.data());
	}
	
	if(gl.streaming_mode->get() == MAP_INVALIDATE){
		GLbitfield flags = GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT;
		
		if(prev_capacity != data.capacity()){
			gl.BufferData(GL_ARRAY_BUFFER, data.capacity(), nullptr, GL_STREAM_DRAW);
			flags &= ~GL_MAP_INVALIDATE_BUFFER_BIT;
		}
		
		void* gl_data = gl.MapBufferRange(
			GL_ARRAY_BUFFER, 
			0, 
			data.size(),
			flags
		);
		
		memcpy(gl_data, data.data(), data.size());
		
		gl.UnmapBuffer(GL_ARRAY_BUFFER);
	}
	
	if(gl.streaming_mode->get() == MAP_UNSYNC_APPEND){
		if(prev_capacity != data.capacity()){
			gl.BufferData(GL_ARRAY_BUFFER, data.capacity(), nullptr, GL_STREAM_DRAW);
		}
		
		void* gl_data = gl.MapBufferRange(
			GL_ARRAY_BUFFER,
			prev_size,
			data.size(),
			GL_MAP_WRITE_BIT | GL_MAP_UNSYNCHRONIZED_BIT | GL_MAP_INVALIDATE_RANGE_BIT
		);
		
		memcpy(gl_data, data.data() + prev_size, data.size() - prev_size);
		
		gl.UnmapBuffer(GL_ARRAY_BUFFER);
	}
	
	if(gl.streaming_mode->get() == DOUBLE_BUFFER){
		log(logging::error, "gl_streaming_mode DOUBLE_BUFFER NYI");
	}

	dirty = false;
	prev_capacity = data.capacity();
	prev_size = data.size();
}

DynamicVertexBuffer::~DynamicVertexBuffer(){

}


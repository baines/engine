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
				current_attr.name_hash = str_hash_len(name_start, len);
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

GLint StaticVertexBuffer::getStride(void) const {
	return stride;
}

size_t StaticVertexBuffer::getSize(void) const {
	return data.size();
}

GLuint StaticVertexBuffer::getID(void) const {
	return id;
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
, attrs()
, stride(0)
, stream_buf() {

}

DynamicVertexBuffer::DynamicVertexBuffer(const char* fmt, size_t initial_capacity)
: data()
, attrs()
, stride(0)
, stream_buf(GL_ARRAY_BUFFER, data) {
	parse_attribs(fmt, attrs, stride);
	data.reserve(initial_capacity);	
}

void DynamicVertexBuffer::clear(){
	data.clear();
	stream_buf.mark();
}

void DynamicVertexBuffer::invalidate(BufferRange&& range){
	stream_buf.invalidate(std::move(range));
}

const ShaderAttribs& DynamicVertexBuffer::getShaderAttribs() const {
	return attrs;
}

GLint DynamicVertexBuffer::getStride() const {
	return stride;
}

size_t DynamicVertexBuffer::getSize() const {
	return data.size();
}

GLuint DynamicVertexBuffer::getID() const {
	return stream_buf.getID();
}

void DynamicVertexBuffer::update(){
	stream_buf.update();
}


#include "vertex_buffer.h"
#include <numeric>
#include <algorithm>

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

}

StaticVertexBuffer::StaticVertexBuffer()
: data()
, attrs()
, stride(0)
, id(0) {

}

StaticVertexBuffer::StaticVertexBuffer(ResourceHandle& data, const char* fmt)
: data(data)
, attrs()
, stride(0)
, id(0) {
	parseAttribs(fmt);
	
	gl.GenBuffers(1, &id);
	gl.BindBuffer(GL_ARRAY_BUFFER, id);
	gl.BufferData(GL_ARRAY_BUFFER, data.size(), data.data(), GL_STATIC_DRAW);
}

// format should be (<name>':'<nelem><type>'|')+ e.g. "a_pos:4S|a_tex:2f"

void StaticVertexBuffer::parseAttribs(const char* fmt) {

	enum { GET_NAME, GET_NELEM, GET_TYPE, GET_EXTRA } state;
	
	ShaderAttribs::Attrib current_attr{};
	stride = 0;
	
	auto add_attr = [&](ShaderAttribs::Attrib& at){
		if(at.nelem != 0 && at.type != 0){
			int aligned_sz = (
				at.nelem
				* lookup_size(at.type) 
				+ 3
			) & ~3;
	
			at.off = stride + aligned_sz;
			stride += at.off;
		
			attrs.setAttrib(at);
		}
	};
	
	const char* name_start = fmt;
	
	for(const char* p = fmt; *p; ++p){
		if(*p == '|'){
			add_attr(current_attr);
			current_attr = ShaderAttribs::Attrib();
			name_start = p + 1;
			state = GET_NELEM;
		} else {
			if(state == GET_NAME){
				if(*p == ':'){
					current_attr.name_hash = djb2(name_start, p - name_start);
					state = GET_NELEM;
				}
			}
			if(state == GET_NELEM){
				int n = *p - 48;
				if(n > 0 && n <= 4){
					current_attr.nelem = n;
					state = GET_TYPE;
				}
			}
			if(state == GET_TYPE){
				if(GLenum t = lookup_type(*p)){
					current_attr.type = t;
					state = GET_EXTRA;
				}
			}
			if(state == GET_EXTRA){
				if(*p == 'N') current_attr.normalised = true;
				if(*p == 'I') current_attr.integral = true;
				if(*p == '/') current_attr.divisor = 1;
			}
		}
	}
	add_attr(current_attr);
}

bool StaticVertexBuffer::containsAttrib(uint32_t attr_hash) const {
	return attrs.getAttrib(attr_hash) != nullptr;
}

bool StaticVertexBuffer::applyAttribFormat(uint32_t attr_hash, GLuint binding_point) const {
	if(const auto* a = attrs.getAttrib(attr_hash)){
		if(a->integral){
			gl.VertexAttribIFormat(binding_point, a->nelem, a->type, a->off);
		} else {
			gl.VertexAttribFormat(binding_point, a->nelem, a->type, a->normalised, a->off);
		}
		return true;
	}
	return false;
}

void StaticVertexBuffer::invalidate(void){
	gl.InvalidateBufferData(id);
}

GLuint StaticVertexBuffer::getID(void) const {
	return id;
}

GLint StaticVertexBuffer::getStride(void) const {
	return stride;
}



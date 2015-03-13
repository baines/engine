#include "shader_attribs.h"
#include <algorithm>

ShaderAttribs::ShaderAttribs()
: attribs() {

}

void ShaderAttribs::initAttrib(uint32_t name_hash, GLint index){
	attribs.push_back({ name_hash, index });
}

void ShaderAttribs::setAttribFormat(uint32_t hash, GLenum type, int nelem, int off, uint32_t flags){
	auto it = std::find(attribs.begin(), attribs.end(), hash);
	
	if(it == attribs.end()){
		attribs.push_back({ hash, -1, type, nelem, off, flags });
	} else {	
		it->type = type;
		it->nelem = nelem;
		it->off = off;
		it->flags = flags;
	}
}

bool ShaderAttribs::containsAttrib(uint32_t hash, GLint index) const {
	auto it = std::find(attribs.begin(), attribs.end(), hash);
	if(it == attribs.end() || (index != -1 && it->index != index)){
		return false;
	} else {
		return true;
	}
}

bool ShaderAttribs::bind(uint32_t hash, GLuint index, GLuint stride) const {
	auto it = std::find(attribs.begin(), attribs.end(), hash);
	
	if(it == attribs.end()) return false;

	const void* off = reinterpret_cast<void*>(it->off);

	if(it->flags & ATR_INT){
		if(gl.VertexAttribIFormat){
			gl.VertexAttribIFormat(index, it->nelem, it->type, it->off);
		} else {
			gl.VertexAttribIPointer(index, it->nelem, it->type, stride, off);
		}
	} else {
		bool norm = it->flags & ATR_NORM;
		if(gl.VertexAttribFormat){
			TRACEF("Set VertexAttribFormat: idx: %d, nelem: %d, type: %x, norm? %d, off: %d",
				index, it->nelem, it->type, norm, it->off);
			gl.VertexAttribFormat(index, it->nelem, it->type, norm, it->off);
		} else {
			gl.VertexAttribPointer(index, it->nelem, it->type, norm, stride, off);
		}
	}
	
	return true;
}

void ShaderAttribs::clear() {
	attribs.clear();	
}

const ShaderAttribs::Attrib* ShaderAttribs::begin() const {
	return attribs.data();
}

const ShaderAttribs::Attrib* ShaderAttribs::end() const {
	return attribs.data() + attribs.size();
}

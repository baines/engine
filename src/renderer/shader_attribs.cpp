#include "shader_attribs.h"
#include <algorithm>

ShaderAttribs::ShaderAttribs(){

}

void ShaderAttribs::initAttrib(uint32_t name_hash, GLuint index){
	attribs.push_back({ name_hash, index });
}

void ShaderAttribs::setAttribFormat(uint32_t hash, GLenum type, int nelem, int off, uint32_t flags){
	auto it = std::find(attribs.begin(), attribs.end(), hash);
	
	assert(it != attribs.end());
	
	it->type = type;
	it->nelem = nelem;
	it->off = off;
	it->flags = flags;
}

bool ShaderAttribs::containsAttrib(uint32_t hash, ssize_t index) const {
	auto it = std::find(attribs.begin(), attribs.end(), hash);
	if(it == attribs.end() || (index != -1 && it->index != index)){
		return false;
	} else {
		return true;
	}
}

bool ShaderAttribs::bind(uint32_t hash) const {
	auto it = std::find(attribs.begin(), attribs.end(), hash);
	
	if(it == attribs.end()) return false;
	
	if(it->flags & ATR_INT){
		gl.VertexAttribIFormat(it->index, it->nelem, it->type, it->off);
	} else {
		bool norm = it->flags & ATR_NORM;
		gl.VertexAttribFormat(it->index, it->nelem, it->type, norm, it->off);
	}
	
	return true;
}

const ShaderAttribs::Attrib* ShaderAttribs::begin() const {
	return attribs.data();
}

const ShaderAttribs::Attrib* ShaderAttribs::end() const {
	return attribs.data() + attribs.size();
}

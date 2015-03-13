#include "sampler.h"
#include "render_state.h"
Sampler::Sampler()
: params()
, id(0){
	if(gl.GenSamplers) gl.GenSamplers(1, &id);
}

Sampler::Sampler(std::initializer_list<Param> params)
: params()
, id(0) {
	if(gl.GenSamplers) gl.GenSamplers(1, &id);
	
	for(auto& p : params){
		setParam(p);
	}
}

void Sampler::setParam(const Param& p){
	setParam(p.key, p.val);
}

void Sampler::setParam(GLenum key, GLint val){
	params[key] = val;
	if(gl.SamplerParameteri){
		gl.SamplerParameteri(id, key, val);
	} else {
		//TODO: fallback if sampler_objects unsupported.
	}
}

void Sampler::bind(size_t tex_unit, RenderState& rs) const {
	if(gl.BindSampler && rs.samp[tex_unit] != id){
		gl.BindSampler(tex_unit, id);
		rs.samp[tex_unit] = id;
	}
}

void Sampler::onGLContextRecreate(){
	if(gl.GenSamplers) gl.GenSamplers(1, &id);
	for(auto& p : params){
		setParam(p.first, p.second);
	}
}

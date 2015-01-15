#include "sampler.h"
#include "render_state.h"
Sampler::Sampler()
: params()
, id(0){
	gl.GenSamplers(1, &id);
}

Sampler::Sampler(std::initializer_list<Param> params)
: params()
, id(0) {
	gl.GenSamplers(1, &id);
	
	for(auto& p : params){
		setParam(p);
	}
}

void Sampler::setParam(const Param& p){
	setParam(p.key, p.val);
}

void Sampler::setParam(GLenum key, GLint val){
	params[key] = val;
	gl.SamplerParameteri(id, key, val);
}

void Sampler::bind(size_t tex_unit, RenderState& rs) const {
	if(rs.samp[tex_unit] != id){
		gl.BindSampler(tex_unit, id);
		rs.samp[tex_unit] = id;
	}
}

void Sampler::onGLContextRecreate(){
	gl.GenSamplers(1, &id);
	for(auto& p : params){
		setParam(p.first, p.second);
	}
}

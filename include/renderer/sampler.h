#ifndef SAMPLER_H_
#define SAMPLER_H_
#include "gl_context.h"
#include <initializer_list>

struct RenderState;

struct Sampler {
	struct Param {
		GLenum key;
		GLint val;
	};
	
	Sampler();
	Sampler(std::initializer_list<Param> params);
	
	void setParam(const Param& p);
	void setParam(GLenum key, GLint val);
	
	bool bind(size_t tex_unit, RenderState& rs) const;
private:
	GLuint id;
};

#endif

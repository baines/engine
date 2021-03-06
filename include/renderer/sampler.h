#ifndef SAMPLER_H_
#define SAMPLER_H_
#include "common.h"
#include "gl_context.h"
#include <initializer_list>
#include <map>

struct Sampler : public GLObject {
	struct Param {
		GLenum key;
		GLint val;
	};
	
	Sampler();
	Sampler(std::initializer_list<Param> params);
	
	void setParam(const Param& p);
	void setParam(GLenum key, GLint val);
	
	void bind(size_t tex_unit, RenderState& rs) const;

	void onGLContextRecreate() override;

	~Sampler();
private:
	std::map<GLenum, GLint> params;
	GLuint id;
};

#endif

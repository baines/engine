#ifndef BLEND_MODE_H_
#define BLEND_MODE_H_
#include "common.h"
#include <GL/gl.h>

struct BlendMode {
	BlendMode();
	BlendMode(const Array<GLenum, 4>& fns, 
	          const Array<GLenum, 2>& eqs = {{ GL_FUNC_ADD, GL_FUNC_ADD }});

	void bind() const;

	bool operator==(const BlendMode& other) const {
		return funcs == other.funcs && equations == other.equations;
	}
private:
	Array<GLenum, 4> funcs;
	Array<GLenum, 2> equations;
};

#endif


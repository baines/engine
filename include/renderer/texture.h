#ifndef TEXTURE_H_
#define TEXTURE_H_
#include "common.h"
#include "util.h"
#include "gl_context.h"
#include <tuple>

struct RenderState;

struct Texture {
	virtual GLenum getType(void) const = 0;
	virtual bool isValid(void) const = 0;
	virtual std::tuple<int, int> getSize() const = 0;
	virtual bool bind(size_t tex_unit, RenderState& rs) const = 0;
	virtual bool setSwizzle(const std::array<GLint, 4>& swizzle) = 0;
	virtual ~Texture(){}
};

struct Texture2D : public Texture, public GLObject {
	Texture2D();
	Texture2D(MemBlock mem);
	Texture2D(GLenum fmt, GLenum int_fmt, int w, int h, const void* data);
	Texture2D& operator=(const Texture2D&) = delete;
	Texture2D& operator=(Texture2D&&);
	
	GLenum getType(void) const;
	bool isValid(void) const;
	std::tuple<int, int> getSize() const;
	bool bind(size_t tex_unit, RenderState& rs) const;
	bool setSwizzle(const std::array<GLint, 4>& swizzle);
	virtual void onGLContextRecreate();	
	virtual ~Texture2D();
private:
	GLuint id;
	int w, h;
};

#endif

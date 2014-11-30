#ifndef TEXTURE_H_
#define TEXTURE_H_
#include "gl_context.h"
#include "resource_system.h"

struct RenderState;

struct Texture {
	virtual GLenum getType(void) const = 0;
	virtual bool isValid(void) const = 0;
	virtual bool bind(size_t tex_unit, RenderState& rs) const = 0;
	virtual ~Texture(){}
};

struct Texture2D : Texture {
	Texture2D();
	Texture2D(GLenum fmt, GLenum int_fmt, int w, int h, const uint8_t* data);
	Texture2D& operator=(const Texture2D&) = delete;
	Texture2D& operator=(Texture2D&&);
	
	void loadFromResource(Engine& e, const ResourceHandle& img);
	
	GLenum getType(void) const;
	bool isValid(void) const;
	bool bind(size_t tex_unit, RenderState& rs) const;
	
	~Texture2D();
private:
	int w, h;
	GLuint id;
};

#endif

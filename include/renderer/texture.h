#ifndef TEXTURE_H_
#define TEXTURE_H_
#include "gl_context.h"
#include "resource_system.h"

struct RenderState;

struct Texture {
	virtual GLenum getType(void) = 0;
	virtual bool isValid(void) = 0;
	virtual bool bind(size_t tex_unit, RenderState& rs) = 0;
	virtual ~Texture(){}
};

struct Texture2D : Texture {
	Texture2D();
	Texture2D(GLenum fmt, GLenum int_fmt, int w, int h, const uint8_t* data);
	
	void loadFromResource(Engine& e, const ResourceHandle& img);
	
	GLenum getType(void);
	bool isValid(void);
	bool bind(size_t tex_unit, RenderState& rs);
	
	~Texture2D();
private:
	int w, h;
	GLuint id;
};

#endif

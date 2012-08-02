#ifndef _RENDERER_H_
#define _RENDERER_H_
#include "glcontext.h"
#include "vec.h"
#include <vector>

typedef struct Vertex{
	short vertx, verty;
	unsigned char textx, texty;
	float rot1, rot2, rot3, rot4;
	unsigned short texID;
} Vertex;

typedef struct {
	struct Vertex v[4];
} Vert4;

class Renderer {
public:
	static Renderer& get();
	void draw(std::vector<uint32_t>& indices);
	Vert4& addQuad(float x, float y, float w, float h, uint32_t* out);
	void delQuad(uint32_t index){ gfx.pop(index/4); }
	void toggleFullscreen(void);
	void toggleBorder();
	int vsync;
	Vec<Vert4> gfx;
protected:
	Renderer(int w, int h, const char* caption);
	void reload(int w, int h, int flags);
	SDL_Surface* screen;
	int fullscreen_w, fullscreen_h, windowed_w, windowed_h;
};

#endif

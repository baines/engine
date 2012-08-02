#include "renderer.h"
#include "texture.h"
#include "SDL/SDL.h"
#include <cstdio>
#include "shader.h"

GLContext gl;

Renderer& Renderer::get(){
	static Renderer r(640, 480, "Engine");
	return r;
}

Renderer::Renderer(int w, int h,const char* caption) : gfx(1024), 
	windowed_w(w), windowed_h(h){
	if(SDL_InitSubSystem(SDL_INIT_VIDEO) != 0){
		fprintf(stderr, "Video error: %s\n", SDL_GetError());
		exit(1);
	}
	SDL_GL_LoadLibrary(NULL);
	SDL_WM_SetCaption(caption, NULL);
	
	fullscreen_w = SDL_GetVideoInfo()->current_w;
	fullscreen_h = SDL_GetVideoInfo()->current_h;
	
	reload(w, h, SDL_OPENGL | SDL_ANYFORMAT);
	SDL_Delay(200);
}

void Renderer::reload(int w, int h, int flags){
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1);
	
	if(!(screen = SDL_SetVideoMode(w, h, 32, flags))){
		fprintf(stderr, "Video error: %s\n", SDL_GetError());
		exit(1);
	}
	gl.createContext(w, h);
	Shader s(gl, "./data/txt/rotate.vs", "./data/txt/rotate.fs");
	Texture::reload();
	
	vsync = 0;
	SDL_GL_GetAttribute(SDL_GL_SWAP_CONTROL, &vsync);
	if(vsync & ~0x01) vsync = 0;
}

void Renderer::toggleFullscreen() {
	int flags = screen->flags ^ SDL_FULLSCREEN;
	
	if((flags & SDL_FULLSCREEN)){
		reload(fullscreen_w, fullscreen_h, flags);
	} else {
		reload(windowed_w, windowed_h, flags);
	}
}

Vert4& Renderer::addQuad(float x, float y, float w, float h, uint32_t* out){
	float _w = w / 2.0f, _h = h / 2.0f;
	short x1 = x - _w, x2 = x + _w, y1 = y - _h, y2 = y + h;
	Vert4 v = {{{ x1, y1, 0, 0, +_w, +_h, }, { x2, y1, 0, 0, -_w, +_h, },
				{ x2, y2, 0, 0, -_w, -_h, }, { x1, y2, 0, 0, +_w, -_h, }}};
	uint16_t r = gfx.push(v);
	*out = r * 4;
	return gfx[r];
}

inline void Renderer::toggleBorder(){
	reload(screen->w, screen->h, screen->flags ^ SDL_NOFRAME);
}

void Renderer::draw(std::vector<uint32_t>& indices){
	if(indices.empty()) return;
	uint32_t i = indices[0] / 4, num = 0, oldnum = 0, sz = indices.size(), tex;
	gl.Clear(GL_COLOR_BUFFER_BIT);
	
	gl.VertexAttribPointerARB(0, 2, GL_SHORT, GL_FALSE, sizeof(Vertex), &(gfx[0].v[0].vertx));
	gl.VertexAttribPointerARB(8, 2, GL_UNSIGNED_BYTE, GL_FALSE, sizeof(Vertex), &(gfx[0].v[0].textx));
	gl.VertexAttribPointerARB(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), &(gfx[0].v[0].rot1));
	
	while(num < sz){
		tex = gfx[i].v[0].texID;
		while((num < sz) && (gfx[i].v[0].texID == tex)){
			num+=4;
			i = indices[num] / 4;
		}
		gl.BindTexture(GL_TEXTURE_2D, tex);
		gl.DrawElements(GL_QUADS, num - oldnum, GL_UNSIGNED_INT, &(indices[oldnum]));
		oldnum = num;
	}

	indices.clear();
	SDL_GL_SwapBuffers();
}


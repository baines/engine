#ifndef RENDERER_H_
#define RENDERER_H_
#include "common.h"

struct SDL_Window;

struct IRenderer {
	virtual void reload(Engine& e) = 0;
	virtual void handleResize(float w, float h) = 0;
	virtual void drawFrame() = 0;
	virtual void addRenderable(Renderable& r) = 0;
	virtual SDL_Window* getWindow() const = 0;
	virtual ~IRenderer(){}
};

#endif

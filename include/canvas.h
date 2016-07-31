#ifndef CANVAS_H_
#define CANVAS_H_
#include "common.h"
#include "vertex_buffer.h"
#include "vertex_state.h"
#include "shader.h"
#include "renderable.h"
#include "resource.h"

struct Canvas {
	Canvas(Engine& e);
	
	void addLine(vec2 from, vec2 to, uint32_t color);
	void addBox(vec2 pos, vec2 size, uint32_t color);
			
	void draw(IRenderer& r);
	void clear();

private:
	DynamicVertexBuffer vertices;
	VertexState vstate;
	Resource<VertShader> vs;
	Resource<FragShader> fs;
	ShaderProgram shader;
	Renderable lines;
};

#endif


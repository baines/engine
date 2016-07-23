#include "canvas.h"
#include "renderer.h"

struct LineVert {
	LineVert(int x, int y, uint8_t (&c)[4]) : x(x), y(y), color{c[0], c[1], c[2], c[3]}{
	}
	int16_t x, y;
	uint8_t color[4];
};

Canvas::Canvas(Engine& e)
: vertices("a_pos:2s|a_col:4BN", 32)
, vstate()
, vs(e, { "color.glslv" })
, fs(e, { "color.glslf" })
, shader(vs, fs) {

	lines.vertex_state = &vstate;
	lines.shader = &shader;
	lines.prim_type = GL_LINES;

	shader.link();
	vstate.setVertexBuffers({ &vertices });
}

void Canvas::addLine(vec2 from, vec2 to, uint32_t color){
	uint8_t c[] = {
		uint8_t(color >> 24),
		uint8_t(color >> 16),
		uint8_t(color >> 8),
		uint8_t(color)
	};
	
	vertices.push(LineVert(from.x, from.y, c));
	vertices.push(LineVert(to.x  , to.y  , c));

	lines.count += 2;
}

void Canvas::addBox(vec2 pos, vec2 size, uint32_t color){
	uint8_t c[] = {
		uint8_t(color >> 24),
		uint8_t(color >> 16),
		uint8_t(color >> 8),
		uint8_t(color)
	};

	size_t w = size.x / 2, h = size.y / 2;

	vertices.push(LineVert(pos.x - w, pos.y - h, c));
	vertices.push(LineVert(pos.x + w, pos.y - h, c));

	vertices.push(LineVert(pos.x + w, pos.y - h, c));
	vertices.push(LineVert(pos.x + w, pos.y + h, c));
	
	vertices.push(LineVert(pos.x + w, pos.y + h, c));
	vertices.push(LineVert(pos.x - w, pos.y + h, c));

	vertices.push(LineVert(pos.x - w, pos.y + h, c));
	vertices.push(LineVert(pos.x - w, pos.y - h, c));

	lines.count += 8;
}
	
void Canvas::clear(){
	vertices.clear();
	lines.count = 0;
}

void Canvas::draw(IRenderer& r){
	r.addRenderable(lines);
}

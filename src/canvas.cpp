#include "canvas.h"
#include "renderer.h"
#include <array>

using glm::vec2;

struct LineVert {
	LineVert(int x, int y, const std::array<uint8_t, 4>& c) : x(x), y(y), color(c){}
	int16_t x, y;
	std::array<uint8_t, 4> color;
};

Canvas::Canvas(Engine& e)
: vertices("a_pos:2s|a_col:4BN", 32)
, vstate()
, vs(e, { "color.glslv" })
, fs(e, { "color.glslf" })
, shader(*vs, *fs)
, lines(&vstate, &shader, RType{ GL_LINES }){
	shader.link();
}

void Canvas::addLine(vec2 from, vec2 to, uint32_t color){
	std::array<uint8_t, 4> c = {
		uint8_t(color >> 24),
		uint8_t(color >> 16),
		uint8_t(color >> 8),
		uint8_t(color)
	};
	
	vertices.push(LineVert(from.x, from.y, c));
	vertices.push(LineVert(to.x  , to.y  , c));

	lines.count += 2;
}

void Canvas::clear(){
	vertices.clear();
	lines.count = 0;
}

void Canvas::draw(Renderer& r){
	r.addRenderable(lines);
}

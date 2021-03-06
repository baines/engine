#include "text.h"
#include "text_system.h"
#include "font.h"
#include "renderer.h"
#include "engine.h"
#include "renderable.h"
#include <glm/glm.hpp>

static const size_t COLORCODE_START = 0xfdd0;
static const size_t COLORCODE_COUNT = 16;

static const std::array<uint32_t, COLORCODE_COUNT> default_palette = {{
	0x000000ff,
	0xa90707ff,
	0x05801fff,
	0x8c8e0bff,
	0x2c2fabff,
	0x960f8eff,
	0x0d5ca0ff,
	0x8d959bff,
	0x414447ff,
	0xdc2c2cff,
	0x28ba1cff,
	0xece63fff,
	0x5053c8ff,
	0xc450c8ff,
	0x50c8c1ff,
	0xffffffff
}};

static bool is_color_code(char32_t c){
	return c >= COLORCODE_START && c < COLORCODE_START + COLORCODE_COUNT;
}

Text::Text()
: engine()
, font()
, palette(default_palette)
, start_pos()
, end_pos()
, str()
, uniforms()
, renderable() {

	uniforms.setUniform("u_samp", { 0 });
	uniforms.setUniform("u_outline_col", { glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) });
}

Text::Text(Engine& e, Proxy<Font> f, glm::ivec2 pos, const alt::StrRef& s)
: engine(&e)
, font(f)
, palette(default_palette)
, start_pos(pos)
, end_pos()
, str(to_utf32(s))
, uniforms()
, renderable(nullptr) {

	e.text->addText(*this);

	uniforms.setUniform("u_samp", { 0 });
	uniforms.setUniform("u_outline_col", { glm::vec4(0.0f, 0.0f, 0.0f, 1.0f) });
}

void Text::setPalette(const std::array<uint32_t, 16>& colors){
	palette = colors;
}

void Text::resetPalette(){
	palette = default_palette;
}

std::array<uint32_t, 16> Text::getDefaultPalette(){
	return default_palette;
}

int Text::update(const alt::StrRef& newstr){
	return update(newstr, start_pos); 
}

int Text::update(const alt::StrRef& newstr, glm::ivec2 newpos){
	if(!engine || !font) return 0;

	alt::StrMut32 u32str = to_utf32(newstr);

	//XXX: tab hack
	for(size_t i = 0; i < u32str.size(); ++i){
		if(u32str[i] == '\t') u32str.replace(i, 1, U"    ");
	}

	if(u32str == str && newpos == start_pos){
		return 0;
	} else {
		int char_count_diff = u32str.size() - str.size();
		engine->text->updateText(*this, u32str, newpos);
		return char_count_diff;
	}
}

void Text::draw(Renderer& r){
	if(renderable){
		renderable->uniforms = &uniforms;
		r.addRenderable(*renderable);
	}
}

void Text::setOutlineColor(uint32_t col){
	auto v = glm::fvec4(
		uint8_t(col >> 24u) / 255.0f,
		uint8_t(col >> 16u) / 255.0f,
		uint8_t(col >> 8u ) / 255.0f,
		uint8_t(col)        / 255.0f
	);
	uniforms.setUniform("u_outline_col", { v });
}

Text::~Text(){
	if(engine) engine->text->delText(*this);
}

void Text::setRenderable(Renderable* r){
	renderable = r;
	renderable->uniforms = &uniforms;
	renderable->textures[0] = font->getTexture();
}

glm::ivec2 Text::getPos(size_t index) const {
	glm::ivec2 result = start_pos;
	index = std::min(index, str.size());

	for(size_t i = 0; i < index; ++i){
		char32_t c = str[i], prev = 0;
		if(is_color_code(c)) continue;

		if(c == '\n'){
			result.x = start_pos.x;
			result.y += font->getLineHeight();
		} else {
			result.x += font->getGlyphInfo(c).advance;
			if(prev) result.x += font->getKerning(prev, c).x;
			prev = c;
		}
	}

	return result;
}


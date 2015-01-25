#include "text.h"
#include "text_system.h"
#include "font.h"
#include "renderer.h"
#include "engine.h"
#include <glm/glm.hpp>

static const size_t COLORCODE_START = 0xfdd0;
static const size_t COLORCODE_COUNT = 16;

static const std::array<uint32_t, COLORCODE_COUNT> default_palette = {
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
};

static bool is_color_code(char32_t c){
	return c >= COLORCODE_START && c < COLORCODE_START + COLORCODE_COUNT;
}

Text::Text()
: engine(nullptr)
, font(nullptr)
, palette(default_palette)
, start_pos()
, end_pos()
, str()
, uniforms()
, renderable(nullptr) {

	uniforms.setUniform("u_samp", { 0 });
}

Text::Text(Engine& e, const std::shared_ptr<Font>& f, glm::ivec2 pos, const string_view& s)
: engine(&e)
, font(&f)
, palette(default_palette)
, start_pos(pos)
, end_pos()
, str(to_utf32(s))
, uniforms()
, renderable(nullptr) {

	e.text.addText(*this);

	uniforms.setUniform("u_samp", { 0 });
}

Text::Text(Text&& other)
: engine(other.engine)
, font(other.font)
, palette(other.palette)
, start_pos(other.start_pos)
, end_pos(other.end_pos)
, str(std::move(other.str))
, uniforms(std::move(other.uniforms))
, renderable(other.renderable) {
	if(renderable) renderable->uniforms = &uniforms;
	other.engine = nullptr;
	other.font = nullptr;
	other.renderable = nullptr;
}

Text& Text::operator=(Text&& other){
	engine = other.engine;
	font = other.font;
	palette = other.palette;
	start_pos = other.start_pos;
	end_pos = other.end_pos;
	str = std::move(other.str);
	uniforms = std::move(other.uniforms);
	renderable = other.renderable;
	if(renderable) renderable->uniforms = &uniforms;
	
	other.engine = nullptr;
	other.font = nullptr;
	other.renderable = nullptr;

	return *this;
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

int Text::update(const string_view& newstr){
	return update(newstr, start_pos); 
}

int Text::update(const string_view& newstr, glm::ivec2 newpos){
	if(!engine || !font) return 0;

	std::u32string u32str = to_utf32(newstr);

	if(u32str == str && newpos == start_pos){
		return 0;
	} else {
		int char_count_diff = u32str.size() - str.size();
		engine->text.updateText(*this, u32str, newpos);
		return char_count_diff;
	}
}

void Text::draw(Renderer& r){
	if(renderable){
		r.addRenderable(*renderable);
	}
}

Text::~Text(){
	if(engine) engine->text.delText(*this);
}

void Text::setRenderable(Renderable* r){
	renderable = r;
	renderable->uniforms = &uniforms;
	renderable->textures[0] = (*font)->getTexture();
}

glm::ivec2 Text::getPos(size_t index) const {
	glm::ivec2 result = start_pos;
	index = std::min(index, str.size());

	for(size_t i = 0; i < index; ++ i){
		char32_t c = str[i];
		if(is_color_code(c)) continue;

		if(c == '\n'){
			result.x = start_pos.x;
			result.y += (*font)->getLineHeight();
		} else {
			result.x += (*font)->getGlyphInfo(c).width;
		}
	}

	return result;
}


#include "text.h"
#include "text_system.h"
#include "font.h"
#include "renderer.h"
#include "engine.h"
#include <glm/glm.hpp>

Text::Text()
: engine(nullptr)
, font(nullptr)
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

		if(c == '\n'){
			result.x = start_pos.x;
			result.y += (*font)->getLineHeight();
		} else {
			result.x += (*font)->getGlyphInfo(c).width;
		}
	}

	return result;
}


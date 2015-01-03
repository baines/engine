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

bool Text::update(const string_view& newstr){
	return update(newstr, start_pos); 
}

bool Text::update(const string_view& newstr, glm::ivec2 newpos){
	if(!engine || !font) return false;

	std::u32string u32str = to_utf32(newstr);

	if(u32str == str && newpos == start_pos){
		return true;
	} else {
		return engine->text.updateText(*this, u32str, newpos);
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


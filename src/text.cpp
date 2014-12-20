#include "text.h"
#include "text_system.h"
#include "font.h"
#include "renderer.h"
#include "engine.h"
#include <glm/glm.hpp>

Text::Text()
: engine(nullptr)
, font(nullptr)
, pos()
, total_width(0)
, str()
, uniforms()
, renderable(nullptr) {

	uniforms.setUniform("u_samp", { 0 });
}

Text::Text(Engine& e, const Font& f, glm::ivec2 pos, const string_view& s)
: engine(&e)
, font(&f)
, pos(pos)
, total_width(0)
, str(std::move(s.to_string()))
, uniforms()
, renderable(nullptr) {

	e.text.addText(*this);

	uniforms.setUniform("u_samp", { 0 });
}

Text::Text(Text&& other)
: engine(other.engine)
, font(other.font)
, pos(other.pos)
, total_width(other.total_width)
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
	pos = other.pos;
	total_width = other.total_width;
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
	if(!engine || !font) return false;

	return engine->text.updateText(*this, newstr);
}

bool Text::update(const string_view& newstr, glm::ivec2 newpos){
	pos = newpos;
	return update(newstr);
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
	renderable->textures[0] = font->getTexture();
}


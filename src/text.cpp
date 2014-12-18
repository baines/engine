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
, uniforms()
, renderable(nullptr) {

}

Text::Text(Engine& e, const Font& f, const glm::ivec2& pos, const string_view& str)
: engine(&e)
, font(&f)
, pos(pos)
, uniforms()
, renderable(e.text.addText(f, pos, str)){

	const Texture2D* tex = f.getTexture();
	
	renderable->uniforms = &uniforms;
	renderable->textures[0] = tex;
	
	uniforms.setUniform("u_samp", { 0 });
}

Text::Text(Text&& other)
: engine(other.engine)
, font(other.font)
, pos(other.pos)
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

	return engine->text.updateText(renderable, *font, pos, newstr);
}

void Text::draw(Renderer& r){
	if(renderable){
		r.addRenderable(*renderable);
	}
}

Text::~Text(){
	if(engine) engine->text.delText(renderable);
}


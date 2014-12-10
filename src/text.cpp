#include "text.h"
#include "text_system.h"
#include "font.h"
#include "renderer.h"
#include "engine.h"
#include <glm/glm.hpp>

Text::Text()
: engine(nullptr)
, uniforms()
, renderable() {

}

Text::Text(Engine& e, const Font& f, const glm::ivec2& pos, const string_view& str)
: engine(&e)
, uniforms()
, renderable(e.text.addText(f, pos, str)){

	const Texture2D* tex = f.getTexture();
	
	renderable.uniforms = &uniforms;
	renderable.textures[0] = tex;
	
	uniforms.setUniform("u_samp", { 0 });
}

Text::Text(Text&& other)
: engine(other.engine)
, uniforms(std::move(other.uniforms))
, renderable(std::move(other.renderable)) {
	renderable.uniforms = &uniforms;
	other.engine = nullptr;
}

Text& Text::operator=(Text&& other){
	engine = other.engine;
	uniforms = std::move(other.uniforms);
	renderable = std::move(other.renderable);
	renderable.uniforms = &uniforms;
	
	other.engine = nullptr;

	return *this;
}

bool Text::update(const string_view& newstr){
	//TODO
	return false;
}

void Text::draw(Renderer& r){
	if(renderable.count > 0){
		r.addRenderable(renderable);
	}
}

Text::~Text(){
	if(engine) engine->text.delText(renderable);
}


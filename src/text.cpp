#include "text.h"
#include "text_system.h"
#include "font.h"
#include "renderer.h"
#include "engine.h"
#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

Text::Text()
: engine(nullptr)
, str()
, uniforms()
, renderable() {

}

Text::Text(Engine& e, const Font& f, const std::string& str, size_t max_len)
: engine(&e)
, str(str)
, uniforms()
, renderable(e.text.addText(f, str, max_len ? max_len : str.size())){

	renderable.uniforms = &uniforms;
	renderable.textures[0] = f.getTexture();
	uniforms.setUniform("u_samp", { 0 });
	uniforms.setUniform("u_mvp", { glm::ortho(0.f, 640.f, 480.f, 0.f) });
}

Text::Text(Text&& other)
: engine(other.engine)
, str(std::move(other.str))
, uniforms(std::move(other.uniforms))
, renderable(std::move(other.renderable)) {
	renderable.uniforms = &uniforms;
	other.engine = nullptr;
}

Text& Text::operator=(Text&& other){
	engine = other.engine;
	str = std::move(other.str);
	uniforms = std::move(other.uniforms);
	renderable = std::move(other.renderable);
	renderable.uniforms = &uniforms;
	
	other.engine = nullptr;

	return *this;
}

bool Text::update(const std::string& newstr){
	//TODO
}

void Text::draw(Renderer& r){
	if(renderable.count > 0){
		r.addRenderable(renderable);
	}
}

Text::~Text(){
	if(engine) engine->text.delText(renderable);
}


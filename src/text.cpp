#include "text.h"
#include "text_system.h"
#include "font.h"
#include "renderer.h"
#include "engine.h"

Text::Text()
: engine(nullptr)
, str()
, uniforms()
, renderable() {

}

//XXX: x / y / z position?
Text::Text(Engine& e, const Font& f, const std::string& str, size_t max_len)
: engine(&e)
, str(str)
, uniforms()
, renderable(e.text.addText(f, str, max_len ? max_len : str.size())){

	const Texture2D* tex = f.getTexture();
	
	renderable.uniforms = &uniforms;
	renderable.textures[0] = tex;
	
	uniforms.setUniform("u_samp", { 0 });
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


#ifndef TEXT_H_
#define TEXT_H_
#include "common.h"
#include "text.h"
#include "shader_uniforms.h"
#include "renderable.h"
#include <string>

struct Font;

struct Text {
	Text();
	Text(Engine& e, const Font& f, const glm::ivec2& pos, const string_view& s);
	Text(Text&&);
	Text& operator=(Text&&);
	
	bool update(const string_view& newstr);
	void draw(Renderer& r);
	
	~Text();
private:
	friend class TextSystem;

	void setRenderable(Renderable* r);

	Engine* engine;
	const Font* font;
	glm::ivec2 pos;
	size_t total_width;
	std::string str;
	ShaderUniforms uniforms;
	Renderable* renderable;
};

#endif

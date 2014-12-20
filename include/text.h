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
	Text(Engine& e, const Font& f, glm::ivec2 pos, const string_view& s);
	Text(Text&&);
	Text& operator=(Text&&);
	
	bool update(const string_view& newstr);
	bool update(const string_view& newstr, glm::ivec2 newpos);

	void draw(Renderer& r);

	const std::string& getStr() const {
		return str;
	}

	glm::ivec2 getPos() const {
		return pos;
	}

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

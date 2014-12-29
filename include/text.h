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

	glm::ivec2 getStartPos() const {
		return start_pos;
	}

	glm::ivec2 getEndPos() const {
		return end_pos;
	}

	size_t size() const {
		return str.size();
	}

	~Text();
private:
	friend class TextSystem;

	void setRenderable(Renderable* r);

	Engine* engine;
	const Font* font;
	glm::ivec2 start_pos, end_pos;
	std::u32string str;
	ShaderUniforms uniforms;
	Renderable* renderable;
};

#endif

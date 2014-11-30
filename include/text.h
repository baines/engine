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
	Text(Engine& e, const Font& f, const std::string& s, size_t max_len = 0);
	Text(Text&&);
	Text& operator=(Text&&);
	
	bool update(const std::string& newstr);
	void draw(Renderer& r);
	
	~Text();
private:
	Engine* engine;
	std::string str;
	ShaderUniforms uniforms;
	Renderable renderable;
};

#endif

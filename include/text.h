#ifndef TEXT_H_
#define TEXT_H_
#include "common.h"
#include "text.h"
#include "shader_uniforms.h"
#include "renderable.h"
#include <string>

struct Font;

#define TXT_BLACK         "\xef\xb7\x90"
#define TXT_DARK_RED      "\xef\xb7\x91"
#define TXT_DARK_GREEN    "\xef\xb7\x92"
#define TXT_DARK_YELLOW   "\xef\xb7\x93"
#define TXT_DARK_BLUE     "\xef\xb7\x94"
#define TXT_DARK_MAGENTA  "\xef\xb7\x95"
#define TXT_DARK_CYAN     "\xef\xb7\x96"
#define TXT_GRAY          "\xef\xb7\x97"
#define TXT_DARK_GRAY     "\xef\xb7\x98"
#define TXT_RED           "\xef\xb7\x99"
#define TXT_GREEN         "\xef\xb7\x9a"
#define TXT_YELLOW        "\xef\xb7\x9b"
#define TXT_BLUE          "\xef\xb7\x9c"
#define TXT_MAGENTA       "\xef\xb7\x9d"
#define TXT_CYAN          "\xef\xb7\x9e"
#define TXT_WHITE         "\xef\xb7\x9f"

struct Text {
	Text();
	Text(Engine& e, const std::shared_ptr<Font>& f, glm::ivec2 pos, const string_view& s);
	Text(Text&&);
	Text& operator=(Text&&);
	
	int update(const string_view& newstr);
	int update(const string_view& newstr, glm::ivec2 newpos);

	void draw(Renderer& r);

	glm::ivec2 getStartPos() const {
		return start_pos;
	}
	glm::ivec2 getEndPos() const {
		return end_pos;
	}
	glm::ivec2 getPos(size_t index) const;

	size_t size() const {
		return str.size();
	}

	~Text();
private:
	friend class TextSystem;

	void setRenderable(Renderable* r);

	Engine* engine;
	const std::shared_ptr<Font>* font;
	std::array<uint32_t, 16> palette;
	glm::ivec2 start_pos, end_pos;
	std::u32string str;
	ShaderUniforms uniforms;
	Renderable* renderable;
};

#endif

#ifndef TEXT_H_
#define TEXT_H_
#include "common.h"
#include "text.h"
#include "shader_uniforms.h"
#include "renderable.h"
#include "proxy.h"
#include <string>

struct Font;

#define TXT_COLOR(c)      "\xef\xb7" STRINGIFY(\x9 ## c)

#define TXT_BLACK         TXT_COLOR(0)
#define TXT_DARK_RED      TXT_COLOR(1)
#define TXT_DARK_GREEN    TXT_COLOR(2)
#define TXT_DARK_YELLOW   TXT_COLOR(3)
#define TXT_DARK_BLUE     TXT_COLOR(4)
#define TXT_DARK_MAGENTA  TXT_COLOR(5)
#define TXT_DARK_CYAN     TXT_COLOR(6)
#define TXT_GRAY          TXT_COLOR(7)
#define TXT_DARK_GRAY     TXT_COLOR(8)
#define TXT_RED           TXT_COLOR(9)
#define TXT_GREEN         TXT_COLOR(a)
#define TXT_YELLOW        TXT_COLOR(b)
#define TXT_BLUE          TXT_COLOR(c)
#define TXT_MAGENTA       TXT_COLOR(d)
#define TXT_CYAN          TXT_COLOR(e)
#define TXT_WHITE         TXT_COLOR(f)

struct Text {
	Text();
	Text(Engine& e, Proxy<Font> f, glm::ivec2 pos, const string_view& s);
	Text(Text&&) = default;
	Text& operator=(Text&&) = default;
	
	int update(const string_view& newstr);
	int update(const string_view& newstr, glm::ivec2 newpos);

	void draw(Renderer& r);

	void setPalette(const std::array<uint32_t, 16>& colors);
	void resetPalette();

	static std::array<uint32_t, 16> getDefaultPalette();

	void setOutlineColor(uint32_t col);

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
	friend struct TextSystem;

	void setRenderable(Renderable* r);

	NullOnMovePtr<Engine> engine;
	Proxy<Font> font;
	std::array<uint32_t, 16> palette;
	glm::ivec2 start_pos, end_pos;
	std::u32string str;
	ShaderUniforms uniforms;
	NullOnMovePtr<Renderable> renderable;
};

#endif

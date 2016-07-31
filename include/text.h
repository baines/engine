#ifndef TEXT_H_
#define TEXT_H_
#include "common.h"
#include "util.h"
#include "shader_uniforms.h"
#include "renderable.h"
#include "proxy.h"

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
	Text(Engine& e, Proxy<Font> f, vec2i pos, const StrRef& s);
	Text(Text&&) = default;
	Text& operator=(Text&&) = default;
	
	int update(const StrRef& newstr);
	int update(const StrRef& newstr, vec2i newpos);

	void draw(IRenderer& r);

	void setPalette(const Array<uint32_t, 16>& colors);
	void resetPalette();

	static Array<uint32_t, 16> getDefaultPalette();

	void setOutlineColor(uint32_t col);

	vec2i getStartPos() const {
		return start_pos;
	}
	vec2i getEndPos() const {
		return end_pos;
	}
	vec2i getPos(size_t index) const;

	size_t size() const {
		return str.size();
	}

	~Text();
private:
	friend struct TextSystem;

	void setRenderable(Renderable* r);

	NullOnMovePtr<Engine> engine;
	Proxy<Font> font;
	Array<uint32_t, 16> palette;
	vec2i start_pos, end_pos;
	StrMut32 str;
	ShaderUniforms uniforms;
	Renderable renderable;
};

#endif

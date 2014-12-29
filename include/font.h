#ifndef FONT_H_
#define FONT_H_
#include "common.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include "texture.h"

struct Font {
	struct GlyphInfo {
		FT_UInt index;
		uint32_t width, x, y;
	};
	
	Font(size_t height, uint16_t utf_lo = 0x0020, uint16_t utf_hi = 0x007F);
	
	std::tuple<uint16_t, uint16_t> getUTFRange() const;
	size_t getLineHeight() const;
	const GlyphInfo& getGlyphInfo(char32_t c) const;
	const Texture2D* getTexture() const;
	
	bool loadFromResource(Engine& e, const ResourceHandle& rh);

private:
	std::vector<GlyphInfo> glyph_info;
	
	size_t height;
	uint16_t utf_lo, utf_hi;
	FT_Face face;
	Texture2D atlas;
};

#endif


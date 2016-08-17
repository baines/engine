#ifndef FONT_H_
#define FONT_H_
#include "common.h"
#include "texture.h"
#include "gl_context.h"
#include <vector>

struct FT_FaceRec_;

struct Font : public GLObject {
	struct GlyphInfo {
		uint32_t index;
		int bearing_x;
		uint32_t advance, width, x, y;
	};
	
	Font(MemBlock, size_t height, uint16_t utf_lo = 0x20, uint16_t utf_hi = 0x7F);
	std::tuple<uint16_t, uint16_t> getUTFRange() const;
	vec2i getKerning(char32_t a, char32_t b) const;
	size_t getLineHeight() const;
	const GlyphInfo& getGlyphInfo(char32_t c) const;
	const Texture2D* getTexture() const;

	~Font();
private:
	std::vector<GlyphInfo> glyph_info;
	
	size_t height;
	uint16_t utf_lo, utf_hi;
	FT_FaceRec_* face;
	Texture2D atlas;
};

#endif


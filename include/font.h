#ifndef FONT_H_
#define FONT_H_
#include "common.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include "texture.h"

struct Font {
	Font(size_t height, uint16_t utf_lo = 0, uint16_t utf_hi = 0xFFFF);
	bool loadFromResource(Engine& e, const ResourceHandle& rh);

private:
	size_t height;
	FT_Face face;
	Texture2D atlas;
};

#endif


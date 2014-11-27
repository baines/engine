#include "font.h"
#include "engine.h"
#include "text_system.h"

Font::Font(size_t h, uint16_t utf_lo, uint16_t utf_hi)
: height(h) {

}

bool Font::loadFromResource(Engine& e, const ResourceHandle& res){
	FT_Library& ft_lib = e.text.getLib();
	
	assert(FT_New_Memory_Face(ft_lib, res.data(), res.size(), 0, &face) == 0);

	// assert FT_IS_SCALABLE
	// get num_glyph, height, scale + max_advance
		// guess texture size
	// alloc buffer
	// load all glyphs from utf_lo - utf_hi
		// render into buffer
		// realloc lines?
	
	// create texture GL_UNSIGNED_BYTE, GL_R8
		// test ARB_texture_compression_rgtc?
	// store in vector per glyph: x_off, y_off, w, h
}


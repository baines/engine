#include "font.h"
#include "engine.h"
#include "text_system.h"
#include <tuple>

struct GlyphBitmapInfo {
	FT_Bitmap* bmp;
	int bearing_x, bearing_y;
};

struct GlyphTextureAtlas {
	int w, h, pen_x, pen_y;
	const size_t line_height;
	const FT_Pos ascender, descender;
	uint8_t* mem;
};

namespace {

static void render_glyph(const GlyphBitmapInfo& glyph, GlyphTextureAtlas& img){
	const FT_Bitmap* bmp = glyph.bmp;
	int prev_w = img.w, prev_h = img.h;
	
	if(img.pen_x + glyph.bearing_x + glyph.bmp->width > img.w){
		img.pen_x = 0;
		img.pen_y += img.line_height;
	}
	
	if(img.pen_y > img.h){
		img.h = img.pen_y;
		img.mem = (uint8_t*)realloc(img.mem, img.w * img.h);
		memset(img.mem + (prev_w * prev_h), 0, (img.w * img.h) - (prev_w * prev_h));
	}

	const int rows_avail = std::min<int>(img.line_height, img.descender + glyph.bearing_y);
	const int rows = std::min(rows_avail, bmp->rows);
	
	for(int i = 0; i < rows; ++i){
		
		const int x = img.pen_x + glyph.bearing_x;
		const int y = img.pen_y - rows_avail + i;
		
		memcpy(img.mem + x + (img.w * y), bmp->buffer + bmp->pitch * i, bmp->width);
	}
}

inline uint32_t next_pow_of_2(uint32_t v){
	--v;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	return ++v;
}

}

Font::Font(size_t h, uint16_t utf_lo, uint16_t utf_hi)
: glyph_info()
, height(h)
, utf_lo(utf_lo)
, utf_hi(utf_hi)
, face(nullptr)
, atlas() {

}

std::tuple<uint16_t, uint16_t> Font::getUTFRange() const {
	return std::make_tuple(utf_lo, utf_hi);
}

size_t Font::getLineHeight() const {
	return height;
}

const Texture2D* Font::getTexture() const {
	return &atlas;
}

const Font::GlyphInfo* Font::getGlyphInfo(size_t idx) const {
	if(idx >= glyph_info.size()){
		return nullptr;
	} else {
		return &glyph_info[idx];
	}
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
	
	FT_Select_Charmap(face, ft_encoding_unicode);
	
	assert(FT_IS_SCALABLE(face));
		
	double scale = (double)face->units_per_EM / (double)face->height;
	assert(FT_Set_Pixel_Sizes(face, 0, height * scale) == 0);
	
	DEBUGF("Font info:\n\tmax_advance: %d\n\tnum_glyphs: %d.",
	       face->size->metrics.max_advance >> 6, face->num_glyphs);
	
	int init_w = 0;
	
	for(init_w = 256; init_w <= 8129; init_w <<= 1){
		int glyphs_per_line = init_w / (face->size->metrics.max_advance >> 6);
		int lines = std::min<int>(utf_hi - utf_lo, face->num_glyphs) / glyphs_per_line;
		
		if(lines * height <= init_w) break;
	}
		
	GlyphTextureAtlas img = {
		init_w,
		0,
		0,
		static_cast<int>(height),
		height,
		face->size->metrics.ascender >> 6,
		-(face->size->metrics.descender >> 6),
		nullptr
	};
	
	bool render_unknown = true;
	
	for(uint16_t i = utf_lo; i < utf_hi; ++i){
		GlyphInfo glyph = {};
		bool render = true;
		
		if(!render_unknown){
			glyph.index = FT_Get_Char_Index(face, i);
		
			for(auto& i : glyph_info){
				if(i.index == glyph.index){
					render = false;
					glyph = i;
					break;
				}
			}
		} else {
			render_unknown = false;
			--i;
		}
		
		const FT_Int32 flags = FT_LOAD_RENDER | FT_LOAD_FORCE_AUTOHINT;
		
		if(render && FT_Load_Glyph(face, glyph.index, flags) == 0){
			const GlyphBitmapInfo bmpinfo = {
				&face->glyph->bitmap,
				face->glyph->bitmap_left,
				face->glyph->bitmap_top
			};
		
			render_glyph(bmpinfo, img);
			
			glyph.width = face->glyph->advance.x >> 6;
			glyph.x = img.pen_x;
			glyph.y = img.pen_y - img.line_height;
			
			img.pen_x += glyph.width;
		}
		
		glyph_info.push_back(glyph);
	}
	
	int old_h = img.h;
	img.h = next_pow_of_2(img.h);
	img.mem = (uint8_t*)realloc(img.mem, img.w * img.h);
	memset(img.mem + (img.w * old_h), 0, (img.w * img.h) - (img.w * old_h));
	
	GLenum int_fmt = GL_R8;
	
	if(gl.hasExtension("ARB_texture_compression_rgtc")
	|| gl.hasExtension("EXT_texture_compression_rgtc")){
		//int_fmt = GL_COMPRESSED_RED_RGTC1;
	}
	
	atlas = Texture2D(GL_UNSIGNED_BYTE, int_fmt, img.w, img.h, img.mem);

	free(img.mem);	
}


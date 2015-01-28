#include "font.h"
#include "engine.h"
#include "text_system.h"
#include <tuple>
#include FT_STROKER_H

struct GlyphBitmapInfo {
	FT_Bitmap* bmp;
	long bearing_x, bearing_y, advance;
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
	
	if(img.pen_x + glyph.advance > img.w){
		img.pen_x = 0;
		img.pen_y += img.line_height;
	}
	
	if(img.pen_y > img.h){
		img.h = img.pen_y;
		img.mem = (uint8_t*)realloc(img.mem, img.w * img.h);
		memset(img.mem + (prev_w * prev_h), 0, (img.w * img.h) - (prev_w * prev_h));
	}

	const int rows_avail = std::min<int>(img.line_height, 
		img.line_height - (img.ascender - glyph.bearing_y)
	);
	const int rows = std::min(rows_avail, bmp->rows);
	
	for(int i = 0; i < rows; ++i){
		
		const int x = std::max<int>(0, img.pen_x + glyph.bearing_x);
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

const Font::GlyphInfo& Font::getGlyphInfo(char32_t c) const {
	size_t idx = std::max<int>(0, (c + 1) - (int)utf_lo);
	if(idx >= glyph_info.size()) idx = 0;

	return glyph_info[idx];
}

bool Font::loadFromResource(Engine& e, const ResourceHandle& res){
	glyph_info.clear();
	face = nullptr;

	FT_Library& ft_lib = e.text.getLib();
	
	assert(FT_New_Memory_Face(ft_lib, res.data(), res.size(), 0, &face) == 0);
	
	FT_Select_Charmap(face, ft_encoding_unicode);
	
	assert(FT_IS_SCALABLE(face));

	FT_Stroker ft_stroker = nullptr;
	FT_Stroker_New(ft_lib, &ft_stroker);
	FT_Stroker_Set(
		ft_stroker,
		4 * height,
		FT_STROKER_LINECAP_SQUARE,
		FT_STROKER_LINEJOIN_MITER_FIXED,
		4 << 16
	);
		
	double scale = (double)face->units_per_EM / (double)face->height;
	assert(FT_Set_Pixel_Sizes(face, 0, height * scale) == 0);
	
	DEBUGF("Font info:\n\tmax_advance: %d\n\tnum_glyphs: %d, asc: %d, desc: %d",
	       face->size->metrics.max_advance >> 6,
	       face->num_glyphs,
	       face->size->metrics.ascender >> 6,
	       -(face->size->metrics.descender >> 6));
	
	int init_w = 0, max_w = 0;

	//XXX: cache common constants like this in GLContext
	gl.GetIntegerv(GL_MAX_TEXTURE_SIZE, &max_w);
	bool got_size = false;

	for(init_w = 256; init_w <= max_w; init_w <<= 1){
		size_t glyphs_per_line = init_w / (face->size->metrics.max_advance >> 6);
		size_t lines = std::min<int>(utf_hi - utf_lo, face->num_glyphs) / glyphs_per_line;
		
		if(lines * height <= (size_t)init_w){
			got_size = true;
			break;
		}
	}
	
	if(!got_size){
		log(logging::warn, "Font texture might be too big for OpenGL.");
	}
		
	GlyphTextureAtlas glyph_tex = {
		init_w,
		0,
		0,
		static_cast<int>(height),
		height,
		face->size->metrics.ascender >> 6,
		-(face->size->metrics.descender >> 6),
		nullptr
	}, outline_tex = glyph_tex;
	
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

		FT_Int32 flags = 0;
		if(height >= 16) flags |= FT_LOAD_FORCE_AUTOHINT;
		
		if(render && FT_Load_Glyph(face, glyph.index, flags) == 0){
			FT_Glyph ft_glyph = nullptr, ft_outline = nullptr;
			assert(FT_Get_Glyph(face->glyph, &ft_glyph) == 0);
			assert(FT_Glyph_Copy(ft_glyph, &ft_outline) == 0);

			assert(FT_Glyph_StrokeBorder(&ft_outline, ft_stroker, 0, 1) == 0);

			assert(FT_Glyph_To_Bitmap(&ft_glyph,   FT_RENDER_MODE_NORMAL, nullptr, 1) == 0);
			assert(FT_Glyph_To_Bitmap(&ft_outline, FT_RENDER_MODE_NORMAL, nullptr, 1) == 0);

			FT_BitmapGlyph ft_glyph_bmp   = (FT_BitmapGlyph)ft_glyph;
			FT_BitmapGlyph ft_outline_bmp = (FT_BitmapGlyph)ft_outline;

			long width = std::max<long>({
				ft_glyph->advance.x >> 16,
				ft_outline->advance.x >> 16,
				ft_glyph_bmp->bitmap.width,
				ft_outline_bmp->bitmap.width
			});
			
			int outline_left = ft_outline_bmp->left, glyph_left = ft_glyph_bmp->left;
			if(outline_left < 0){
				glyph_left -= outline_left;
				outline_left = 0;
			}

			const GlyphBitmapInfo glyph_info = {
				&ft_glyph_bmp->bitmap,
				glyph_left,
				ft_glyph_bmp->top,
				width
			}, outline_info = {
				&ft_outline_bmp->bitmap,
				outline_left,
				ft_outline_bmp->top,
				width
			};

			render_glyph(outline_info, outline_tex);
			render_glyph(glyph_info, glyph_tex);
			
			glyph.width = width - outline_left;
			glyph.x = std::min(
				outline_tex.pen_x + outline_info.bearing_x,
				glyph_tex.pen_x + glyph_info.bearing_x
			);
			glyph.y = outline_tex.pen_y - outline_tex.line_height;
			
			glyph_tex.pen_x   += 1 + width;
			outline_tex.pen_x += 1 + width;

			FT_Done_Glyph(ft_glyph);
			FT_Done_Glyph(ft_outline);

			FT_Stroker_Rewind(ft_stroker);
		}
		
		glyph_info.push_back(glyph);
	}

	FT_Stroker_Done(ft_stroker);

	size_t sz = glyph_tex.w * glyph_tex.h,
		   combined_w = glyph_tex.w,
		   combined_h = next_pow_of_2(glyph_tex.h);

	uint8_t* combined = new uint8_t[combined_w * combined_h * 2]();

	for(size_t i = 0; i < sz * 2; ++i){
		combined[i] = (i % 2) ? outline_tex.mem[i/2] : glyph_tex.mem[i/2];
	}
	
	GLenum int_fmt = GL_LUMINANCE8_ALPHA8;
	
	//TODO: use glInternalFormatQuery instead of extensions (if available)?
	if(gl.hasExtension("ARB_texture_compression_rgtc")
	|| gl.hasExtension("EXT_texture_compression_rgtc")){
		int_fmt = GL_COMPRESSED_RG_RGTC2;
	} else if(gl.version > 30 || gl.hasExtension("ARB_texture_rg")){
		int_fmt = GL_RG8;
	}
	
	gl.validateObject(atlas);
	atlas = Texture2D(GL_UNSIGNED_BYTE, int_fmt, combined_w, combined_h, combined);
	if(int_fmt != GL_LUMINANCE8_ALPHA8){
		atlas.setSwizzle({ GL_RED, GL_RED, GL_RED, GL_GREEN });
	}

	free(glyph_tex.mem);
	free(outline_tex.mem);
	delete [] combined;
	
	return true;
}


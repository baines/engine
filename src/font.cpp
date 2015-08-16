#include "font.h"
#include "engine.h"
#include "text_system.h"
#include <tuple>
#include FT_STROKER_H

struct GlyphBitmapInfo {
	FT_Bitmap* bmp;
	long bearing_x, bearing_y;
	uint32_t advance;
};

struct GlyphTextureAtlas {
	uint32_t w, h, pen_x, pen_y;
	const size_t line_height;
	const int descender;
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

	const int y_off = std::min<int>(img.line_height, img.descender + glyph.bearing_y);
	const int rows = std::min<int>(y_off, bmp->rows);
	
	for(int i = 0; i < rows; ++i){
		
		const int x = std::max<int>(0, img.pen_x + glyph.bearing_x);
		const int y = (img.pen_y - y_off) + i;
		
		memcpy(img.mem + x + (img.w * y), bmp->buffer + bmp->pitch * i, bmp->width);
	}
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

glm::ivec2 Font::getKerning(char32_t a, char32_t b) const {
	FT_Vector vec = {};
	FT_Get_Kerning(
		face,
		FT_Get_Char_Index(face, a),
		FT_Get_Char_Index(face, b),
		FT_KERNING_DEFAULT,
		&vec
	);
	return glm::ivec2(vec.x >> 6, vec.y >> 6);
}

bool Font::loadFromResource(Engine& e, const ResourceHandle& res){
	glyph_info.clear();
	face = nullptr;

	FT_Library& ft_lib = e.text.getLib();
	assert(FT_New_Memory_Face(ft_lib, res.data(), res.size(), 0, &face) == 0);
	FT_Select_Charmap(face, ft_encoding_unicode);
	assert(FT_IS_SCALABLE(face));

	// stroker to create the font outline.
	FT_Stroker ft_stroker = nullptr;
	FT_Stroker_New(ft_lib, &ft_stroker);
	FT_Stroker_Set(
		ft_stroker,
	 	2 * height, //TODO: custom outline width / none at all?
		FT_STROKER_LINECAP_SQUARE,
		FT_STROKER_LINEJOIN_MITER_FIXED,
		4 << 16
	);
	
	// choose a font size that the requested height fits the line height of the outlined glyphs.
	double scale = (double)face->units_per_EM / (double)(face->height + 4 * height);
	assert(FT_Set_Pixel_Sizes(face, 0, height * scale) == 0);
	
	DEBUGF("Font info:\n\th: %d\n\tmax_advance: %d\n\tnum_glyphs: %d, asc: %d, desc: %d",
	       (int)face->size->metrics.height >> 6,
	       (int)face->size->metrics.max_advance >> 6,
	       (int)face->num_glyphs,
	       (int)face->size->metrics.ascender >> 6,
	       (int)-(face->size->metrics.descender >> 6));
	
	int init_w = 0, max_w = 0;

	//XXX: cache common constants like this in GLContext
	gl.GetIntegerv(GL_MAX_TEXTURE_SIZE, &max_w);
	bool got_size = false;

	// estimate the lowest POT texture width that will fit the glyphs.
	for(init_w = 256; init_w <= max_w; init_w <<= 1){
		size_t glyphs_per_line = init_w / (face->size->metrics.max_advance >> 6);
		size_t lines = std::min<int>(utf_hi - utf_lo, face->num_glyphs) / glyphs_per_line;
		
		if(lines * height <= (size_t)init_w){
			got_size = true;
			break;
		}
	}
	if(!got_size)log(logging::warn, "Font texture might be too big for OpenGL.");

	// calculate a more accurate descender for our line height.
	int desc = std::round((
		static_cast<double>(abs(face->descender)) /
		static_cast<double>(
			face->ascender + abs(face->descender)
		)) * static_cast<double>(height)
	);

	// initialise two 8bit texture atlases for the glyphs and outlines. They get merged later.
	GlyphTextureAtlas glyph_tex = {
		static_cast<uint32_t>(init_w),
		0,
		0,
		static_cast<uint32_t>(height),
		height,
		desc,
		nullptr
	}, outline_tex = glyph_tex;
	
	// always render the "unknown" glyph at index 0 to the atlas first for missing characters.
	bool render_unknown = true;
	
	for(uint16_t i = utf_lo; i < utf_hi; ++i){
		GlyphInfo glyph = {};
		bool render = true;
		
		if(!render_unknown){
			glyph.index = FT_Get_Char_Index(face, i);
		
			// don't render duplicate glyphs.
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

		// The FT autohinter looks better at large font sizes imo.
		FT_UInt flags = height >= 16 ? FT_LOAD_FORCE_AUTOHINT : 0;

		// load and render the glyphs and outlines to their respective atlases.
		if(render && FT_Load_Glyph(face, glyph.index, flags) == 0){
			FT_Glyph ft_glyph = nullptr, ft_outline = nullptr;
			assert(FT_Get_Glyph(face->glyph, &ft_glyph) == 0);
			assert(FT_Glyph_Copy(ft_glyph, &ft_outline) == 0);

			assert(FT_Glyph_StrokeBorder(&ft_outline, ft_stroker, 0, 1) == 0);

			assert(FT_Glyph_To_Bitmap(&ft_glyph,   FT_RENDER_MODE_NORMAL, nullptr, 1) == 0);
			assert(FT_Glyph_To_Bitmap(&ft_outline, FT_RENDER_MODE_NORMAL, nullptr, 1) == 0);

			FT_BitmapGlyph ft_glyph_bmp   = (FT_BitmapGlyph)ft_glyph;
			FT_BitmapGlyph ft_outline_bmp = (FT_BitmapGlyph)ft_outline;

			uint32_t width = ft_outline_bmp->bitmap.width;

			const GlyphBitmapInfo glyph_info = {
				&ft_glyph_bmp->bitmap,
				// offset the left bearing, so that the outline's is always 0.
				ft_glyph_bmp->left - ft_outline_bmp->left,
				ft_glyph_bmp->top,
				width
			}, outline_info = {
				&ft_outline_bmp->bitmap,
				0,
				ft_outline_bmp->top,
				width
			};

			// render to the texture atlases.
			render_glyph(outline_info, outline_tex);
			render_glyph(glyph_info, glyph_tex);
			
			// store info required for rendering text using this font.
			glyph.bearing_x = ft_glyph_bmp->left; 
			glyph.advance = (ft_glyph->advance.x >> 16);
			glyph.width = width;
			glyph.x = std::min(outline_tex.pen_x, glyph_tex.pen_x);
			glyph.y = outline_tex.pen_y - outline_tex.line_height;

			// move the pen position on the texture atlases, 1px extra to prevent bleeding.
			glyph_tex.pen_x   += 1 + width;
			outline_tex.pen_x += 1 + width;

			FT_Done_Glyph(ft_glyph);
			FT_Done_Glyph(ft_outline);

			FT_Stroker_Rewind(ft_stroker);
		}
		
		glyph_info.push_back(glyph);
	}

	FT_Stroker_Done(ft_stroker);

	// combine the 8bit glyph + outline textures into a single 16bit one.
	size_t sz = glyph_tex.w * glyph_tex.h,
		   combined_w = glyph_tex.w,
		   combined_h = next_pow_of_2(glyph_tex.h);

	uint8_t* combined = new uint8_t[combined_w * combined_h * 2]();

	for(size_t i = 0; i < sz * 2; ++i){
		combined[i] = (i % 2) ? outline_tex.mem[i/2] : glyph_tex.mem[i/2];
	}
	
	// figure out the apropriate opengl texture format for a two channel texture.
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
		atlas.setSwizzle({{ GL_RED, GL_RED, GL_RED, GL_GREEN }});
	}

	free(glyph_tex.mem);
	free(outline_tex.mem);
	delete [] combined;
	
	return true;
}


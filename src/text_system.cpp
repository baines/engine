#include "text_system.h"
#include "font.h"

struct TextVert {
	int16_t x, y;
	uint16_t tex_x, tex_y;
};

TextSystem::TextSystem(Engine& e)
: ft_lib(nullptr)
, v_state()
, text_buffer("a_pos:2s|a_tex:2SN", 512)
, text_vs(e, { "text.glslv" })
, text_fs(e, { "text.glslf" })
, text_shader(*text_vs, *text_fs)
, blend_mode({ GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA }) {
	assert(FT_Init_FreeType(&ft_lib) == 0);
	text_shader.link();
	v_state.setVertexBuffers({ &text_buffer });
}

FT_Library& TextSystem::getLib(){
	return ft_lib;
}

Renderable TextSystem::addText(const Font& f, const std::string& str, size_t max_len){

	size_t str_len = std::min(str.size(), max_len);
	uint16_t utf_lo = 0, utf_hi = 0;
	std::tie(utf_lo, utf_hi) = f.getUTFRange();
	
	int off = text_buffer.getSize();
	int16_t w = 0, h = f.getLineHeight();
	
	int tw, th;
	std::tie(tw, th) = f.getTexture()->getSize();
	float x_scale = USHRT_MAX / (float)tw, y_scale = USHRT_MAX / (float)th;
	
	for(size_t i = 0; i < str_len; ++i){
		//TODO: SDL_iconv to UTF-32
		int32_t letter = std::max(0, (str[i] + 1) - utf_lo);
		if(letter >= utf_hi) letter = 0;
		
		const Font::GlyphInfo* ginfo = f.getGlyphInfo(letter);
		assert(ginfo);
				
		DEBUGF("[TEXT] '%c' -> [%d], x: %d, y: %d, w: %d",
			str[i], letter, ginfo->x, ginfo->y, ginfo->width);
						
		uint16_t tx0 = ginfo->x * x_scale,
		         ty0 = ginfo->y * y_scale,
		         tx1 = (ginfo->x + ginfo->width) * x_scale,
		         ty1 = (ginfo->y + h) * y_scale;
		
		text_buffer.push(TextVert{ w, 0, tx0, ty0 });
		text_buffer.push(TextVert{ w, h, tx0, ty1 });
		
		w += ginfo->width;
		
		text_buffer.push(TextVert{ w, 0, tx1, ty0 });
		text_buffer.push(TextVert{ w, h, tx1, ty1 });
	}
	
	for(size_t i = str_len; i < max_len; ++i){
		/*TODO: fill up to max_len uninialized to allow string to be expanded
				without needing to orphan the vertex buffer */
	}
	
	const GLsizei count = 4 * str_len;
	return Renderable(&v_state, &text_shader, blend_mode, RType{GL_TRIANGLE_STRIP}, RCount{count}, ROff{off});
}

void TextSystem::delText(Renderable& r){
	//TODO
}

TextSystem::~TextSystem(){
	FT_Done_FreeType(ft_lib);
}


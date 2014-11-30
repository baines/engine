#include "text_system.h"
#include "font.h"

struct TextVert {
	uint16_t x, y;
	float tex[2];
	float width;
};

TextSystem::TextSystem(Engine& e)
: ft_lib(nullptr)
, v_state()
, text_buffer("a_pos:2S|a_tex:2f|a_width:1f", 512)
, text_vs(e, { "text.glslv" })
, text_fs(e, { "text.glslf" })
, text_shader(*text_vs, *text_fs) {
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
	uint16_t w = 0, h = f.getLineHeight();
	
	for(size_t i = 0; i < str_len; ++i){
		//TODO: SDL_iconv to UTF-32
		int32_t letter = std::max(0, (str[i] + 1) - utf_lo);
		if(letter >= utf_hi) letter = 0;
		
		const Font::GlyphInfo* ginfo = f.getGlyphInfo(letter);
		assert(ginfo);
		
		w += ginfo->width;
		
		DEBUGF("[TEXT] '%c' -> [%d], x: %d, y: %d, w: %d",
			str[i], letter, ginfo->x, ginfo->y, ginfo->width);
		
		if(i == 0){
			text_buffer.push(TextVert{ 0, 0, {}});
			text_buffer.push(TextVert{ 0, h, {}});
		}
		
		int tw, th;
		std::tie(tw, th) = f.getTexture()->getSize();
		
		float tx = ginfo->x / (float)tw, ty = ginfo->y / (float)th;
		float fw = ginfo->width / (float)tw;
		
		text_buffer.push(TextVert{ w, 0, { tx, ty }, fw });
		text_buffer.push(TextVert{ w, h, { tx, ty }, fw });
	}
	
	for(size_t i = str_len; i < max_len; ++i){
		/*TODO: fill up to max_len uninialized to allow string to be expanded
				without needing to orphan the vertex buffer */
	}
	
	const GLsizei count = 2 + str_len * 2;
	return Renderable(&v_state, &text_shader, RType{GL_TRIANGLE_STRIP}, RCount{count}, ROff{off});
}

void TextSystem::delText(Renderable& r){
	//TODO
}

TextSystem::~TextSystem(){
	FT_Done_FreeType(ft_lib);
}


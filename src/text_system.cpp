#include "text_system.h"
#include "font.h"

struct TextVert {
	uint16_t x, y;
	uint8_t tex[4];
};

TextSystem::TextSystem(Engine& e)
: ft_lib(nullptr)
, v_state()
, text_buffer("a_pos:2S|a_tex:4B", 512)
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
		int32_t letter = std::max(0, str[i] - utf_lo);
		if(letter >= utf_hi) letter = 0;
		
		const Font::GlyphInfo* ginfo = f.getGlyphInfo(letter);
		assert(ginfo);
		
		w += ginfo->width;
		
		if(i == 0){
			text_buffer.push(TextVert{ 0, 0, {}});
			text_buffer.push(TextVert{ 0, h, {}});
		}
		
		uint8_t x0 = ginfo->x / 256, x1 = ginfo->x % 256, row = ginfo->y / h;
		
		text_buffer.push(TextVert{ w, 0, { x0, x1, row, (uint8_t)ginfo->width }});
		text_buffer.push(TextVert{ w, h, { x0, x1, row, (uint8_t)ginfo->width }});
	}
	
	for(size_t i = str_len; i < max_len; ++i){
		/*TODO: fill up to max_len uninialized to allow string to be expanded
				without needing to orphan the vertex buffer */
	}
	
	const GLsizei count = str_len + 2;
	return Renderable(&v_state, &text_shader, RType{GL_TRIANGLE_STRIP}, RCount{count}, ROff{off});
}

void TextSystem::delText(Renderable& r){
	//TODO
}

TextSystem::~TextSystem(){
	FT_Done_FreeType(ft_lib);
}


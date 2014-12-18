#include "text_system.h"
#include "font.h"
#include <algorithm>

struct TextVert {
	TextVert(int16_t x, int16_t y, uint16_t tx, uint16_t ty) : x(x), y(y), tex_x(tx), tex_y(ty){}
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

Renderable* TextSystem::addText(const Font& f, glm::ivec2 pos, const string_view& str){
	const size_t str_len = str.size();
	
	uint16_t utf_lo = 0, utf_hi = 0;
	std::tie(utf_lo, utf_hi) = f.getUTFRange();
	
	int off = text_buffer.getSize() / sizeof(TextVert);
	int16_t x = pos.x, y = pos.y, w = 0, h = f.getLineHeight();
	
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
		
		text_buffer.push(TextVert(x + w, y + 0, tx0, ty0));
		text_buffer.push(TextVert(x + w, y + h, tx0, ty1));
		
		w += ginfo->width;
		
		text_buffer.push(TextVert(x + w, y + 0, tx1, ty0));
		text_buffer.push(TextVert(x + w, y + h, tx1, ty1));
	}
		
	const GLsizei count = 4 * str_len;
	text_renderables.push_back(
		Renderable(
			&v_state,
			&text_shader, 
			blend_mode, 
			RType{GL_TRIANGLE_STRIP}, 
			RCount{count}, 
			ROff{off}
		)
	);
	return &text_renderables.back();
}

bool TextSystem::updateText(Renderable*& r, const Font& f, glm::ivec2 pos, const string_view& newstr){
	if(!r) return false;
	//TODO: optimization if newstr is substring of old str: only adjust r->off / r->count.
	//    + optimization if renderable is last in buffer + appending text

	delText(r);
	r = addText(f, pos, newstr);
	return true;
}

void TextSystem::delText(Renderable* r){
	if(!r) return;

	size_t start = r->offset * sizeof(TextVert);
	size_t count = r->count * sizeof(TextVert);
	text_buffer.invalidate(BufferRange{ start, count, this });

	auto i = text_renderables.begin();
	for(auto j = text_renderables.end(); i != j; ++i){
		if(&(*i) == r){
			break;
		}
	}
	if(i == text_renderables.end()){
		return;
	} else {
		text_renderables.erase(i);
	}
}

void TextSystem::onBufferRangeInvalidated(size_t off, size_t len){
	for(auto& r : text_renderables){
		if(r.offset * sizeof(TextVert) > off){
			r.offset -= (len / sizeof(TextVert));
		}
	}
}

TextSystem::~TextSystem(){
	FT_Done_FreeType(ft_lib);
}


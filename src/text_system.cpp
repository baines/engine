#include "text_system.h"
#include "font.h"
#include "text.h"
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

GLsizei TextSystem::writeString(Text& t, glm::ivec2 pos, const string_view& str){
	const Font& f = *t.font;
	size_t str_len = str.size(), x = pos.x, y = pos.y, w = 0, h = f.getLineHeight();
	
	uint16_t utf_lo = 0, utf_hi = 0;
	std::tie(utf_lo, utf_hi) = f.getUTFRange();
	
	int tw = 0, th = 0;
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

	t.total_width += w;

	return str_len * 4;
}

void TextSystem::addText(Text& t){
	const GLint off = text_buffer.getSize() / sizeof(TextVert);
	const GLsizei count = writeString(t, t.pos, t.str);

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
	
	t.setRenderable(&text_renderables.back());
}

bool TextSystem::updateText(Text& t, const string_view& newstr, glm::ivec2 newpos){
	if(!t.renderable) return false;

	bool pos_changed = t.pos != newpos;
	// if this is only appending text, and it's at the end of the vertex buffer,
	// then we can just push the new characters on the end.
	if(!pos_changed
	&& newstr.size() >= t.str.size()
	&& (t.renderable->offset + t.renderable->count) * sizeof(TextVert) == text_buffer.getSize()
	&& newstr.find(t.str) == 0){

		auto suffix = string_view(newstr.data() + t.str.size(), newstr.size() - t.str.size());
		t.renderable->count += writeString(
			t,
			glm::ivec2(t.pos.x + t.total_width, t.pos.y),
			suffix
		);
		t.str.append(suffix.data(), suffix.size());

	} else {
		// if the new text is a substring of the old text then don't add to the vertex buffer,
		// just adjust the renderable's count and offset and invalidate the old bits.
		auto i = t.str.find(newstr.data(), 0, newstr.size());
		if(i != std::string::npos && !pos_changed){
			size_t start = t.renderable->offset * sizeof(TextVert),
			       count = t.renderable->count * sizeof(TextVert),
			       v_diff = (t.str.size() - newstr.size()) * 4,
			       diff = v_diff * sizeof(TextVert);

			BufferRange r_lo = { start, i * sizeof(TextVert) * 4, this },
						r_hi = { (start + count) - diff, diff, this };

			if(r_hi.len > 0) text_buffer.invalidate(std::move(r_hi));
			if(r_lo.len > 0) text_buffer.invalidate(std::move(r_lo));

			t.renderable->offset += i;
			t.renderable->count  -= v_diff;

			t.str = std::move(newstr.to_string());
		} else {
			// otherwise we'll have to invalidate all the old vertices and append new ones.
			delText(t);
			t.str = std::move(newstr.to_string());
			t.pos = newpos;
			addText(t);
		}
	}
	return true;
}

void TextSystem::delText(Text& t){
	auto* r = t.renderable;

	if(!r) return;

	size_t start = r->offset * sizeof(TextVert);
	size_t count = r->count * sizeof(TextVert);
	if(count > 0){
		text_buffer.invalidate(BufferRange{ start, count, this });
	}

	auto i = text_renderables.begin();
	for(auto j = text_renderables.end(); i != j; ++i){
		if(&(*i) == r){
			break;
		}
	}
	assert(i != text_renderables.end());
	text_renderables.erase(i);
	t.total_width = 0;
	t.renderable = nullptr;
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


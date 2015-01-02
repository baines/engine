#include "text_system.h"
#include "font.h"
#include "text.h"
#include <algorithm>

struct TextVert {
	TextVert(int16_t x, int16_t y, uint16_t tx, uint16_t ty)
	: x(x), y(y), tex_x(tx), tex_y(ty){}

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

GLsizei TextSystem::writeString(Text& t, glm::ivec2 pos, const u32string_view& str){
	const Font& f = *t.font;

	size_t str_len = str.size(),
		   x = pos.x,
		   y = pos.y,
		   w = 0,
		   h = f.getLineHeight();
	
	int tw = 0, th = 0;
	std::tie(tw, th) = f.getTexture()->getSize();

	float x_scale = USHRT_MAX / (float)tw,
	      y_scale = USHRT_MAX / (float)th;

	for(size_t i = 0; i < str_len; ++i){
		
		if(str[i] == '\n'){
			text_buffer.push(TextVert(x + w, y + h, 0, 0));
			w = 0;
			y += h;
			text_buffer.push(TextVert(x + w, y + 0, 0, 0));

			//XXX: two unnecessary vertices, but adding them means num_verts == num_chars * 4.
			text_buffer.push(TextVert(x + w, y + 0, 0, 0));
			text_buffer.push(TextVert(x + w, y + 0, 0, 0));
			continue;
		}

		const Font::GlyphInfo& ginfo = f.getGlyphInfo(str[i]);

		DEBUGF("TEXT: %c -> [x: %d, y: %d, w: %d]", str[i], ginfo.x, ginfo.y, ginfo.width);	

		uint16_t tx0 = ginfo.x * x_scale,
		         ty0 = ginfo.y * y_scale,
		         tx1 = (ginfo.x + ginfo.width) * x_scale,
		         ty1 = (ginfo.y + h) * y_scale;
		
		text_buffer.push(TextVert(x + w, y + 0, tx0, ty0));
		text_buffer.push(TextVert(x + w, y + h, tx0, ty1));
		
		w += ginfo.width;
		
		text_buffer.push(TextVert(x + w, y + 0, tx1, ty0));
		text_buffer.push(TextVert(x + w, y + h, tx1, ty1));
	}

	t.end_pos = glm::ivec2(x + w, y);

	return str_len * 4;
}

void TextSystem::addText(Text& t){
	const GLint off = text_buffer.getSize() / sizeof(TextVert);
	const GLsizei count = writeString(t, t.start_pos, t.str);

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

bool TextSystem::updateText(Text& t, const u32string_view& newstr, glm::ivec2 newpos){
	if(!t.renderable) return false;

	bool pos_changed = t.start_pos != newpos;
	// if this is only appending text, and it's at the end of the vertex buffer,
	// then we can just push the new characters on the end.
	if(!pos_changed
	&& newstr.size() >= t.str.size()
	&& (t.renderable->offset + t.renderable->count) * sizeof(TextVert) == text_buffer.getSize()
	&& newstr.find(t.str) == 0){

		auto suffix = u32string_view(newstr.data() + t.str.size(), newstr.size() - t.str.size());
		t.renderable->count += writeString(
			t,
			t.end_pos,
			suffix
		);
		t.str.append(suffix.data(), suffix.size());

	} else {
		// if the new text is a substring of the old text starting at offset 0, then don't 
		// add to the vertex buffer, just lower the renderable's count and invalidate the end.
		auto i = t.str.find(newstr.data(), 0, newstr.size());
		if(i == 0 && !pos_changed){
			size_t start = t.renderable->offset * sizeof(TextVert),
			       count = t.renderable->count * sizeof(TextVert),
			       v_diff = (t.str.size() - newstr.size()) * 4,
			       diff = v_diff * sizeof(TextVert);

			text_buffer.invalidate(BufferRange{ (start + count) - diff, diff, this });

			t.renderable->count -= v_diff;
	
			t.end_pos = t.start_pos;
			for(const char32_t* c = newstr.data(); *c; ++c){
				if(*c == '\n'){
					t.end_pos.x = t.start_pos.x;
					t.end_pos.y += t.font->getLineHeight();
				} else {
					t.end_pos.x += t.font->getGlyphInfo(*c).width;
				}
			}

			t.str = std::move(newstr.to_string());
		} else {
			// otherwise we'll have to invalidate all the old vertices and append new ones.
			delText(t);
			t.str = std::move(newstr.to_string());
			t.start_pos = newpos;
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


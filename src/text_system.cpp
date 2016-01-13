#include "text_system_private.h"
#include "font.h"
#include "text.h"
#include "renderable.h"

namespace {

struct TextVert {
	TextVert(int16_t x, int16_t y, uint16_t tx, uint16_t ty, uint32_t c = 0xffffffff)
	: x(x), y(y), tex_x(tx), tex_y(ty) {
		color[0] = c >> 24;
		color[1] = c >> 16;
		color[2] = c >> 8;
		color[3] = c;
	}

	int16_t x, y;
	uint16_t tex_x, tex_y;
	uint8_t color[4];
};

static char32_t COLORCODE_START = 0xfdd0;
static size_t COLORCODE_COUNT = 16;

bool is_color_code(char32_t c){
	return c >= COLORCODE_START && c < COLORCODE_START + COLORCODE_COUNT;
}

bool get_color_code(char32_t c, const std::array<uint32_t, 16>& pal, uint32_t* out = nullptr){
	if(!is_color_code(c)) return false;

	if(out) *out = pal[c - COLORCODE_START];
	return true;
}

size_t count_verts(const StrRef32& str){
	size_t result = 0;

	for(size_t i = 0; i < str.size(); ++i){
		if(is_color_code(str[i]) || str[i] == '\n'){
			continue;
		} else {
			result += 6;
		}
	}

	return result;
}

}

TextSystem::TextSystem(Engine& e)
: ft_lib(nullptr)
, v_state()
, text_buffer("a_pos:2s|a_tex:2SN|a_col:4BN", 512)
, text_vs(e, { "text.glslv" })
, text_fs(e, { "text.glslf" })
, text_shader(text_vs, text_fs)
, blend_mode{{{ GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA }}} {
	assert(FT_Init_FreeType(&ft_lib) == 0);
	text_shader.link();
	v_state.setVertexBuffers({ &text_buffer });
}

FT_Library& TextSystem::getLib(){
	return ft_lib;
}

size_t TextSystem::writeString(Text& t, glm::ivec2 pos, const StrRef32& str){
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

	uint32_t current_color = t.palette[COLORCODE_COUNT - 1]; // white in default palette
	size_t num_verts = 0;
	char32_t prev_char = 0;
	
	for(size_t i = 0; i < str_len; ++i){

		if(get_color_code(str[i], t.palette, &current_color)){
			continue;
		}
		
		if(str[i] == '\n'){
			w = 0;
			y += h;
			continue;
		}

		const Font::GlyphInfo& ginfo = f.getGlyphInfo(str[i]);

		TRACEF(
			"TEXT: %c -> [%3d:%3d, w: %3d, b: %3d a: %3d]",
			str[i],
			ginfo.x,
			ginfo.y,
			ginfo.width,
			ginfo.bearing_x,
			ginfo.advance
		);

		if(prev_char){
			auto kern = f.getKerning(prev_char, str[i]);
			w += kern.x;

			if(kern.x != 0) TRACEF("KERN: %c + %c = %d", prev_char, str[i], kern.x);
		}

		w += ginfo.bearing_x;

		uint16_t tx0 = ginfo.x * x_scale,
		         ty0 = ginfo.y * y_scale,
		         tx1 = (ginfo.x + ginfo.width) * x_scale,
		         ty1 = (ginfo.y + h) * y_scale,
				 w2  = w + ginfo.width;
		
		text_buffer.push(TextVert(x + w , y + 0, tx0, ty0, current_color));
		text_buffer.push(TextVert(x + w , y + h, tx0, ty1, current_color));
		text_buffer.push(TextVert(x + w2, y + 0, tx1, ty0, current_color));
		text_buffer.push(TextVert(x + w2, y + 0, tx1, ty0, current_color));
		text_buffer.push(TextVert(x + w , y + h, tx0, ty1, current_color));
		text_buffer.push(TextVert(x + w2, y + h, tx1, ty1, current_color));

		num_verts += 6;
		w += ginfo.advance - ginfo.bearing_x;
		prev_char = str[i];
	}

	t.end_pos = glm::ivec2(x + w, y);

	return num_verts;
}

void TextSystem::addText(Text& t){
	const GLint off = text_buffer.getSize();
	const GLsizei count = writeString(t, t.start_pos, t.str);

	text_renderables.push_back(
		Renderable(
			&v_state,
			&text_shader, 
			blend_mode, 
			RType{GL_TRIANGLES}, 
			RCount{count}, 
			ROff{off}
		)
	);
	
	t.setRenderable(&text_renderables.back());
}

void TextSystem::updateText(Text& t, const StrRef32& newstr, int x, int y){
	if(!t.renderable) return;
	
	glm::ivec2 newpos = { x, y };

	bool pos_changed = t.start_pos != newpos;
	// if this is only appending text, and it's at the end of the vertex buffer,
	// then we can just push the new characters on the end.
	if(!pos_changed
	&& newstr.size() >= t.str.size()
	&& size_t(t.renderable->offset + t.renderable->count) == text_buffer.getSize()
	&& newstr.find(t.str) == 0){

		StrRef32 suffix(newstr.data() + t.str.size(), newstr.size() - t.str.size());
		
		t.renderable->count += writeString(
			t,
			t.end_pos,
			suffix
		);
		t.str.append(suffix);

	} else {
		// if the new text is a substring of the old text starting at offset 0, then don't 
		// add to the vertex buffer, just lower the renderable's count and invalidate the end.
		if(!t.str.empty() && t.str.find(newstr) == 0 && !pos_changed){
			size_t start = t.renderable->offset * sizeof(TextVert),
			       count = t.renderable->count * sizeof(TextVert),
			       verts = count_verts(newstr),
				   bytes = verts * sizeof(TextVert);

			text_buffer.invalidate(BufferRange{ start + bytes, count - bytes, this });

			t.renderable->count = verts;
			t.str = StrMut32(newstr);
			t.end_pos = t.getPos(newstr.size());

		} else {
			// otherwise we'll have to invalidate all the old vertices and append new ones.
			delText(t);
			t.str = StrMut32(newstr);
			t.start_pos = newpos;
			addText(t);
		}
	}
}

void TextSystem::delText(Text& t){
	Renderable* r = t.renderable;

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


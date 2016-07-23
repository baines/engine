#include "text_system_private.h"
#include "font.h"
#include "text.h"
#include "renderer.h"
#include "renderable.h"
#include <assert.h>
#include <ft2build.h>
#include FT_FREETYPE_H

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

static constexpr char32_t COLORCODE_START = 0xfdd0;
static constexpr size_t COLORCODE_COUNT = 16;

bool is_color_code(char32_t c){
	return c >= COLORCODE_START && c < COLORCODE_START + COLORCODE_COUNT;
}

bool get_color_code(char32_t c, const Array<uint32_t, 16>& pal, uint32_t* out = nullptr){
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

size_t TextSystem::writeString(Text& t, vec2i pos, const StrRef32& str){
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

	t.end_pos = vec2i { int(x + w), int(y) };

	return num_verts;
}

void TextSystem::addText(Text& t){
	const GLint off = text_buffer.getSize();
	const GLsizei count = writeString(t, t.start_pos, t.str);

	t.renderable.vertex_state =	&v_state;
	t.renderable.shader = &text_shader;
	t.renderable.blend_mode = blend_mode; 
	t.renderable.prim_type = GL_TRIANGLES;
	t.renderable.count = count; 
	t.renderable.offset = off;

	texts.push_back(&t);
}

void TextSystem::updateText(Text& t, const StrRef32& newstr, int x, int y){
	
	vec2i newpos = { x, y };

	bool pos_changed = t.start_pos != newpos;
	// if this is only appending text, and it's at the end of the vertex buffer,
	// then we can just push the new characters on the end.
	if(!pos_changed
	&& newstr.size() >= t.str.size()
	&& size_t(t.renderable.offset + t.renderable.count) == text_buffer.getSize()
	&& newstr.find(t.str) == 0){

		StrRef32 suffix(newstr.data() + t.str.size(), newstr.size() - t.str.size());
		
		t.renderable.count += writeString(
			t,
			t.end_pos,
			suffix
		);
		t.str.append(suffix);

	} else {
		// if the new text is a substring of the old text starting at offset 0, then don't 
		// add to the vertex buffer, just lower the renderable's count and invalidate the end.
		if(!t.str.empty() && t.str.find(newstr) == 0 && !pos_changed){
			size_t start = t.renderable.offset * sizeof(TextVert),
			       count = t.renderable.count * sizeof(TextVert),
			       verts = count_verts(newstr),
				   bytes = verts * sizeof(TextVert);

			text_buffer.invalidate(BufferRange{ start + bytes, count - bytes, this });

			t.renderable.count = verts;
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
	Renderable& r = t.renderable;

	size_t start = r.offset * sizeof(TextVert);
	size_t count = r.count * sizeof(TextVert);

	if(count > 0){
		text_buffer.invalidate(BufferRange{ start, count, this });
	}

	for(size_t i = 0; i < texts.size(); ++i){
		if(texts[i] == &t){
			texts.erase(texts.begin() + i);
			break;
		}
	}
	
	memset(&t.renderable, 0, sizeof(Renderable));
}

void TextSystem::onBufferRangeInvalidated(size_t off, size_t len){
	for(auto* t : texts){
		Renderable& r = t->renderable;
		if(r.offset * sizeof(TextVert) > off){
			r.offset -= (len / sizeof(TextVert));
		}
	}
}

TextSystem::~TextSystem(){
	FT_Done_FreeType(ft_lib);
}

/* Text */

static const Array<uint32_t, COLORCODE_COUNT> default_palette = {{
	0x000000ff,
	0xa90707ff,
	0x05801fff,
	0x8c8e0bff,
	0x2c2fabff,
	0x960f8eff,
	0x0d5ca0ff,
	0x8d959bff,
	0x414447ff,
	0xdc2c2cff,
	0x28ba1cff,
	0xece63fff,
	0x5053c8ff,
	0xc450c8ff,
	0x50c8c1ff,
	0xffffffff
}};

Text::Text()
: engine()
, font()
, palette(default_palette)
, start_pos()
, end_pos()
, str()
, uniforms()
, renderable() {
	uniforms.setUniform("u_samp", { 0 });
	uniforms.setUniform("u_outline_col", { vec4 { 0.0f, 0.0f, 0.0f, 1.0f } });
}

Text::Text(Engine& e, Proxy<Font> f, vec2i pos, const StrRef& s)
: engine(&e)
, font(f)
, palette(default_palette)
, start_pos(pos)
, end_pos()
, str(to_utf32(s))
, uniforms()
, renderable() {

	e.text->addText(*this);

	uniforms.setUniform("u_samp", { 0 });
	uniforms.setUniform("u_outline_col", { vec4 { 0.0f, 0.0f, 0.0f, 1.0f } });
}

void Text::setPalette(const Array<uint32_t, 16>& colors){
	palette = colors;
}

void Text::resetPalette(){
	palette = default_palette;
}

Array<uint32_t, 16> Text::getDefaultPalette(){
	return default_palette;
}

int Text::update(const StrRef& newstr){
	return update(newstr, start_pos); 
}

int Text::update(const StrRef& newstr, vec2i newpos){
	if(!engine || !font) return 0;

	StrMut32 u32str = to_utf32(newstr);

	//XXX: tab hack
	for(size_t i = 0; i < u32str.size(); ++i){
		if(u32str[i] == '\t') u32str.replace(i, 1, U"    ");
	}

	if(u32str == str && newpos == start_pos){
		return 0;
	} else {
		int char_count_diff = u32str.size() - str.size();
		engine->text->updateText(*this, u32str, newpos.x, newpos.y);
		return char_count_diff;
	}
}

void Text::draw(IRenderer& r){
	if(renderable.count){
		renderable.uniforms = &uniforms;
		renderable.textures[0] = font->getTexture();
		r.addRenderable(renderable);
	}
}

void Text::setOutlineColor(uint32_t col){
	auto v = vec4 {
		uint8_t(col >> 24u) / 255.0f,
		uint8_t(col >> 16u) / 255.0f,
		uint8_t(col >> 8u ) / 255.0f,
		uint8_t(col)        / 255.0f
	};
	uniforms.setUniform("u_outline_col", { v });
}

Text::~Text(){
	if(engine) engine->text->delText(*this);
}

vec2i Text::getPos(size_t index) const {
	vec2i result = start_pos;
	index = std::min(index, str.size());

	for(size_t i = 0; i < index; ++i){
		char32_t c = str[i], prev = 0;
		if(is_color_code(c)) continue;

		if(c == '\n'){
			result.x = start_pos.x;
			result.y += font->getLineHeight();
		} else {
			result.x += font->getGlyphInfo(c).advance;
			if(prev) result.x += font->getKerning(prev, c).x;
			prev = c;
		}
	}

	return result;
}



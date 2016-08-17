#include "gui.h"
#include "blend_mode.h"
#include "renderable.h"
#include "renderer.h"
#include "font.h"
#include "input.h"
#include <cmath>
#include <climits>

#define NK_IMPLEMENTATION
#include "nuklear.h"

#define MAX_VERT_SIZE (1024 * 512)
#define MAX_INDX_SIZE (1024 * 128)

static const struct nk_color col_darker  = nk_rgb(0x0C, 0x08, 0x18);
static const struct nk_color col_dark    = nk_rgb(0x13, 0x10, 0x20);
static const struct nk_color col_base    = nk_rgb(0x1b, 0x18, 0x25);
static const struct nk_color col_light   = nk_rgb(0x31, 0x2E, 0x3A);
//static const struct nk_color col_lighter = nk_rgb(0x54, 0x4F, 0x64);
static const struct nk_color col_lighter = nk_rgb(0x92, 0x8E, 0x9E);

static const struct nk_color col_white   = nk_rgba_u32(0xffffffff);

static const struct nk_color gui_style[NK_COLOR_COUNT] = {
	col_white   , // text
	col_darker  , // window
	col_light   , // header
	col_light   , // border
	col_base    , // button
	col_light   , // button hover
	col_lighter , // button active
	col_light   , // toggle
	col_lighter , // toggle hover
	col_white   , // toggle cursor
	col_base    , // select
	col_light   , // select active
	col_base    , // slider
	col_white   , // slider cursor
	col_light   , // slider hover
	col_lighter , // slider active
	col_base    , // property
	col_base    , // edit
	col_white   , // edit cursor
	col_base    , // combo
	col_base    , // chart
	col_white   , // chart color
	col_lighter , // chart color highlight
	col_light   , // scrollbar
	col_white   , // scrollbar cursor
	col_light   , // scrollbar hover
	col_lighter , // scrollbar active
	col_base    , // tab header
};

enum {
	GUI_BUTTON_LEFT = NK_KEY_MAX,
	GUI_BUTTON_RIGHT,
	GUI_BUTTON_MIDDLE,
	GUI_BUTTON_MAX,

	GUI_SCROLL_UP   = GUI_BUTTON_MAX,
	GUI_SCROLL_DOWN,
	GUI_SCROLL_MAX,

	GUI_CURSOR_X    = GUI_SCROLL_MAX,
	GUI_CURSOR_Y,

	GUI_INPUT_MAX,
};

static float gui_font_width_fn(nk_handle handle, float h, const char* text, int len){
	Font* font = (Font*)handle.ptr;

	float result = 0.0f;
	for(char32_t c : utf8_iterate(StrRef(text, len))){
		const Font::GlyphInfo& glyph = font->getGlyphInfo(c);
		result += glyph.advance;
	}

	return result;
}

static void gui_font_glyph_fn(nk_handle handle, float h, struct nk_user_font_glyph* out, nk_rune cp, nk_rune next_cp){
	Font* font = (Font*)handle.ptr;
	const Font::GlyphInfo& glyph = font->getGlyphInfo(cp);

	int _tw, _th;
	std::tie(_tw, _th) = font->getTexture()->getSize();
	float tw = (float)_tw, th = (float)_th;

	out->uv[0]    = nk_vec2(glyph.x / tw, glyph.y / th);
	out->uv[1]    = nk_vec2((glyph.x + glyph.width) / tw, (glyph.y + h) / th);
	out->offset   = nk_vec2(glyph.bearing_x, 0);
	out->width    = glyph.width;
	out->height   = h;
	out->xadvance = glyph.advance + font->getKerning(cp, next_cp).x;
}

GUI::GUI(Engine& e)
: ctx         (nullptr)
, nk_font     ()
, verts       ("a_pos:2f|a_tex:2f|a_col:4BN", MAX_VERT_SIZE)
, indices     ()
, state       ({ &verts }, &indices)
, vs          (e, {"gui.glslv"})
, fs          (e, {"gui.glslf"})
, shader      (vs, fs)
, font        (e, {"DejaVuSansMono.ttf"}, 16)
, cursor      ({ 0, 0 })
, input_id    (-1)
, dirty       (false)
, renderables () {

	renderables.reserve(100);

	nk_font.userdata = nk_handle_ptr((void*)font.getRawPtr());
	nk_font.height = 16.0f;
	nk_font.width = &gui_font_width_fn;
	nk_font.query = &gui_font_glyph_fn;
	nk_font.texture = nk_handle_ptr((void*)font->getTexture());

	ctx = new nk_context();
	nk_init_default(ctx, &nk_font);
	
	nk_style_from_table(ctx, gui_style);

	ctx->style.button.rounding = 0;
	ctx->style.window.rounding = 0;
	ctx->style.property.rounding = 0;
	ctx->style.slider.rounding = 0;
	ctx->style.slider.cursor_size = nk_vec2(10, 10);

	ctx->style.window.border = 1;
	ctx->style.window.menu_border = 1;
	ctx->style.window.background = col_base;

	ctx->style.window.header.minimize_symbol = NK_SYMBOL_UNDERSCORE;

	ctx->style.window.header.close_button.normal = nk_style_item_color(col_base);
	//	ctx->style.window.header.close_button.border = 1;
	//	ctx->style.window.header.close_button.border_color = col_white;

	ctx->style.window.header.minimize_button.normal = nk_style_item_color(col_base);
	//	ctx->style.window.header.minimize_button.border = 1;
	//	ctx->style.window.header.minimize_button.border_color = col_white;

	ctx->style.window.header.padding = nk_vec2(2, 2);
	ctx->style.window.header.label_padding = nk_vec2(1, 1);
	ctx->style.window.header.spacing = nk_vec2(2, 0);

	ctx->style.menu_button.normal = nk_style_item_color(col_base);

	shader.link();

}

GUI::~GUI(){
	nk_free(ctx);
	delete ctx;
}

int GUI::initInput(Engine& e, GameState* state, int id){

	input_id = id;

	e.input->subscribe(state, "gui_shift"      , id + NK_KEY_SHIFT);
	e.input->subscribe(state, "gui_ctrl"       , id + NK_KEY_CTRL);
	e.input->subscribe(state, "gui_delete"     , id + NK_KEY_DEL);
	e.input->subscribe(state, "gui_enter"      , id + NK_KEY_ENTER);
	e.input->subscribe(state, "gui_tab"        , id + NK_KEY_TAB);
	e.input->subscribe(state, "gui_backspace"  , id + NK_KEY_BACKSPACE);
	e.input->subscribe(state, "gui_copy"       , id + NK_KEY_COPY);
	e.input->subscribe(state, "gui_cut"        , id + NK_KEY_CUT);
	e.input->subscribe(state, "gui_paste"      , id + NK_KEY_PASTE);
	e.input->subscribe(state, "gui_up"         , id + NK_KEY_UP);
	e.input->subscribe(state, "gui_down"       , id + NK_KEY_DOWN);
	e.input->subscribe(state, "gui_left"       , id + NK_KEY_LEFT);
	e.input->subscribe(state, "gui_right"      , id + NK_KEY_RIGHT);

	e.input->subscribe(state, "gui_insert"     , id + NK_KEY_TEXT_INSERT_MODE);
	e.input->subscribe(state, "gui_replace"    , id + NK_KEY_TEXT_REPLACE_MODE);
	e.input->subscribe(state, "gui_reset"      , id + NK_KEY_TEXT_RESET_MODE);
	e.input->subscribe(state, "gui_sol"        , id + NK_KEY_TEXT_LINE_START);
	e.input->subscribe(state, "gui_eol"        , id + NK_KEY_TEXT_LINE_END);
	e.input->subscribe(state, "gui_start"      , id + NK_KEY_TEXT_START);
	e.input->subscribe(state, "gui_end"        , id + NK_KEY_TEXT_END);
	e.input->subscribe(state, "gui_undo"       , id + NK_KEY_TEXT_UNDO);
	e.input->subscribe(state, "gui_redo"       , id + NK_KEY_TEXT_REDO);
	e.input->subscribe(state, "gui_word_left"  , id + NK_KEY_TEXT_WORD_LEFT);
	e.input->subscribe(state, "gui_word_right" , id + NK_KEY_TEXT_WORD_RIGHT);

	e.input->subscribe(state, "lmb", id + GUI_BUTTON_LEFT);
	e.input->subscribe(state, "mmb", id + GUI_BUTTON_MIDDLE);
	e.input->subscribe(state, "rmb", id + GUI_BUTTON_RIGHT);

	e.input->subscribe(state, "gui_scroll_up"  , id + GUI_SCROLL_UP);
	e.input->subscribe(state, "gui_scroll_down", id + GUI_SCROLL_DOWN);

	e.input->subscribe(state, "cursor_x", id + GUI_CURSOR_X); 
	e.input->subscribe(state, "cursor_y", id + GUI_CURSOR_Y);

	return id + GUI_INPUT_MAX;
}

void GUI::onInput(int key, bool pressed){

	if(input_id == -1){
		log(logging::error, "GUI::initInput not called");
		return;
	}

	if(key >= input_id){
		if(key < input_id + NK_KEY_MAX){
			auto k = (enum nk_keys)key;
			nk_input_key(ctx, k, pressed);
		} else if(key < input_id + GUI_BUTTON_MAX){
			auto b = (enum nk_buttons)(key - NK_KEY_MAX);
			nk_input_button(ctx, b, cursor.x, cursor.y, pressed);
		} else if(key < input_id + GUI_SCROLL_MAX){
			nk_input_scroll(ctx, key == GUI_SCROLL_UP ? 1.5f : -1.5f);
		}
	}
}

void GUI::onMotion(int axis, int val){

	if(input_id == -1){
		log(logging::error, "GUI::initInput not called");
		return;
	}

	if(axis == input_id + GUI_CURSOR_X){
		cursor.x = val;
		nk_input_motion(ctx, cursor.x, cursor.y);
	} else if(axis == input_id + GUI_CURSOR_Y){
		cursor.y = val;
		nk_input_motion(ctx, cursor.x, cursor.y);
	}
}

void GUI::onText(const char* txt){
	for(char32_t c : utf8_iterate(txt)){
		nk_input_unicode(ctx, c);
	}
}

nk_context* GUI::begin(){
	nk_input_end(ctx);
	dirty = true;
	return ctx;
}

void GUI::end(){
	nk_input_begin(ctx);
}

void GUI::draw(IRenderer& r){

	if(!dirty){
		for(auto& i : renderables) r.addRenderable(i);
		return;
	}

	struct nk_convert_config cfg = {};
	cfg.global_alpha = 0.9f;
	cfg.circle_segment_count = cfg.arc_segment_count = cfg.curve_segment_count = 22;
	cfg.null.texture.ptr = (void*)font->getTexture();
	cfg.null.uv = nk_vec2(0.5f, 0.9f);

	uint8_t* vptr = verts.beginWrite(MAX_VERT_SIZE);
	uint8_t* iptr = indices.beginWrite(MAX_INDX_SIZE);

	struct nk_buffer vb, ib, cmds;
	nk_buffer_init_fixed(&vb, vptr, MAX_VERT_SIZE);
	nk_buffer_init_fixed(&ib, iptr, MAX_INDX_SIZE);

	char cmd_buf[4096];
	nk_buffer_init_fixed(&cmds, cmd_buf, sizeof(cmd_buf));

	nk_convert(ctx, &cmds, &vb, &ib, &cfg);

	verts.endWrite(vb.needed);
	indices.endWrite(ib.needed);

	renderables.clear();

	Renderable obj = {};
	obj.prim_type = GL_TRIANGLES;
	obj.shader = &shader;
	obj.vertex_state = &state;
	obj.blend_mode = BlendMode({ GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA });

	const struct nk_draw_command* cmd;

	nk_draw_foreach(cmd, ctx, &cmds){
		if(!cmd->elem_count) continue;

		obj.textures[0] = (Texture*)cmd->texture.ptr;
		obj.clip = {
			(int)cmd->clip_rect.x,
			(int)cmd->clip_rect.y,
			(int)cmd->clip_rect.w,
			(int)cmd->clip_rect.h
		};
		obj.count = cmd->elem_count;

		renderables.push_back(obj);
		r.addRenderable(renderables.back());

		obj.offset += obj.count * 2;
	}

	nk_clear(ctx);

	dirty = false;
}


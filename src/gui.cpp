#include "gui.h"
#include "blend_mode.h"
#include "renderable.h"
#include "renderer.h"
#include "font.h"
#include "input.h"
#include <cmath>
#include <climits>

#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_BUTTON_TRIGGER_ON_RELEASE
#define NK_MEMSET memset
#define NK_MEMCOPY memcpy
#define NK_SQRT sqrt
#define NK_SIN sin
#define NK_COS cos
#define NK_IMPLEMENTATION
#include "nuklear.h"

#define MAX_VERT_SIZE (1024 * 512)
#define MAX_INDX_SIZE (1024 * 128)

static struct nk_color gui_style[NK_COLOR_COUNT] = {
	{ 255, 255, 255, 255 }, // text
	{ 26 , 23 , 36 , 255 }, // window
	{ 255, 255, 255, 255 }, // header
	{ 255, 255, 255, 255 }, // border
	{ 26 , 23 , 36 , 255 }, // button
	{ 52 , 46 , 71 , 255 }, // button hover
	{ 79 , 70 , 110, 255 }, // button active
	{ 26 , 23 , 36 , 255 }, // toggle
	{ 52 , 46 , 71 , 255 }, // toggle hover
	{ 79 , 70 , 110, 255 }, // toggle cursor ??
	{ 26 , 23 , 36 , 255 }, // select
	{ 79 , 70 , 110, 255 }, // select active
	{ 26 , 23 , 36 , 255 }, // slider
	{ 26 , 23 , 36 , 255 }, // slider cursor ??
	{ 52 , 46 , 71 , 255 }, // slider hover
	{ 79 , 70 , 110, 255 }, // slider active
	{ 26 , 23 , 36 , 255 }, // property
	{ 26 , 23 , 36 , 255 }, // edit
	{ 26 , 23 , 36 , 255 }, // edit cursor ??
	{ 26 , 23 , 36 , 255 }, // combo
	{ 26 , 23 , 36 , 255 }, // chart
	{ 255, 255, 255, 255 }, // chart color
	{ 79 , 70 , 110, 255 }, // chart color highlight
	{ 255, 255, 255, 255 }, // scrollbar
	{ 255, 255, 255, 255 }, // scrollbar cursor ??
	{ 26 , 23 , 36 , 255 }, // scrollbar hover
	{ 79 , 70 , 110, 255 }, // scrollbar active
	{ 255, 255, 255, 255 }, // tab header
};

enum {
	GUI_BUTTON_LEFT = NK_KEY_MAX,
	GUI_BUTTON_MIDDLE,
	GUI_BUTTON_RIGHT,
	GUI_DIGITAL_MAX,

	GUI_CURSOR_X = GUI_DIGITAL_MAX,
	GUI_CURSOR_Y,

	GUI_INPUT_MAX,
};

static float gui_font_width_fn(nk_handle handle, float h, const char* text, int len){
	Font* font = (Font*)handle.ptr;

	float result = 0.0f;
	StrMut32 str32 = to_utf32(StrRef(text, len));
	for(const char32_t& c : str32){
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

// FIXME should be cached
static const uint32_t white_rgba = UINT32_C(0xffffffff);

GUI::GUI(Engine& e)
: ctx         (nullptr)
, verts       ("a_pos:2f|a_tex:2f|a_col:4BN", MAX_VERT_SIZE)
, indices     ()
, state       ({ &verts }, &indices)
, vs          (e, {"gui.glslv"})
, fs          (e, {"gui.glslf"})
, shader      (vs, fs)
, null_tex    (GL_UNSIGNED_BYTE, GL_RGBA8, 1, 1, &white_rgba)
, font        (e, {"DejaVuSansMono.ttf"}, 16)
, cursor      ({ 0, 0 })
, input_id    (-1)
, renderables () {

	struct nk_user_font nk_font = {
		nk_handle_ptr((void*)font.getRawPtr()),
		16.0f,
		&gui_font_width_fn,
		&gui_font_glyph_fn,
		nk_handle_ptr((void*)font->getTexture())
	};

	ctx = new nk_context();
	nk_init_default(ctx, &nk_font);

	nk_style_from_table(ctx, gui_style);

	ctx->style.button.rounding = 0;
	ctx->style.window.rounding = 0;
	ctx->style.property.rounding = 0;

	ctx->style.window.header.label_normal = { 26, 23, 36, 255 };
	ctx->style.window.header.label_hover  = { 26, 23, 36, 255 };
	ctx->style.window.header.label_active = { 26, 23, 36, 255 };
	
	ctx->style.window.header.padding = nk_vec2(0, 0);
	ctx->style.window.header.label_padding = nk_vec2(10, 0);

	shader.link();
}

GUI::~GUI(){
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

//	e.input->subscribe(state, "gui_"       , id + NK_KEY_TEXT_END);

	e.input->subscribe(state, "cursor_x", id + GUI_CURSOR_X); 
	e.input->subscribe(state, "cursor_y", id + GUI_CURSOR_Y);

	e.input->subscribe(state, "lmb", id + GUI_BUTTON_LEFT);
	e.input->subscribe(state, "mmb", id + GUI_BUTTON_MIDDLE);
	e.input->subscribe(state, "rmb", id + GUI_BUTTON_RIGHT);

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
		} else if(key < input_id + GUI_DIGITAL_MAX){
			auto b = (enum nk_buttons)(key - NK_KEY_MAX);
			nk_input_button(ctx, b, cursor.x, cursor.y, pressed);
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

void GUI::draw(IRenderer& r){

	struct nk_convert_config cfg = {};
	cfg.global_alpha = 1.0f;
	cfg.circle_segment_count = cfg.arc_segment_count = cfg.curve_segment_count = 22;
	cfg.null.texture.ptr = (void*)font->getTexture();
	cfg.null.uv = nk_vec2(0.5f, 0.9f);

	uint8_t* vptr = verts.beginWrite(MAX_VERT_SIZE);
	uint8_t* iptr = indices.beginWrite(MAX_INDX_SIZE);

	struct nk_buffer vb, ib, cmds;
	nk_buffer_init_fixed(&vb, vptr, MAX_VERT_SIZE);
	nk_buffer_init_fixed(&ib, iptr, MAX_INDX_SIZE);

	nk_buffer_init_default(&cmds);

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
}


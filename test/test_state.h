#ifndef TEST_STATE_H_
#define TEST_STATE_H_
#include "engine_all.h"
#include "gui.h"

#define NK_INCLUDE_FIXED_TYPES
#include "nuklear.h"

static const constexpr struct {
	float x, y;
	uint8_t r, g, b, a;
} vertices[3] = {
	{ 0.0f, 0.5f, 255, 0, 0, 255 },
	{ 0.5f, -0.5f, 0, 255, 0, 255 },
	{ -0.5f, -0.5f, 0, 0, 255, 255 }
};

struct TestState : public GameState {

	TestState(Engine& e)
	: timer(0)
	, tri_vbo       (vertices, "a_pos:2f|a_col:4BN")
	, tri_vs        (e, {"test.glslv"})
	, sprite_vs     (e, {"sprite.glslv"})
	, tri_fs        (e, {"test.glslf"})
	, sprite_fs     (e, {"sprite.glslf"})
	, tri_shader    (tri_vs, tri_fs)
	, sprite_shader (sprite_vs, sprite_fs)
	, tri_vstate    ()
	, font          (e, {"LiberationSans-Regular.ttf"}, 64)
	, text          (e, font, { 270, 208 }, "Testing!")
	, samp_nearest  ({{ GL_TEXTURE_MAG_FILTER, GL_NEAREST }})
	, sprite_tex    (e, {"test_sprite.png"})
	, sprite_mat    (sprite_shader, *sprite_tex, samp_nearest)
	, sprite_batch  (sprite_mat)
	, test_sprite   (sprite_batch, { 200, 200 }, { 64, 64 })
	, center        ({ 320, 240 })
	, gui           (e) {
		
		triangle.vertex_state = &tri_vstate;
		triangle.shader = &tri_shader;
		triangle.uniforms = &tri_uniforms;
		triangle.prim_type = GL_TRIANGLES;
		triangle.count = 3;

		tri_vstate.setVertexBuffers({ &tri_vbo });

		gui.initInput(e, this, 0);
		nk_input_begin(gui.ctx);
	}

	bool onInit(Engine& e){
		tri_shader.link();
		sprite_shader.link();

		return true;
	}

	bool onInput(Engine& e, int act, bool pressed){
		gui.onInput(act, pressed);
		return false;
	}

	bool onMotion(Engine& e, int act, int val, bool rel){
		gui.onMotion(act, val);
		return false;
	}

	void onResize(Engine& e, int w, int h){
		center = { w / 2, h / 2 };

		vec2i text_sz = text.getEndPos() - text.getStartPos();
		text.update(
			TXT_RED     "T"
			TXT_YELLOW  "e"
			TXT_GREEN   "s"
			TXT_BLUE    "t"
			TXT_CYAN    "i"
			TXT_MAGENTA "n"
			TXT_WHITE   "g"
			TXT_GRAY    "!",
			center - (text_sz / 2)
		);
	}

	void update(Engine& e, uint32_t delta){
		timer = (timer + delta / 2);
		float tri_timer    = sinf((timer % 628)  / 100.0f);
		float sprite_timer = sinf((timer % 1256) / 200.0f);

		tri_uniforms.setUniform("timer", { 1.0f + tri_timer });

		test_sprite.setPosition({ center.x + int(sprite_timer * 200), (center.y + 140) });
	}

	void draw(IRenderer& renderer){
		renderer.addRenderable(triangle);
		text.draw(renderer);
		sprite_batch.draw(renderer);

		nk_context* ctx = gui.ctx;

		nk_input_end(ctx);

		static nk_color background = { 255, 255, 255, 255 };
		{
			struct nk_panel layout;
			if (nk_begin(ctx, &layout, "Demo", nk_rect(50, 50, 230, 250),
					NK_WINDOW_BORDER|NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE|
					NK_WINDOW_MINIMIZABLE|NK_WINDOW_TITLE))
			{
				enum {EASY, HARD};
				static int op = EASY;
				static int property = 20;
				nk_layout_row_static(ctx, 30, 80, 1);
				if (nk_button_label(ctx, "button"))
					fprintf(stdout, "button pressed\n");

				nk_layout_row_dynamic(ctx, 30, 2);
				if (nk_option_label(ctx, "easy", op == EASY)) op = EASY;
				if (nk_option_label(ctx, "hard", op == HARD)) op = HARD;

				nk_layout_row_dynamic(ctx, 25, 1);
				nk_property_int(ctx, "Compression:", 0, &property, 100, 10, 1);

				{
					struct nk_panel combo;
					nk_layout_row_dynamic(ctx, 20, 1);
					nk_label(ctx, "background:", NK_TEXT_LEFT);
					nk_layout_row_dynamic(ctx, 25, 1);
					if (nk_combo_begin_color(ctx, &combo, background, 400)) {
						nk_layout_row_dynamic(ctx, 120, 1);
						background = nk_color_picker(ctx, background, NK_RGBA);
						nk_layout_row_dynamic(ctx, 25, 1);
						background.r = (nk_byte)nk_propertyi(ctx, "#R:", 0, background.r, 255, 1,1);
						background.g = (nk_byte)nk_propertyi(ctx, "#G:", 0, background.g, 255, 1,1);
						background.b = (nk_byte)nk_propertyi(ctx, "#B:", 0, background.b, 255, 1,1);
						background.a = (nk_byte)nk_propertyi(ctx, "#A:", 0, background.a, 255, 1,1);
						nk_combo_end(ctx);
					}
				}
			}
			nk_end(ctx);
		}

		gui.draw(renderer);
	
		nk_input_begin(ctx);
	}
private:
	unsigned int timer;
	StaticVertexBuffer tri_vbo;

	Resource<VertShader> tri_vs, sprite_vs;
	Resource<FragShader> tri_fs, sprite_fs;
	ShaderProgram tri_shader, sprite_shader;

	ShaderUniforms tri_uniforms;
	VertexState tri_vstate;
	Renderable triangle;

	Resource<Font, uint16_t> font;
	Text text;

	Sampler samp_nearest;
	Resource<Texture2D> sprite_tex;
	Material sprite_mat;
	SpriteBatch sprite_batch;
	Sprite test_sprite;

	vec2i center;

	GUI gui;
};

#endif

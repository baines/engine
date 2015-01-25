#ifndef TEST_STATE_H_
#define TEST_STATE_H_
#include "engine.h"
#include "game_state.h"
#include "resource.h"
#include "shader.h"
#include "font.h"
#include "text.h"
#include "texture.h"
#include "material.h"
#include "sprite_batch.h"
#include "sprite.h"
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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
	, tri_vbo       (make_resource(vertices), "a_pos:2f|a_col:4BN")
	, tri_vs        (e, {"test.glslv"})
	, sprite_vs     (e, {"sprite.glslv"})
	, tri_fs        (e, {"test.glslf"})
	, sprite_fs     (e, {"sprite.glslf"})
	, tri_shader    (*tri_vs, *tri_fs)
	, sprite_shader (*sprite_vs, *sprite_fs)
	, tri_vstate    ()
	, triangle      (&tri_vstate, &tri_shader, &tri_uniforms, RType{GL_TRIANGLES}, RCount{3})
	, font          (e, {"FreeSans.ttf"}, 32)
	, text          (e, *font, { 270, 208 }, "Testing!")
	, samp_nearest  ({{ GL_TEXTURE_MAG_FILTER, GL_NEAREST }})
	, sprite_tex    (e, {"test_sprite.png"})
	, sprite_mat    (sprite_shader, *(*sprite_tex), samp_nearest)
	, sprite_batch  (sprite_mat)
	, test_sprite   (sprite_batch, { 200, 200 }, { 64, 64 })
	, center        ({ 320, 240 }){
		tri_vstate.setVertexBuffers({ &tri_vbo });
	}
	
	bool onInit(Engine& e){
		tri_shader.link();
		sprite_shader.link();
		
		return true;
	}

	void onResize(Engine& e, int w, int h){
		center = { w / 2, h / 2 };

		glm::ivec2 text_sz = text.getEndPos() - text.getStartPos();
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
		test_sprite.setPosition({ center.x + sprite_timer * 200.0f, (center.y + 140.0f) });
	}
	
	void draw(Renderer& renderer){
		renderer.addRenderable(triangle);
		text.draw(renderer);
		sprite_batch.draw(renderer);
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

	glm::ivec2 center;
};

#endif

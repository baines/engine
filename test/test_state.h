#include "engine.h"
#include "game_state.h"
#include "resource.h"
#include "shader.h"
#include "font.h"
#include "text.h"
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
	, vbo(make_resource(vertices), "a_pos:2f|a_col:4BN")
	, vs(e, {"test.glslv"})
	, fs(e, {"test.glslf"})
	, shader(*vs, *fs)
	, vstate()
	, drawme(&vstate, &shader, &uniforms, RType{GL_TRIANGLES}, RCount{3})
	, font(e, {"FreeSans.ttf"}, 32)
	, text() {

	}
	
	bool onInit(Engine& e){
		vs.load();
		fs.load();
		shader.link();
		vstate.setVertexBuffers({ &vbo });
		
		text = Text(e, *font, "Testing!");
		
		return true;
	}
	
	void update(Engine& e, uint32_t delta){
		timer = (timer + delta / 2) % 628;
		uniforms.setUniform("timer", { 1.0f + sinf(timer / 100.0f) });
	}
	
	void draw(Renderer& r){
		r.addRenderable(drawme);
		text.draw(r);
	}
private:
	int timer;
	StaticVertexBuffer vbo;
	Resource<VertShader> vs;
	Resource<FragShader> fs;
	ShaderProgram shader;
	ShaderUniforms uniforms;
	VertexState vstate;
	Renderable drawme;
	
	Resource<Font, uint16_t> font;
	Text text;
};

#include "engine.h"
#include "game_state.h"
#include "resource.h"
#include "shader.h"
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
	, vs("test.vs")
	, fs("test.fs")
	, shader(*vs, *fs)
	, vstate()
	, drawme() {
		e.res.addImmediate("test.vs",
			"#version 330 core\n"
			"uniform float timer = 1.0f;\n"
			"in vec2 a_pos;\n"
			"in vec4 a_col;\n"
			"out vec3 pass_col;\n"
			"void main(){\n"
			"	gl_Position = vec4(vec2(a_pos.xy * timer), 0, 1);\n"
			"	pass_col = a_col.xyz;\n"
			"}\n"
		);
		e.res.addImmediate("test.fs",
			"#version 330 core\n"
			"in vec3 pass_col;\n"
			"out vec3 color;\n"
			"void main(){\n"
			"	color = pass_col;\n"
			"}\n"
		);
		
		drawme.vertex_state = &vstate;
		drawme.shader = &shader;
		drawme.uniforms = &uniforms;
		drawme.prim_type = GL_TRIANGLES;
		drawme.count = 3;
	}
	
	bool onInit(Engine& e){
		vs.load(e);
		fs.load(e);
		shader.link();
		vstate.setVertexBuffers({ &vbo });
		
		return true;
	}
	
	void update(Engine& e, uint32_t delta){
		timer = (timer + delta / 2) % 628;
		float f_timer = 1.0f + sin(timer / 100.0f);
		uniforms.setUniform("timer", { f_timer });
	}
	
	void draw(Renderer& r){
		r.addRenderable(drawme);
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
};
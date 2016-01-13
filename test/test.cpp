//hack for testing purposes
#define private public

#include "engine.h"
#include "config.h"
#include "shader_uniforms.h"
#include "test_state.h"
#include "test_collision_state.h"

#ifdef __EMSCRIPTEN__
#include "emscripten.h"
#endif

using namespace std;

void test_shader_uniforms(int, char**){
	ShaderUniforms su;

	//glm::vec2 v1, v2;
	//glm::ivec4 v3, v4;
	//glm::mat4 m1, m2;
	//glm::mat2x4 m3, m4;

	//su.setUniform(0, { v1, v2 });
	//su.setUniform(0, { v3, v4 });
	//su.setUniform(0, { m1, m2 });
	//su.setUniform(0, { m3, m4 });
	
	glm::mat2 m(0.0, 1.0, 2.0, 3.0);
	glm::vec2 v(4.0, 5.0);
	float f = 6.0;
	
	constexpr str_const test1("test1"), test2("test2"), test3("test3");
	
	// usually done internally by Shader
	//su.initUniform(test1.hash, 1, 0);
	//su.initUniform(test2.hash, 1, 1);
	//su.initUniform(test3.hash, 1, 2);
	
	su.setUniform("test1", { m });
	su.setUniform("test2", { v });
	su.setUniform("test3", { f });
	
	int counter = 0;
	for(auto& v : su.uniforms){
		float f = 0;
		memcpy(&f, &v, 4);
		
		assert((int)f == counter++);
		
		printf("%.2f\n", f);
	}
}

void test_engine_rendering(int argc, char** argv){
	Engine e(argc, argv, "Test");
	TestState ts(e);
	
	e.addState(&ts);
#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop_arg([](void* e){
		reinterpret_cast<Engine*>(e)->run();
	}, &e, 0, true);
#else
	while(e.run());
#endif
}

void test_engine_collision(int argc, char** argv){
	Engine e(argc, argv, "Test");
	TestCollisionState ts(e);
	
	e.addState(&ts);
	
#ifdef __EMSCRIPTEN__
	emscripten_set_main_loop_arg([](void* e){
		reinterpret_cast<Engine*>(e)->run();
	}, &e, 0, true);
#else
	while(e.run());
#endif
}

struct Test {
	const char* name;
	void (*func)(int, char**);
} tests[] = {
	{ "shader-uniforms", &test_shader_uniforms },
	{ "rendering",       &test_engine_rendering },
	{ "collision",       &test_engine_collision }
};

int main(int argc, char** argv){

	void (*chosen_test)(int, char**) = &test_engine_rendering;

	if(argc > 1){

		if(strcasecmp(argv[1], "list") == 0){
			for(auto& t : tests){
				fprintf(stderr, "%s\n", t.name);
			}
			return 1;
		}

		for(auto& t : tests){
			if(strcasecmp(argv[1], t.name) == 0){
				chosen_test = t.func;
				break;
			}
		}
	}

	chosen_test(argc, argv);
	
	return 0;
}

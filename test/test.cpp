#include "include/config.h"
#include "include/shader_uniforms.h"
#include <iostream>

using namespace std;

void testConfig(void){
	Config cfg;
	
	cfg.addVar("abcdef", CVarInt(123, 0, 999));
	cfg.addVar("a_b_c", CVarInt(0, 0, 0));
	cfg.addVar("abc_def", CVarInt(0, 0, 0));
	
	vector<CVar*> vec;
	cfg.getVarsWithPrefix("ab", vec);
	
	for(auto& v : vec){
		puts(v->name.c_str());
	}
	
	CVarInt* v = cfg.getVar<CVarInt>("abcdef");
	
	printf("%d\n", v->val);
}

void testPrefixExtend(void){
	Config cfg;
	
	cfg.addVar("test", CVarInt(0,0,0));
	cfg.addVar("te_st", CVarInt(0,0,0));
	cfg.addVar("testtastic", CVarInt(0,0,0));
	cfg.addVar("testing", CVarInt(0,0,0));
	
	string str("t");
	cfg.extendPrefix(str);
	puts(str.c_str());
}

void testTabComplete(void){
	Config cfg;
	
	cfg.addVar("test", CVarInt(0,0,0));
	cfg.addVar("te_st", CVarInt(0,0,0));
	cfg.addVar("testtastic", CVarInt(0,0,0));
	cfg.addVar("testing", CVarInt(0,0,0));
	
	string input;
	vector<CVar*> options;
	
	while(cout << "> " << flush, getline(cin, input)){
		if(cfg.extendPrefix(input)){
			cout << "completed to: " << input << endl;
		} else {
			options.clear();
			cfg.getVarsWithPrefix(input.c_str(), options);
			cout << "options:" << endl;
			for(int i = 0; i < options.size(); ++i){
				cout << " " << i << ".\t" << options[i]->name << endl;
			}
		}
	}
}

void testShaderUniforms(void){
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
	
	su.setUniform(0, { m });
	su.setUniform(1, { v });
	su.setUniform(2, { f });
	
	for(auto& f : su.float_uniforms){
		printf("%.2f\n", f);
	}
	
}

int main(void){

	//testShaderUniforms();
	//testPrefixExtend();
	testTabComplete();
	
	return 0;
}

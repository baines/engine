#ifndef _SHADER_H_
#define _SHADER_H_
#include "glcontext.h"

class Shader {
public:
	Shader(const char* vsname, const char* fsname) {
		load(vsname, v_id, GL_VERTEX_PROGRAM_ARB);
		load(fsname, f_id, GL_FRAGMENT_PROGRAM_ARB);
	}
private:
	void load(const char* name, GLuint& id, GLenum type){
		gl.GenProgramsARB(1, &id);
		gl.BindProgramARB(type, id);
		
		FILE* f = fopen(name, "rb");
		if(f == NULL){
			fprintf(stderr, "Couldn't load %s\n", name);
			exit(1);
		}
		fseek(f, 0L, SEEK_END);
		int sz = ftell(f);
		char* prog = new char[sz+1];
		fseek(f, 0L, SEEK_SET);
		sz = fread(prog, 1, sz, f);
		prog[sz] = '\0';
		fclose(f);
		
		gl.ProgramStringARB(type, GL_PROGRAM_FORMAT_ASCII_ARB, sz, prog);
		if(gl.GetError() == GL_INVALID_OPERATION){
			const GLubyte *errString = gl.GetString(GL_PROGRAM_ERROR_STRING_ARB);
			fprintf(stderr, "Error(s) compiling shader %s:\n %s\n", name, errString);
			exit(1);
		}
		delete [] prog;
	}
	GLuint v_id, f_id;
};
#endif

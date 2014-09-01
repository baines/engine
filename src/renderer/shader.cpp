#include "shader.h"
#include "gl_context.h"
#include <algorithm>

using namespace std;

ShaderBase::ShaderBase(GLenum type)
: type(type)
, id(0){

}

bool ShaderBase::load(const shared_ptr<Buffer>& data){
	id = gl.CreateShader(type);

	const GLchar* str = reinterpret_cast<const GLchar*>(data->data);
	const GLint str_sz = data->size;

	gl.ShaderSource(id, 1, &str, &str_sz);
	gl.CompileShader(id);

	GLint compiled_ok = GL_FALSE;
	gl.GetShaderiv(id, GL_COMPILE_STATUS, &compiled_ok);

	if(!compiled_ok){
		GLchar buffer[1024];
		gl.GetShaderInfoLog(id, sizeof(buffer), nullptr, buffer);
		fprintf(stderr, "Error compiling shader:\n%s\n", buffer);

		gl.DeleteShader(id);
		id = 0;
		return false;
	} else {
		return true;
	}
}

GLuint ShaderBase::getID(void) const {
	return id;
}

ShaderBase::~ShaderBase(){
	if(id) gl.DeleteShader(id);
}

ShaderProgram::ShaderProgram(const VertShader& v, const FragShader& f)
: vs(v)
, fs(f)
, program_id(0){

}

bool ShaderProgram::link(void){
	if(program_id) return true;
	
	program_id = gl.CreateProgram();
	
	gl.AttachShader(program_id, vs.getID());
	gl.AttachShader(program_id, fs.getID());

	gl.LinkProgram(program_id);
	
	GLint linked_ok = GL_FALSE;
	gl.GetProgramiv(program_id, GL_LINK_STATUS, &linked_ok);
	
	if(!linked_ok){
		GLchar buffer[1024];
		gl.GetProgramInfoLog(program_id, sizeof(buffer), nullptr, buffer);
		fprintf(stderr, "Error linking shader program:\n%s\n", buffer);
		
		gl.DeleteProgram(program_id);
		program_id = 0;
		return false;
	}
	
	return true;
}

bool ShaderProgram::bind(RenderState& render_state){
	if(program_id != render_state.program){
		gl.UseProgram(program_id);
		render_state.program = program_id;
	}
	return program_id != 0;
}

void ShaderProgram::setUniforms(const ShaderUniforms& uniforms){
	uniforms.bind(program_id, current_uniforms);
	current_uniforms = uniforms;
}

ShaderProgram::~ShaderProgram(){
	if(program_id) gl.DeleteProgram(program_id);
}


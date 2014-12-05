#include "shader.h"
#include "gl_context.h"
#include "vertex_state.h"
#include <algorithm>

using namespace std;

static const char* shader_name(GLenum type){
	switch(type){
		case GL_VERTEX_SHADER:   return "vertex";
		case GL_FRAGMENT_SHADER: return "fragment";
		default: return "unknown";
	}
}

ShaderBase::ShaderBase(GLenum type)
: type(type)
, id(0){

}

bool ShaderBase::loadFromResource(Engine& e, const ResourceHandle& res){
	id = gl.CreateShader(type);

	const GLchar* str = reinterpret_cast<const GLchar*>(res.data());
	const GLint str_sz = res.size();

	gl.ShaderSource(id, 1, &str, &str_sz);
	gl.CompileShader(id);

	GLint compiled_ok = GL_FALSE;
	gl.GetShaderiv(id, GL_COMPILE_STATUS, &compiled_ok);

	if(!compiled_ok){
		GLchar buffer[1024];
		gl.GetShaderInfoLog(id, sizeof(buffer), nullptr, buffer);
		
		//XXX: handle error?
		log(logging::fatal, "Error compiling %s shader:\n%s", shader_name(type), buffer);

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
	if(gl.initialized() && id){
		DEBUGF("Deleting shader %d.", id);
		gl.DeleteShader(id);
	}
}

ShaderProgram::ShaderProgram(const VertShader& v, const FragShader& f)
: vs(v)
, fs(f)
, program_id(0)
, uniforms()
, attribs() {

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
		
		//XXX: handle error?
		log(logging::fatal, "Error linking shader program:\n%s", buffer);
		
		gl.DeleteProgram(program_id);
		program_id = 0;
		
		return false;
	}
	
	GLint amount = 0;
	char name_buf[256];
	
	gl.GetProgramiv(program_id, GL_ACTIVE_UNIFORMS, &amount);
		
	for(GLint i = 0; i < amount; ++i){
		GLint size = 0;
		GLenum type = 0;
		
		gl.GetActiveUniform(program_id, i, sizeof(name_buf), nullptr, &size, &type, name_buf);
		
		const char* n = name_buf;
		if(!*n || (n[0] == 'g' && n[1] == 'l' && n[2] == '_')) continue;
		
		GLint index = gl.GetUniformLocation(program_id, name_buf);
		
		uniforms.initUniform(name_buf, program_id, index, size, type);
	}
	
	gl.GetProgramiv(program_id, GL_ACTIVE_ATTRIBUTES, &amount);
	
	for(GLint i = 0; i < amount; ++i){
		GLint size = 0;
		GLenum type = 0;
		
		gl.GetActiveAttrib(program_id, i, sizeof(name_buf), nullptr, &size, &type, name_buf);

		const char* n = name_buf;
		if(!*n || (n[0] == 'g' && n[1] == 'l' && n[2] == '_')) continue;
		
		GLint index = gl.GetAttribLocation(program_id, name_buf);
		
		uint32_t hash = str_hash(name_buf);
		attribs.initAttrib(hash, index);
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

void ShaderProgram::setUniforms(const ShaderUniforms& su){
	su.bind(program_id, uniforms);
}

void ShaderProgram::setAttribs(RenderState& rs, VertexState& vstate){
	vstate.setAttribArrays(rs, attribs);
}

ShaderProgram::~ShaderProgram(){
	if(program_id && gl.initialized()){
		gl.DeleteProgram(program_id);
	}
}


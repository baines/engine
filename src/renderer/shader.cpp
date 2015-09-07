#include "shader.h"
#include "gl_context.h"
#include "vertex_state.h"
#include "render_state.h"
#include <algorithm>

using namespace std;

static const char* shader_name(GLenum type){
	switch(type){
		case GL_VERTEX_SHADER:   return "vertex";
		case GL_FRAGMENT_SHADER: return "fragment";
		default: return "unknown";
	}
}

ShaderBase::ShaderBase(GLenum type, MemBlock mem)
: type(type)
, id(gl.CreateShader(type)){

	const GLchar* str = reinterpret_cast<const GLchar*>(mem.ptr);
	GLint str_sz = mem.size;

	const char** str_ptr = &str;
	const GLint* str_sz_ptr = &str_sz;
	GLint num_lines = 1;

#ifdef __EMSCRIPTEN__
	const char webgl_precision[] = "precision mediump float;\n";
	const char* webgl_str = strchr(str, '\n') + 1;
	const char* webgl_lines[2] = { webgl_precision, webgl_str };
	const GLint webgl_sizes[2] = { sizeof(webgl_precision) - 1, str_sz - (webgl_str - str) };

	str_ptr = webgl_lines;
	str_sz_ptr = webgl_sizes;

	num_lines = 2;
#endif

	gl.ShaderSource(id, num_lines, str_ptr, str_sz_ptr);
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
	}
}

GLuint ShaderBase::getID(void) const {
	return id;
}

ShaderBase::~ShaderBase(){
	if(gl.initialized() && id){
		TRACEF("Deleting shader %d.", id);
		gl.DeleteShader(id);
	}
}

ShaderProgram::ShaderProgram(Proxy<VertShader> v, Proxy<FragShader> f)
: vs(v)
, fs(f)
, program_id(0)
, uniforms()
, attribs() {

}

bool ShaderProgram::link(void){
	if(program_id) return true;
	
	program_id = gl.CreateProgram();
	
	gl.AttachShader(program_id, vs->getID());
	gl.AttachShader(program_id, fs->getID());

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

void ShaderProgram::onGLContextRecreate(){
	program_id = 0;

	gl.validateObject(*vs);
	gl.validateObject(*fs);

	uniforms.clear();
	attribs.clear();

	link();
}

ShaderProgram::~ShaderProgram(){
	if(program_id && gl.initialized()){
		gl.DeleteProgram(program_id);
	}
}


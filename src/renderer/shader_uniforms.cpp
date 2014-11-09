#include "shader_uniforms.h"
#include <algorithm>

bool ShaderUniforms::bind(GLuint program_id) const {
	decltype(uniforms)::const_pointer p = uniforms.data();
	
	for(auto& i : uniform_info){
		
		if(i.type == GL_INT){
			assert(i.rows == 1);
			const GLint* ip = reinterpret_cast<const GLint*>(p);
			switch(i.cols){
				case 1: gl.Uniform1iv(i.loc, i.count, ip); break;
				case 2: gl.Uniform2iv(i.loc, i.count, ip); break;
				case 3: gl.Uniform3iv(i.loc, i.count, ip); break;
				case 4: gl.Uniform4iv(i.loc, i.count, ip); break;
			}
		} else 
		if(i.type == GL_UNSIGNED_INT){
			assert(i.rows == 1);
			const GLuint* up = reinterpret_cast<const GLuint*>(p);
			switch(i.cols){
				case 1: gl.Uniform1uiv(i.loc, i.count, up); break;
				case 2: gl.Uniform2uiv(i.loc, i.count, up); break;
				case 3: gl.Uniform3uiv(i.loc, i.count, up); break;
				case 4: gl.Uniform4uiv(i.loc, i.count, up); break;
			}
		} else {
			assert(i.type == GL_FLOAT);
			const GLfloat* fp = reinterpret_cast<const GLfloat*>(p);
			     if(i.rows == 1 && i.cols == 1) gl.Uniform1fv(i.loc, i.count, fp);
			else if(i.rows == 1 && i.cols == 2) gl.Uniform2fv(i.loc, i.count, fp);
			else if(i.rows == 1 && i.cols == 3) gl.Uniform3fv(i.loc, i.count, fp);
			else if(i.rows == 1 && i.cols == 4) gl.Uniform4fv(i.loc, i.count, fp);
			else if(i.rows == 2 && i.cols == 2) gl.UniformMatrix2fv(i.loc, i.count, GL_FALSE, fp);
			else if(i.rows == 3 && i.cols == 3) gl.UniformMatrix3fv(i.loc, i.count, GL_FALSE, fp);
			else if(i.rows == 4 && i.cols == 4) gl.UniformMatrix4fv(i.loc, i.count, GL_FALSE, fp);
			else if(i.rows == 2 && i.cols == 3) gl.UniformMatrix2x3fv(i.loc, i.count, GL_FALSE, fp);
			else if(i.rows == 3 && i.cols == 2) gl.UniformMatrix3x2fv(i.loc, i.count, GL_FALSE, fp);
			else if(i.rows == 2 && i.cols == 4) gl.UniformMatrix2x4fv(i.loc, i.count, GL_FALSE, fp);
			else if(i.rows == 4 && i.cols == 2) gl.UniformMatrix4x2fv(i.loc, i.count, GL_FALSE, fp);
			else if(i.rows == 3 && i.cols == 4) gl.UniformMatrix3x4fv(i.loc, i.count, GL_FALSE, fp);
			else if(i.rows == 4 && i.cols == 3) gl.UniformMatrix4x3fv(i.loc, i.count, GL_FALSE, fp);		
		}
		p += i.rows * i.cols * i.count;
	}
	return true;
}

bool ShaderUniforms::bind(GLuint program_id, const ShaderUniforms& other) const {
	//TODO
	return bind(program_id);
}

void ShaderUniforms::initUniform(uint32_t hash, GLint count, GLuint loc){
	auto it = std::find(uniform_info.begin(), uniform_info.end(), hash);

	if(it != uniform_info.end()){
		assert(hash  == it->name_hash);
		assert(count == it->count);
		assert(loc   == it->loc);
	} else {
		//TODO: calculate rows & cols upfront from glGetActiveUniform type.
		uniform_info.push_back({ hash, 0, 0, count, -1, 0, loc });
	}
}

void ShaderUniforms::_setUniform(uint32_t hash, int rows, int cols, int count, GLenum type, const void* ptr){
	const ustorage* storage_ptr = reinterpret_cast<const ustorage*>(ptr);
	int num = rows * cols * count;

	auto it = std::find(uniform_info.begin(), uniform_info.end(), hash);

	assert(it != uniform_info.end());

	if(it->storage_index == -1){
		it->rows = rows;
		it->cols = cols;
		it->type = type;
		it->storage_index = uniforms.size();

		for(int i = 0; i < num; ++i){
			uniforms.push_back(storage_ptr[i]);
		}
	} else {
		assert(it->rows == rows && it->cols == cols && it->count == count && it->type == type);
		
		for(int i = 0; i < num; ++i){
			uniforms[it->storage_index + i] = *storage_ptr;
		}
	}
}



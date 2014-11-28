#include "shader_uniforms.h"
#include <algorithm>
#include <tuple>

namespace {
using namespace std;

tuple<GLenum, uint32_t, uint32_t> get_full_type_info(GLenum type){
	switch(type){
		case GL_FLOAT:        return make_tuple(GL_FLOAT, 1, 1);
		case GL_FLOAT_VEC2:   return make_tuple(GL_FLOAT, 1, 2);
		case GL_FLOAT_VEC3:   return make_tuple(GL_FLOAT, 1, 3);
		case GL_FLOAT_VEC4:   return make_tuple(GL_FLOAT, 1, 4);
		case GL_FLOAT_MAT2:   return make_tuple(GL_FLOAT, 2, 2);
		case GL_FLOAT_MAT3:   return make_tuple(GL_FLOAT, 3, 3);
		case GL_FLOAT_MAT4:   return make_tuple(GL_FLOAT, 4, 4);
		case GL_FLOAT_MAT2x3: return make_tuple(GL_FLOAT, 2, 3);
		case GL_FLOAT_MAT3x2: return make_tuple(GL_FLOAT, 3, 2);
		case GL_FLOAT_MAT2x4: return make_tuple(GL_FLOAT, 2, 4);
		case GL_FLOAT_MAT4x2: return make_tuple(GL_FLOAT, 4, 2);
		case GL_FLOAT_MAT3x4: return make_tuple(GL_FLOAT, 3, 4);
		case GL_FLOAT_MAT4x3: return make_tuple(GL_FLOAT, 4, 3);
	
		case GL_BOOL:
		case GL_INT:        return make_tuple(GL_INT, 1, 1);
		case GL_BOOL_VEC2:
		case GL_INT_VEC2:   return make_tuple(GL_INT, 1, 2);
		case GL_BOOL_VEC3:
		case GL_INT_VEC3:   return make_tuple(GL_INT, 1, 3);
		case GL_BOOL_VEC4:
		case GL_INT_VEC4:   return make_tuple(GL_INT, 1, 4);
		
		case GL_UNSIGNED_INT:        return make_tuple(GL_UNSIGNED_INT, 1, 1);
		case GL_UNSIGNED_INT_VEC2:   return make_tuple(GL_UNSIGNED_INT, 1, 2);
		case GL_UNSIGNED_INT_VEC3:   return make_tuple(GL_UNSIGNED_INT, 1, 3);
		case GL_UNSIGNED_INT_VEC4:   return make_tuple(GL_UNSIGNED_INT, 1, 4);
		
		case GL_SAMPLER_1D:
		case GL_SAMPLER_2D:
		case GL_SAMPLER_3D:
		case GL_SAMPLER_CUBE:
		case GL_SAMPLER_1D_SHADOW:
		case GL_SAMPLER_2D_SHADOW:
		case GL_SAMPLER_1D_ARRAY:
		case GL_SAMPLER_2D_ARRAY:
		case GL_SAMPLER_1D_ARRAY_SHADOW:
		case GL_SAMPLER_2D_ARRAY_SHADOW:
		case GL_SAMPLER_2D_MULTISAMPLE:
		case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:
		case GL_SAMPLER_CUBE_SHADOW:
		case GL_SAMPLER_BUFFER:
		case GL_SAMPLER_2D_RECT:
		case GL_SAMPLER_2D_RECT_SHADOW:
		case GL_INT_SAMPLER_1D:
		case GL_INT_SAMPLER_2D:
		case GL_INT_SAMPLER_3D:
		case GL_INT_SAMPLER_CUBE:
		case GL_INT_SAMPLER_1D_ARRAY:
		case GL_INT_SAMPLER_2D_ARRAY:
		case GL_INT_SAMPLER_2D_MULTISAMPLE:
		case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
		case GL_INT_SAMPLER_BUFFER:
		case GL_INT_SAMPLER_2D_RECT:
		case GL_UNSIGNED_INT_SAMPLER_1D:
		case GL_UNSIGNED_INT_SAMPLER_2D:
		case GL_UNSIGNED_INT_SAMPLER_3D:
		case GL_UNSIGNED_INT_SAMPLER_CUBE:
		case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY:
		case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
		case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
		case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
		case GL_UNSIGNED_INT_SAMPLER_BUFFER:
		case GL_UNSIGNED_INT_SAMPLER_2D_RECT:
												return make_tuple(GL_INT, 1, 1);
	
		default: return make_tuple(0, 0, 0);
	}
}

}

bool ShaderUniforms::bind(GLuint program_id, ShaderUniforms& active) const {
	const ustorage* p = uniforms.data();
	
	for(auto& i : uniform_info){
		
		auto ai = std::find(active.uniform_info.begin(), active.uniform_info.end(), i.name_hash);
		if(ai == active.uniform_info.end()){
			continue;
		} else if(i.type != ai->type || i.rows != ai->rows || i.cols != ai->cols || i.count != ai->count){
			DEBUGF("Uniform incompatible:\nt: %#x\t%#x\nr: %d\t%d\nc: %d\t%d\nn: %d\t%d",
			i.type, ai->type, i.rows, ai->rows, i.cols, ai->cols, i.count, ai->count);
			continue;
		}
				
		const size_t sz = i.rows & i.cols * i.count;
			
		if(memcmp(p + i.storage_index, active.uniforms.data() + ai->storage_index, sz) == 0){
			continue;
		}
		
		DEBUGF("Updating uniform %d...", ai->idx);
		memcpy(active.uniforms.data() + ai->storage_index, p + i.storage_index, sz);
		
		const GLint idx = ai->idx;
		assert(idx >= 0);
		
		if(i.type == GL_INT){
			assert(i.rows == 1);
			const GLint* ip = reinterpret_cast<const GLint*>(p+i.storage_index);
			switch(i.cols){
				case 1: gl.Uniform1iv(idx, i.count, ip); break;
				case 2: gl.Uniform2iv(idx, i.count, ip); break;
				case 3: gl.Uniform3iv(idx, i.count, ip); break;
				case 4: gl.Uniform4iv(idx, i.count, ip); break;
			}
		} else 
		if(i.type == GL_UNSIGNED_INT){
			assert(i.rows == 1);
			const GLuint* up = reinterpret_cast<const GLuint*>(p+i.storage_index);
			switch(i.cols){
				case 1: gl.Uniform1uiv(idx, i.count, up); break;
				case 2: gl.Uniform2uiv(idx, i.count, up); break;
				case 3: gl.Uniform3uiv(idx, i.count, up); break;
				case 4: gl.Uniform4uiv(idx, i.count, up); break;
			}
		} else {
			assert(i.type == GL_FLOAT);
			const GLfloat* fp = reinterpret_cast<const GLfloat*>(p+i.storage_index);
			     if(i.rows == 1 && i.cols == 1) gl.Uniform1fv(idx, i.count, fp);
			else if(i.rows == 1 && i.cols == 2) gl.Uniform2fv(idx, i.count, fp);
			else if(i.rows == 1 && i.cols == 3) gl.Uniform3fv(idx, i.count, fp);
			else if(i.rows == 1 && i.cols == 4) gl.Uniform4fv(idx, i.count, fp);
			else if(i.rows == 2 && i.cols == 2) gl.UniformMatrix2fv(idx, i.count, GL_FALSE, fp);
			else if(i.rows == 3 && i.cols == 3) gl.UniformMatrix3fv(idx, i.count, GL_FALSE, fp);
			else if(i.rows == 4 && i.cols == 4) gl.UniformMatrix4fv(idx, i.count, GL_FALSE, fp);
			else if(i.rows == 2 && i.cols == 3) gl.UniformMatrix2x3fv(idx, i.count, GL_FALSE, fp);
			else if(i.rows == 3 && i.cols == 2) gl.UniformMatrix3x2fv(idx, i.count, GL_FALSE, fp);
			else if(i.rows == 2 && i.cols == 4) gl.UniformMatrix2x4fv(idx, i.count, GL_FALSE, fp);
			else if(i.rows == 4 && i.cols == 2) gl.UniformMatrix4x2fv(idx, i.count, GL_FALSE, fp);
			else if(i.rows == 3 && i.cols == 4) gl.UniformMatrix3x4fv(idx, i.count, GL_FALSE, fp);
			else if(i.rows == 4 && i.cols == 3) gl.UniformMatrix4x3fv(idx, i.count, GL_FALSE, fp);		
		}
	}
	return true;
}

void ShaderUniforms::initUniform(const char* name, GLuint prog, GLint idx, GLuint size, GLenum full_type){
	//TODO: arrays: name will not have [n], size > 0. 
	assert(size == 1 && "Arrays are NYI :(");

	uint32_t hash = djb2(name);
	auto it = std::find(uniform_info.begin(), uniform_info.end(), hash);

	assert(it == uniform_info.end());
	
	// default value is probably 0, but get it anyway.
	
	uint32_t rows = 0, cols = 0;
	GLenum subtype;
	
	std::tie(subtype, rows, cols) = get_full_type_info(full_type);
	
	ustorage buf[16];
	
	/* 16*4 bytes should be big enough for mat4, the largest supported type,
	   but use the ARB_robustness functions anyway if they're available */
	
	if(subtype == GL_FLOAT){	
		if(gl.GetnUniformfv){
			gl.GetnUniformfv(prog, idx, sizeof(buf), reinterpret_cast<GLfloat*>(buf));
		} else {
			gl.GetUniformfv(prog, idx, reinterpret_cast<GLfloat*>(buf));
		}
	} else if(subtype == GL_INT){
		if(gl.GetnUniformiv){
			gl.GetnUniformiv(prog, idx, sizeof(buf), reinterpret_cast<GLint*>(buf));
		} else {
			gl.GetUniformiv(prog, idx, reinterpret_cast<GLint*>(buf));
		}
	} else if(subtype == GL_UNSIGNED_INT){
		if(gl.GetnUniformuiv){
			gl.GetnUniformuiv(prog, idx, sizeof(buf), reinterpret_cast<GLuint*>(buf));
		} else {
			gl.GetUniformuiv(prog, idx, reinterpret_cast<GLuint*>(buf));
		}
	} else {
		// unknown type
		log(logging::fatal, "Shader uniform '%s' has unknown type %#x", name, full_type);
		return;
	}

	const int limit = rows * cols;
	const size_t storage_idx = uniforms.size();
	
	for(int i = 0; i < limit; ++i){
		uniforms.push_back(buf[i]);
	}
	
	uniform_info.push_back({ hash, rows, cols, 1, storage_idx, subtype, idx });
}

void ShaderUniforms::_setUniform(uint32_t hash, uint32_t rows, uint32_t cols, uint32_t count, GLenum type, const void* ptr){
	const ustorage* storage_ptr = reinterpret_cast<const ustorage*>(ptr);
	const int limit = rows * cols * count;
	
	auto it = std::find(uniform_info.begin(), uniform_info.end(), hash);

	if(it == uniform_info.end()){
		const size_t storage_idx = uniforms.size();
		for(int i = 0; i < limit; ++i){
			uniforms.push_back(storage_ptr[i]);
		}
		
		uniform_info.push_back({ hash, rows, cols, count, storage_idx, type, -1 });
	} else {
		assert(rows == it->rows);
		assert(cols == it->cols);
		assert(count == it->count);
		assert(type == it->type);
		
		for(int i = 0; i < limit; ++i){
			uniforms[it->storage_index + i] = storage_ptr[i];
		}
	}
}



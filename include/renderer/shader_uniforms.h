#ifndef SHADER_UNIFORMS_
#define SHADER_UNIFORMS_
#include "common.h"
#include "gl_context.h"
#include "util.h"
#include <glm/glm.hpp>
#include <vector>
#include <initializer_list>

template<class T> struct get_glenum{};
template<> struct get_glenum<float>{ static const GLenum value = GL_FLOAT; };
template<> struct get_glenum<int>{ static const GLenum value = GL_INT; };
template<> struct get_glenum<unsigned>{ static const GLenum value = GL_UNSIGNED_INT; };

struct ShaderUniforms {
	ShaderUniforms();
	
	template<class T>
	void setUniform(const str_const& str, std::initializer_list<T>&& t){
		setUniform(str.hash, std::forward<std::initializer_list<T>>(t));
	}
	
	// for setUniform({ float, int, ... });
	template<class T>
	typename std::enable_if<std::is_scalar<T>::value>::type
	setUniform(uint32_t hash, std::initializer_list<T> vals){
		_setUniform(hash, 1, 1, vals.size(), get_glenum<T>::value, vals.begin());
	}
	
	// for setUniform({ glm::{i}vec{2,3,4}, ... });
	template<template<class, glm::precision> class V, class T, glm::precision P>
	typename std::enable_if<is_glm_vector<V, T>::value>::type
	setUniform(uint32_t hash, std::initializer_list<V<T, P>> vals){
		_setUniform(
			hash,
			vals.begin()->length(),
			1,
			vals.size(),
			get_glenum<T>::value,
			reinterpret_cast<const T*>(vals.begin())
		);
	}
	
	// for setUniform({ glm::mat{2,3,4}{x{2,3,4}}, ... });
	template<template<class, glm::precision> class M, class T, glm::precision P>
	typename std::enable_if<is_glm_matrix<M, T>::value>::type
	setUniform(uint32_t hash, std::initializer_list<M<T, P>> vals){
		_setUniform(
			hash,
			typename M<T, P>::row_type().length(),
			typename M<T, P>::col_type().length(),
			vals.size(),
			get_glenum<T>::value,
			reinterpret_cast<const T*>(vals.begin())
		);
	}
	
	void initUniform(const char* name, GLuint prog, GLint idx, GLuint size, GLenum full_type);
		
	bool bind(GLuint program_id, ShaderUniforms& current) const;

private:

	void _setUniform(uint32_t hash, uint32_t rows, uint32_t cols, uint32_t n, GLenum type, const void* ptr);

	struct uinfo {
		uint32_t name_hash, rows, cols, count;
		size_t storage_index;
		GLenum type;
		GLint idx;

		bool operator==(uint32_t h) const { return name_hash == h; }
	};

	typedef variant<GLint, GLuint, GLfloat>::type ustorage;
	static_assert(sizeof(ustorage) == 4, "ustorage should be 4 bytes");

	std::vector<ustorage> uniforms;
	std::vector<uinfo> uniform_info;
};

#endif


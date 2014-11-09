#ifndef SHADER_UNIFORMS_
#define SHADER_UNIFORMS_
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
	ShaderUniforms(){}
	
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
	
	void initUniform(uint32_t hash, GLuint count, GLuint loc);
	
	bool bind(GLuint program_id, const ShaderUniforms& current) const;

private:

	void _setUniform(uint32_t hash, int rows, int cols, int n, GLenum type, const void* ptr);

	typedef variant<GLint, GLuint, GLfloat>::type ustorage;
	static_assert(sizeof(ustorage) == 4, "ustorage should be 4 bytes");

	struct uinfo {
		uint32_t name_hash, rows, cols, count;
		int storage_index;
		GLenum type;
		GLuint loc;

		bool operator==(uint32_t h) const { return name_hash == h; }
	};

	std::vector<ustorage> uniforms;
	std::vector<uinfo> uniform_info;
};

#endif


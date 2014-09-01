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
	setUniform(unsigned loc, std::initializer_list<T> vals){
		_setUniform(loc, 1, 1, vals.size(), get_glenum<T>::value, vals.begin());
	}
	
	// for setUniform({ glm::{i}vec{2,3,4}, ... });
	template<template<class> class V, class T>
	typename std::enable_if<glm::detail::is_vector<V<T>>::_YES>::type
	setUniform(unsigned loc, std::initializer_list<V<T>> vals){
		_setUniform(
			loc,
			V<T>::value_size(),
			1,
			vals.size(),
			get_glenum<T>::value,
			reinterpret_cast<const T*>(vals.begin())
		);
	}
	
	// for setUniform({ glm::mat{2,3,4}{x{2,3,4}}, ... });
	template<template<class> class M, class T>
	typename std::enable_if<glm::detail::is_vector<typename M<T>::col_type>::_YES>::type
	setUniform(unsigned loc, std::initializer_list<M<T>> vals){
		_setUniform(
			loc,
			M<T>::row_size(),
			M<T>::col_size(),
			vals.size(),
			get_glenum<T>::value,
			reinterpret_cast<const T*>(vals.begin())
		);
	}
	
	bool bind(GLuint program_id) const;
	bool bind(GLuint program_id, const ShaderUniforms& current) const;

//private:

	void _setUniform(unsigned loc, int rows, int cols, int n, GLenum type, void* ptr){
		int num = rows * cols * n;
		for(int i = 0; i < num; ++i){
			uniforms.push_back(reinterpret_cast<decltype(uniforms)::pointer>(ptr)[i]);
		}
		uniform_info.push_back({ type, loc, rows, cols, n });
	}

	struct uinfo {
		GLenum type;
		unsigned loc;
		int rows, cols, count;
	};

	std::vector<variant<GLint, GLuint, GLfloat>::type> uniforms;
	std::vector<uinfo> uniform_info;
};

#endif


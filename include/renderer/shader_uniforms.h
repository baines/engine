#ifndef SHADER_UNIFORMS_
#define SHADER_UNIFORMS_
#include "common.h"
#include <GL/gl.h>
#include "util.h"
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
	
	template<class T>
	typename std::enable_if<std::is_scalar<T>::value>::type
	setUniform(uint32_t hash, std::initializer_list<T> vals){
		_setUniform(hash, 1, 1, vals.size(), get_glenum<T>::value, vals.begin());
	}

	template<class T>
	void_t<typename T::alt_vector_type> setUniform(uint32_t hash, std::initializer_list<T> vals){
		_setUniform(
			hash,
			1,
			T::size,
			vals.size(),
			get_glenum<typename T::value_type>::value,
			static_cast<const T*>(vals.begin())
		);
	}
	
	template<class T>
	void_t<typename T::alt_matrix_type> setUniform(uint32_t hash, std::initializer_list<T> vals){
		_setUniform(
			hash,
			T::row_size,
			T::col_size,
			vals.size(),
			get_glenum<typename T::value_type>::value,
			static_cast<const T*>(vals.begin())
		);
	}
	
	void initUniform(const char* name, GLuint prog, GLint idx, GLuint size, GLenum full_type);
	bool operator==(const ShaderUniforms& other) const;
	bool bind(GLuint program_id, ShaderUniforms& current) const;
	void clear();

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


#ifndef UTIL_H_
#define UTIL_H_
#include "common.h"
#include <memory>
#include <cstring>
#include <array>
#include <experimental/string_view>
using string_view = std::experimental::string_view;

/* Integer sequence stuff since C++14 isn't out yet */

template<unsigned...> 
struct seq { using type = seq; };

template<class S1, class S2> struct concat;
template<unsigned... I1, unsigned... I2>
struct concat<seq<I1...>, seq<I2...>>
  : seq<I1..., (sizeof...(I1)+I2)...>{};

template<unsigned N> struct gen_seq;
template<unsigned N>
struct gen_seq : concat<typename gen_seq<N/2>::type, typename gen_seq<N - N/2>::type>::type{};

template<> struct gen_seq<0> : seq<>{};
template<> struct gen_seq<1> : seq<0>{};

/* Get the arity (number of args) from a member function */

template<class T>
struct mf_arity;

template<class R, class C, class... Args>
struct mf_arity<R(C::*)(Args...)> {
	static const size_t value = sizeof...(Args);
};

template<class T, class... Ts>
struct variant {

	template<class... Us>
	struct helper {
		static const size_t align = 0;
		static const size_t size = 0;
	};

	template<class U, class... Us>
	struct helper<U, Us...> {
		static const size_t align = alignof(U) > helper<Us...>::align ?
		                            alignof(U) : helper<Us...>::align;
		static const size_t size  = sizeof(U) > helper<Us...>::size ?
		                            sizeof(U) : helper<Us...>::size;
	};

	static const size_t alignment = helper<Ts...>::align;
	static const size_t union_size = helper<Ts...>::size;
	using type = typename std::aligned_storage<union_size, alignment>::type;
};

static unsigned log2ll(uint64_t n){
	return 64 - __builtin_clzll(n) - 1;
}

inline constexpr uint32_t str_hash(const char* str, uint32_t hash = 6159){
	return *str ? str_hash(str+1, 187 * hash + *str) : hash;
}

inline uint32_t str_hash_len(const char* str, size_t len){
	uint32_t hash = 6159;
	
	for(int i = 0; i < len; ++i){
		hash = hash * 187 + str[i];
	}
	
	return hash;
}

inline uint32_t str_hash(const string_view& str){
	return str_hash_len(str.data(), str.size());
}

struct str_const {

	template<size_t N>
	constexpr str_const(const char(&s)[N])
	: str(s)
	, size(N-1)
	, hash(str_hash(s)){
	
	}
	
	constexpr bool operator==(const str_const& other) const {
		return hash == other.hash;
	}
	
	constexpr bool operator<(const str_const& other) const {
		return hash < other.hash;
	}
		
	const char* const str;
	const size_t size;
	const uint32_t hash;
};

template<class T, class... Args>
constexpr typename std::array<T, sizeof...(Args)> make_array(Args&&... args){
	return {{ T(std::forward<Args>(args))... }};
}

template<class... Args>
constexpr typename std::array<str_const, sizeof...(Args)> make_enum(Args&&... args){
	return make_array<str_const>(std::forward<Args>(args)...);
}

struct ArrayDeleter {
	void operator()(const uint8_t* arr) const { delete [] arr; }
};

//XXX: std::numpunct::truename()?
inline bool str_to_bool(const char* str){
	return str[0] == '1' 
	    || (tolower(str[0]) == 't' && tolower(str[1]) == 'r' 
	    &&  tolower(str[2]) == 'u' && tolower(str[3]) == 'e');
}

inline constexpr bool str_to_bool(const string_view& str){
	return str[0] == '1' || str.compare(0, 4, string_view("true", 4)) == 0;
}

/* GLM stuff */

#include <glm/glm.hpp>

template<template<class, glm::precision> class V, class T>
struct is_glm_vector {
	static const bool value =
		std::is_same<V<T, glm::highp>, typename glm::detail::tvec2<T, glm::highp>>::value ||
		std::is_same<V<T, glm::highp>, typename glm::detail::tvec3<T, glm::highp>>::value ||
		std::is_same<V<T, glm::highp>, typename glm::detail::tvec4<T, glm::highp>>::value
	;
};

template<template<class, glm::precision> class M, class T>
struct is_glm_matrix {
	static const bool value =
		std::is_same<M<T, glm::highp>, glm::detail::tmat2x2<T, glm::highp>>::value ||
		std::is_same<M<T, glm::highp>, glm::detail::tmat2x3<T, glm::highp>>::value ||
		std::is_same<M<T, glm::highp>, glm::detail::tmat2x4<T, glm::highp>>::value ||
		std::is_same<M<T, glm::highp>, glm::detail::tmat3x2<T, glm::highp>>::value ||
		std::is_same<M<T, glm::highp>, glm::detail::tmat3x3<T, glm::highp>>::value ||
		std::is_same<M<T, glm::highp>, glm::detail::tmat3x4<T, glm::highp>>::value ||
		std::is_same<M<T, glm::highp>, glm::detail::tmat4x2<T, glm::highp>>::value ||
		std::is_same<M<T, glm::highp>, glm::detail::tmat4x3<T, glm::highp>>::value ||
		std::is_same<M<T, glm::highp>, glm::detail::tmat4x4<T, glm::highp>>::value
	;
};


#endif

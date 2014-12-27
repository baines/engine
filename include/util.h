#ifndef UTIL_H_
#define UTIL_H_
#include "common.h"
#include <memory>
#include <cstring>
#include <array>
#include <SDL2/SDL_stdinc.h>

/* Integer sequence stuff to unwrap tuples since C++14 isn't out yet */
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

/* type to store multiple types with correct alignment */
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

/* fast integer log2 */
inline unsigned log2ll(uint64_t n){
	return 64 - __builtin_clzll(n) - 1;
}

/* fast string hashing functions */
inline constexpr uint32_t str_hash(const char* str, uint32_t hash = 6159){
	return *str ? str_hash(str+1, 187 * hash + *str) : hash;
}

inline uint32_t str_hash_len(const char* str, size_t len){
	uint32_t hash = 6159;
	
	for(size_t i = 0; i < len; ++i){
		hash = hash * 187 + str[i];
	}
	
	return hash;
}

inline uint32_t str_hash(const string_view& str){
	return str_hash_len(str.data(), str.size());
}

/* constant-expression string class */
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

/* makes an array from variadic args */
template<class T, class... Args>
constexpr typename std::array<T, sizeof...(Args)> make_array(Args&&... args){
	return {{ T(std::forward<Args>(args))... }};
}

/* specialization of make_array for str_const used in enums.h */
template<class... Args>
constexpr typename std::array<str_const, sizeof...(Args)> make_enum(Args&&... args){
	return make_array<str_const>(std::forward<Args>(args)...);
}

/* array deletion functor for use with std::shared_ptr */
struct ArrayDeleter {
	void operator()(const uint8_t* arr) const { delete [] arr; }
};

/* convert string to bool */
//XXX: std::numpunct::truename()?
inline bool str_to_bool(const char* str){
	return str[0] == '1' || SDL_strncasecmp(str, "true", 4) == 0; 
}

inline constexpr bool str_to_bool(const string_view& str){
	return str[0] == '1' || str.compare(0, 4, "true", 4) == 0;
}

inline std::u32string to_utf32(const string_view& s){
/* XXX: GCC doesn't have <codecvt> yet, despite it being part of the C++11 standard...
	
	std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv("");
	return conv.from_bytes(s.data(), s.data()+s.size());
*/
	size_t u32str_max = s.size() * sizeof(char32_t);

	char32_t* u32str = SDL_stack_alloc(char32_t, s.size());

	char* out        = reinterpret_cast<char*>(u32str);
	const char* in   = s.data();
	size_t out_sz    = u32str_max;
	size_t in_sz     = s.size();

	auto ctx = SDL_iconv_open("UTF-32LE", "UTF-8");
	SDL_iconv(ctx, &in, &in_sz, &out, &out_sz);
	
	auto ret = std::u32string(u32str, (u32str_max - out_sz) / sizeof(char32_t));
	SDL_stack_free(u32str);
	return ret;
}

/* GLM stuff to determine if a type is a vector or matrix */
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


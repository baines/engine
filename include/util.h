#ifndef UTIL_H_
#define UTIL_H_
#include "common.h"
#include <SDL_stdinc.h>

/* Macros */

#define STRINGIFY(x) #x

/* Simple wrapper for a block of memory */
struct MemBlock {
	MemBlock() = default;
	MemBlock(const void* p, size_t sz)
	: ptr(reinterpret_cast<const uint8_t*>(p)), size(sz){}

	template<class T, size_t N>
	MemBlock(const T (&arr)[N])
	: ptr(reinterpret_cast<const uint8_t*>(arr)), size(N*sizeof(T)){}

	const uint8_t* ptr;
	size_t size;
};

/* Get the arity (number of args) from a member function */
template<class T>
struct mf_arity;

template<class R, class C, class... Args>
struct mf_arity<R(C::*)(Args...)> {
	static const size_t value = sizeof...(Args);
};

template<class T>
T clamp(T val, T min, T max){
	return std::max<T>(min, std::min<T>(max, val));
}

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

/* 64-bit popcount */
inline unsigned popcount64(uint64_t n){
	return __builtin_popcountll(n);
}

inline uint32_t next_pow_of_2(uint32_t v){
	--v;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	return ++v;
}

/* lerp */
template<class T>
T lerp(T a, T b, float t){
	return a + (b - a) * t;
}

/* fast string hashing functions */

inline constexpr strhash_t str_hash(const char* str, uint32_t hash = 6159){
	return *str ? str_hash(str+1, 187 * hash + *str) : hash;
}

inline strhash_t str_hash_len(const char* str, size_t len){
	strhash_t hash = 6159;
	
	for(size_t i = 0; i < len; ++i){
		hash = hash * 187 + str[i];
	}
	
	return hash;
}

inline strhash_t str_hash(const alt::StrRef& str){
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
	const strhash_t hash;
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
	return str[0] == '1' || strncasecmp(str, "true", 4) == 0; 
}

inline bool str_to_bool(const alt::StrRef& str){
	return str[0] == '1' || str.cmp("true");
}

/* unicode related things */
inline alt::StrMut32 to_utf32(const alt::StrRef& s){
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
	SDL_iconv_close(ctx);
	
	alt::StrMut32 ret(u32str, (u32str_max - out_sz) / sizeof(char32_t));
	SDL_stack_free(u32str);
	return ret;
}

inline bool is_utf8_continuation(char c){
	return (c & 0xC0) == 0x80;
}

inline size_t utf8_char_index(const alt::StrRef& str, size_t utf32_index){
	for(size_t i = 0; i < str.size(); ++i){
		if(!is_utf8_continuation(str[i])){
			if(utf32_index-- == 0) return i;
		}
	}

	return str.size();	
}

/* Pointer that nulls itself on move, so the default move assign/constructors work */

template<class T>
struct NullOnMovePtr {
	NullOnMovePtr() : ptr(nullptr){}
	NullOnMovePtr(nullptr_t) : NullOnMovePtr(){}
	NullOnMovePtr(T* ptr) : ptr(ptr){}

	T& operator* (){ return *ptr; }
	T* operator->(){ return ptr; }

	void operator=(NullOnMovePtr&& p){
		ptr = p.ptr;
		p.ptr = nullptr;
	}

	T*& get() { return ptr; }
	T* const& get() const { return ptr; }

	operator T*() { return ptr; }
private:
	T* ptr;
};

/* GLM stuff to determine if a type is a vector or matrix */
#include "glm/glm.hpp"

namespace glmstuff {

using namespace glm;
using namespace glm::detail;
	
template<template<class, glm::precision> class V, class T>
struct is_glm_vector {
	static const bool value =
		std::is_same<V<T, highp>, tvec2<T, highp>>::value ||
		std::is_same<V<T, highp>, tvec3<T, highp>>::value ||
		std::is_same<V<T, highp>, tvec4<T, highp>>::value
	;
};

template<template<class, glm::precision> class M, class T>
struct is_glm_matrix {
	static const bool value =
		std::is_same<M<T, highp>, tmat2x2<T, highp>>::value ||
		std::is_same<M<T, highp>, tmat2x3<T, highp>>::value ||
		std::is_same<M<T, highp>, tmat2x4<T, highp>>::value ||
		std::is_same<M<T, highp>, tmat3x2<T, highp>>::value ||
		std::is_same<M<T, highp>, tmat3x3<T, highp>>::value ||
		std::is_same<M<T, highp>, tmat3x4<T, highp>>::value ||
		std::is_same<M<T, highp>, tmat4x2<T, highp>>::value ||
		std::is_same<M<T, highp>, tmat4x3<T, highp>>::value ||
		std::is_same<M<T, highp>, tmat4x4<T, highp>>::value
	;
};

}

#endif


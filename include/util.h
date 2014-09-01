#ifndef UTIL_H_
#define UTIL_H_
#include <memory>
#include <cstring>
#include <array>

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

constexpr size_t djb2(const char* str){
	return *str ? djb2(str+1) * 33 + *str : 5381;
}

struct str_const {
	template<size_t N>
	constexpr str_const(const char(&s)[N])
	: str(s)
	, size(N)
	, hash(djb2(s)){
	
	}
	
	constexpr bool operator==(const str_const& other){
		return hash == other.hash;
	}
		
	const char* const str;
	const size_t size;
	const size_t hash;
};

template<class T, class... Args>
constexpr typename std::array<T, sizeof...(Args)> make_array(Args&&... args){
	return {{ T(std::forward<Args>(args))... }};
}

template<class... Args>
constexpr typename std::array<str_const, sizeof...(Args)> make_enum(Args&&... args){
	return make_array<str_const>(std::forward<Args>(args)...);
}

#endif

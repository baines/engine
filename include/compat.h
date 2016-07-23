// compatibility for mingw-w64 4.8
#ifndef COMPAT_H_
#define COMPAT_H_
#include <memory>
#include <string.h>
#include <algorithm>

namespace compat {

template<size_t...> struct seq { using type = seq; };

template<class S1, class S2> struct concat;

template<size_t... I1, size_t... I2>
struct concat<seq<I1...>, seq<I2...>> : seq<I1..., (sizeof...(I1)+I2)...>{};

template<size_t N> struct gen_seq;

template<size_t N>
struct gen_seq : concat<typename gen_seq<N/2>::type, typename gen_seq<N - N/2>::type>::type{};

template<> struct gen_seq<0> : seq<>{};
template<> struct gen_seq<1> : seq<0>{};

template<size_t N, class T, class Tuple>
constexpr typename std::enable_if<
	std::is_same<T, typename std::tuple_element<N, Tuple>::type>::value,
	T&
>::type do_tuple_get(Tuple& tup){
	return std::get<N>(tup);
}

template<size_t N, class T, class Tuple>
constexpr typename std::enable_if<
	!std::is_same<T, typename std::tuple_element<N, Tuple>::type>::value,
	T&
>::type do_tuple_get(Tuple& tup){
	return do_tuple_get<N+1, T, Tuple>(tup);
}
		
}

inline const char* strchrnul(const char* haystack, char needle){
	const char* p = haystack;
	
	for(; *p; ++p){
		if(*p == needle) break;
	}

	return p;
}
namespace std {

	template<size_t... SZ>
	using index_sequence = typename compat::seq<SZ...>;

	template<size_t N>
	constexpr auto make_index_sequence(void){
		return typename compat::gen_seq<N>::type();
	}

	template< class T, class... Args >
	std::unique_ptr<T> make_unique( Args&&... args ){
		return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
	}

	template<class T, class Tuple>
	T& get(Tuple& t){
		return compat::do_tuple_get<0, T, Tuple>(t);
	}
}

#endif

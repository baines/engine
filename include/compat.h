// compatibility for mingw-w64 4.8
#ifndef COMPAT_H_
#define COMPAT_H_
#include <memory>
#include <cstring>
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

template<class CT>
struct string_view_template {

	string_view_template() = default;

	template<size_t N>
	string_view_template(const CT (&str)[N]) : ptr(str), sz(N-1){}

	string_view_template(const CT* str) : ptr(str), sz(strlen(str)){}

	string_view_template(const CT* str, size_t sz) : ptr(str), sz(sz){}

	string_view_template(const std::basic_string<CT>& str) : ptr(&str[0]), sz(str.size()){}
	
	size_t size() const { return sz; }
	const CT* data() const { return ptr; }

	void copy(CT* buff, size_t buff_sz) const {
		size_t min_sz = std::min(buff_sz, sz);
		memcpy(buff, ptr, min_sz);
	}

	const CT& operator[](size_t index) const {
		return ptr[index];
	}

	int compare(size_t idx, size_t len, const CT* txt, size_t txt_len) const {
		size_t min_sz = std::min(std::min(sz - idx, len), txt_len);
		return memcmp(ptr + idx, txt, min_sz);
	}

	int find(const string_view_template& other) const {
		auto* p = std::search(ptr, ptr + sz, other.ptr, other.ptr + other.sz);
		return *p ? p - ptr : -1;
	}

	CT front() const { return ptr ? ptr[0] : 0; };
	CT back() const { return ptr && sz ? ptr[sz-1] : 0; };

	void remove_prefix(size_t N){
		size_t min_sz = std::min(sz, N);
		ptr += min_sz;
		sz -= min_sz;
	}

	void remove_suffix(size_t N){
		sz -= std::min(sz, N);
	}

	std::basic_string<CT> to_string() const {
		return std::basic_string<CT>(ptr, sz);
	}

	operator std::basic_string<CT>() const {
		return to_string();
	}

private:
	const CT* ptr;
	size_t sz;
};

inline const char* strchrnul(const char* haystack, char needle){
	const char* p = haystack;
	
	for(; *p; ++p){
		if(*p == needle) break;
	}

	return p;
}

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

#define strchrnul compat::strchrnul

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

	namespace experimental {

		using string_view = typename compat::string_view_template<char>;
		using u32string_view = typename compat::string_view_template<char32_t>;

	}
}

#endif

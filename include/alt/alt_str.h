#ifndef ALT_STR_H_
#define ALT_STR_H_
#include <cstdint>
#include <alloca.h>
#include <initializer_list>

/********************************************************
  Define these to use your own memory-related functions
********************************************************/

#if !defined(ALT_STR_ALLOC) || !defined(ALT_STR_FREE)
	#include <cstdlib>
	#define ALT_STR_ALLOC    malloc
	#define ALT_STR_FREE     free
#endif

#if !defined(ALT_STR_MEMCPY)
	#include <cstring>
	#define ALT_STR_MEMCPY   memcpy
#endif

#if !defined(ALT_STR_MEMMOVE)
	#include <cstring>
	#define ALT_STR_MEMMOVE  memmove
#endif

#if !defined(ALT_STR_MEMCMP)
	#include <cstring>
	#define ALT_STR_MEMCMP   memcmp
#endif

namespace alt {

/********************************************************
          Miscellanea used by the string types
********************************************************/

namespace detail {

	template<class T> constexpr T min(const T& a, const T& b){ return a <  b ? a : b; }
	template<class T> constexpr T max(const T& a, const T& b){ return a >= b ? a : b; }

	template<class T>
	size_t StrLen(const T* ptr){
		const T* p = ptr;
		while(*p) ++p;
		return p - ptr;
	}

	inline size_t get_next_size(size_t current, size_t required){
		current = max<size_t>(current, 32);
		while(current < required){
			current <<= 2;
		}
		return current;
	}

	template<bool, class T = void> struct enable_if{};
	template<class T> struct enable_if<true, T> { typedef T type; };

	static const size_t STR_REF_ALLOCA_LIMIT = 1024;
}

template<class T> class TStrMut;

/********************************************************
   StrRef, StrRef16, StrRef32 - Immutable string types
********************************************************/

template<class T>
struct TStrRef {

	typedef T* iterator;
	typedef const T* const_iterator;

	TStrRef() = default;
	
	constexpr TStrRef(const T* p, size_t sz) : ptr(p), sz(sz){}
	constexpr TStrRef(const T* a, const T* b) : ptr(a), sz(b - a){}
	constexpr TStrRef(const T& c) : ptr(&c), sz(1){}

	constexpr TStrRef(const TStrMut<T>& str);

	template<size_t N>
	constexpr TStrRef(const T (&p)[N]) : ptr(p), sz(N-1){}

	template<class U>
	constexpr TStrRef(const U* const& p) : ptr(p), sz(detail::StrLen<T>(p)){}

	template<class F>
	auto pass_c_str(F&& func) const -> decltype(func((T*)1)) {

		struct ScopeDel {
			~ScopeDel(){ if(ptr) ALT_STR_FREE(ptr); }
			T* ptr = nullptr;
		} auto_del;

		T* buf;
		
		if((sz * sizeof(T)) >= detail::STR_REF_ALLOCA_LIMIT){
			buf = (T*) ALT_STR_ALLOC((sz+1) * sizeof(T));
			auto_del.ptr = buf;
		} else {
			buf = reinterpret_cast<T*>(alloca((sz+1)*sizeof(T)));
		}

		ALT_STR_MEMCPY(buf, ptr, sz * sizeof(T));
		buf[sz] = 0;

		return func(buf);
	}

	constexpr const T& front() const { return ptr[0]; }
	constexpr const T& back() const { return ptr[sz-1]; }
	
	bool contains(const TStrRef<T>& needle, size_t b = 0, size_t e = -1) const {
		return find(needle, b, e) != sz;
	}
	
	bool contains_not(const TStrRef<T>& needle, size_t b = 0, size_t e = -1) const {
		return find_not(needle, b, e) != sz;
	}

	size_t find(const TStrRef<T>& needle, size_t b = 0, size_t e = -1) const {
		const size_t max_sz = detail::min<uint32_t>(e, sz);
		const size_t limit  = detail::max<ssize_t>(0, max_sz - needle.size());

		for(const T* p = ptr + b; p < ptr + limit; ++p){
			if(substr(p, p + needle.size()) == needle){
				return p - ptr;
			}
		}

		return max_sz; 
	}	

	size_t rfind(const TStrRef<T>& needle, size_t b = -1, size_t e = -1) const {
		const size_t max_sz = detail::min<uint32_t>(b, sz);
		const size_t limit  = detail::max<ssize_t>(0, max_sz - needle.size());

		for(const T* p = ptr + limit; p >= ptr + (e+1); --p){
			if(substr(p, p + needle.size()) == needle){
				return p - ptr;
			}
		}

		return e; 
	}
	
	size_t find_any(const TStrRef<T>& chars, size_t b = 0, size_t e = -1) const {
		const size_t max_sz = detail::min<uint32_t>(e, sz);

		for(const T* p = ptr + b; p < ptr + max_sz; ++p){
			for(char c : chars){
				if(c == *p) return p - ptr;
			}
		}

		return max_sz; 
	}

	size_t rfind_any(const TStrRef<T>& chars, size_t b = -1, size_t e = -1) const {
		const size_t max_sz = detail::min<uint32_t>(b, sz);
		const size_t limit  = detail::max<ssize_t>(0, max_sz - 1);

		for(const T* p = ptr + limit; p >= ptr + (e+1); --p){
			for(char c : chars){
				if(c == *p) return p - ptr;
			}
		}

		return e; 
	}

	size_t find_not(const TStrRef<T>& chars, size_t b = 0, size_t e = -1) const {
		const size_t max_sz = detail::min<uint32_t>(e, sz);

		for(const T* p = ptr + b; p < ptr + max_sz; ++p){
			for(char c : chars){
				if(c != *p) return p - ptr;
			}
		}

		return max_sz; 
	}

	size_t rfind_not(const TStrRef<T>& chars, size_t b = -1, size_t e = -1) const {
		const size_t max_sz = detail::min<uint32_t>(b, sz);
		const size_t limit  = detail::max<ssize_t>(0, max_sz - 1);

		for(const T* p = ptr + limit; p >= ptr + (e+1); --p){
			for(char c : chars){
				if(c != *p) return p - ptr;
			}
		}

		return e; 
	}

	void remove_prefix(size_t n){
		size_t min_sz = detail::min(sz, n);
		ptr += min_sz;
		sz -= min_sz;	
	}

	void remove_suffix(size_t n){
		sz = detail::max<intmax_t>(0, sz - n);
	}

	TStrRef<T> substr(size_t index, size_t count) const {
		return substr(ptr + index, ptr + index + count);
	}
	TStrRef<T> substr(const T* begin, const T* end) const {
		return TStrRef<T>(begin, end);
	}
	
	bool cmp(const TStrRef<T>& other) const {
		return size() == other.size() && ALT_STR_MEMCMP(ptr, other.data(), size() * sizeof(T)) == 0;
	}
	
	bool operator==(const TStrRef<T>& other) const { return cmp(other); }
	bool operator!=(const TStrRef<T>& other) const { return !cmp(other); }
	
	constexpr const T* data() const { return ptr; }
	constexpr const T* begin() const { return ptr; }
	constexpr const T* end() const { return ptr + sz; }
	constexpr const T& operator[](size_t i) const { return ptr[i]; }
	
	constexpr size_t size() const { return sz; }
	constexpr int sizei() const { return static_cast<int>(sz); }
private:
	const T* ptr;
	size_t sz;
};

using StrRef   = TStrRef<char>;
using StrRef16 = TStrRef<char16_t>;
using StrRef32 = TStrRef<char32_t>;

/********************************************************
   StrMut, StrMut16, StrMut32 - Mutable string types
********************************************************/

template<class T>
class TStrMut {
	static constexpr size_t LOCAL_SZ = (28 - sizeof(T*)) / sizeof(T);
public:
	TStrMut() : ptr(local_str), used_size(0) {}
		
	template<size_t N>
	TStrMut(const T (&ref)[N], typename detail::enable_if<!(N <= LOCAL_SZ)>::type* = 0)
	: ptr(ALT_STR_ALLOC(detail::get_next_size(0, N) * sizeof(T)))
	, used_size(N-1)
	, allocated_size(detail::get_next_size(0, N)) {
		ALT_STR_MEMCPY(ptr, &ref, N * sizeof(T));
	}

	template<size_t N>
	TStrMut(const T (&ref)[N], typename detail::enable_if<N <= LOCAL_SZ>::type* = 0)
	: ptr(local_str)
	, used_size(N-1) {
		ALT_STR_MEMCPY(local_str, &ref, N * sizeof(T));
	}
	
	TStrMut(const T* t, size_t sz)
	: ptr(sz >= LOCAL_SZ ? (T*)ALT_STR_ALLOC(detail::get_next_size(0, sz+1) * sizeof(T)) : local_str)
	, used_size(sz){
		ALT_STR_MEMCPY(ptr, t, sz * sizeof(T));
		ptr[used_size] = 0;
		if(ptr != local_str){
			allocated_size = detail::get_next_size(0, sz+1);
		}
	}
	
	TStrMut(const T* a, const T* b) : TStrMut(a, b - a){}
	TStrMut(const TStrRef<T>& str) : TStrMut(str.data(), str.size()){}
	TStrMut(const TStrMut<T>& str) : TStrMut(str.c_str(), str.size()){}
		
	TStrMut<T>& operator=(TStrMut<T>&& other){
		used_size = other.size();
		
		if(other.ptr != other.local_str){
			if(ptr != local_str) delete [] ptr;
			ptr = other.ptr;
			allocated_size = other.allocated_size;
			
			other.ptr = other.local_str;
		} else {
			reserve(used_size);
			ALT_STR_MEMCPY(ptr, other.ptr, other.used_size * sizeof(T));
			ptr[used_size] = 0;
		}

		other.clear();

		return *this;
	}
	
	TStrMut<T>& operator=(const TStrMut<T>& other){
		used_size = other.size();
		reserve(used_size);
		ALT_STR_MEMCPY(ptr, other.c_str(), used_size * sizeof(T));
		ptr[used_size] = 0;

		return *this;
	}

	TStrMut<T>& assign(const TStrRef<T>& other){
		return (*this) = other;
	}
	
	template<size_t N>
	TStrMut<T>& operator=(const char (&other)[N]){
		used_size = N-1;
		reserve(used_size);
		ALT_STR_MEMCPY(ptr, &other, used_size * sizeof(T));
		ptr[used_size] = 0;

		return *this;
	}
		
	TStrMut<T>& clear(){
		used_size = 0;
		*ptr = 0;
		return *this;
	}
	
	void reserve(size_t amount){
		const bool using_local = ptr == local_str;
		const size_t limit = using_local ? LOCAL_SZ : allocated_size;

		if(amount >= limit){
			const size_t new_size = detail::get_next_size(limit, amount + 1);
			T* new_ptr = (T*) ALT_STR_ALLOC(new_size * sizeof(T));
			ALT_STR_MEMCPY(new_ptr, ptr, (1 + used_size) * sizeof(T));
			if(!using_local) ALT_STR_FREE(ptr);
			ptr = new_ptr;
			allocated_size = new_size;
		}
	}
		
	TStrMut<T>& insert(size_t index, const TStrRef<T>& str){
		const size_t _sz = str.size();
		reserve(used_size + _sz);
		ALT_STR_MEMMOVE(ptr + index + _sz, ptr + index, (used_size - index) * sizeof(T));
		ALT_STR_MEMCPY(ptr + index, str.data(), _sz * sizeof(T));
		used_size += _sz;
		ptr[used_size] = 0;

		return *this;
	}
	
	TStrMut<T>& erase(size_t index, size_t count){
		erase(ptr + index, ptr + index + count);

		return *this;
	}
	
	TStrMut<T>& erase(T* _begin, T* _end){
		ALT_STR_MEMMOVE(_begin, _end, (end() - _end) * sizeof(T));
		used_size -= _end - _begin;
		ptr[used_size] = 0;

		return *this;
	}
	
	TStrMut<T>& append(const TStrRef<T>& str){
		const size_t _sz = str.size();
		reserve(used_size + _sz);
		ALT_STR_MEMCPY(ptr + used_size, str.data(), _sz * sizeof(T));
		used_size += _sz;
		ptr[used_size] = 0;

		return *this;
	}
	
	TStrMut<T>& append(std::initializer_list<TStrRef<T>> strs){
		size_t total = 0;
		for(const auto& s : strs){
			total += s.size();	
		}

		reserve(total);
		
		for(const auto& s : strs){
			const size_t _sz = s.size();
			ALT_STR_MEMCPY(ptr + used_size, s.data(), _sz * sizeof(T));
			used_size += _sz;
		}
		
		ptr[used_size] = 0;

		return *this;
	}
	
	TStrMut<T>& append(size_t num, const TStrRef<T>& str){
		const size_t _sz = str.size();
		const T* _data = str.data();
		
		reserve(used_size + (_sz * num));
		while(num--){
			ALT_STR_MEMCPY(ptr + used_size, _data, _sz * sizeof(T));
			used_size += _sz;
		}
		ptr[used_size] = 0;

		return *this;
	}
		
	TStrMut<T>& replace(size_t index, size_t count, const TStrRef<T>& str){
		const size_t _sz = str.size();
		const int diff = _sz - count;

		reserve(used_size + diff);
		T *_begin = ptr + index, *_end = ptr + index + count;
		if(diff != 0){
			ALT_STR_MEMMOVE(_begin + _sz, _end, (end() - _end) * sizeof(T));
			used_size += diff;
			ptr[used_size] = 0;
		}
		ALT_STR_MEMCPY(_begin, str.data(), _sz * sizeof(T));

		return *this;
	}
	
	TStrMut<T>& replace(T* _begin, T* _end, const TStrRef<T>& str){
		replace(_begin - ptr, _end - _begin, str);

		return *this;
	}

	bool contains(const TStrRef<T>& needle, size_t b = 0, size_t e = -1) const {
		return find(needle, b, e) != used_size;
	}
	
	bool contains_not(const TStrRef<T>& needle, size_t b = 0, size_t e = -1) const {
		return find_not(needle, b, e) != used_size;
	}

	size_t find(const TStrRef<T>& needle, size_t b = 0, size_t e = -1) const {
		const size_t max_used_size = detail::min<uint32_t>(e, used_size);
		const size_t limit  = detail::max<ssize_t>(0, max_used_size - needle.size());

		for(const T* p = ptr + b; p < ptr + limit; ++p){
			if(substr(p, p + needle.size()) == needle){
				return p - ptr;
			}
		}

		return max_used_size; 
	}	

	size_t rfind(const TStrRef<T>& needle, size_t b = -1, size_t e = -1) const {
		const size_t max_used_size = detail::min<uint32_t>(b, used_size);
		const size_t limit  = detail::max<ssize_t>(0, max_used_size - needle.size());

		for(const T* p = ptr + limit; p >= ptr + (e+1); --p){
			if(substr(p, p + needle.size()) == needle){
				return p - ptr;
			}
		}

		return e; 
	}
	
	size_t find_any(const TStrRef<T>& chars, size_t b = 0, size_t e = -1) const {
		const size_t max_used_size = detail::min<uint32_t>(e, used_size);

		for(const T* p = ptr + b; p < ptr + max_used_size; ++p){
			for(char c : chars){
				if(c == *p) return p - ptr;
			}
		}

		return max_used_size; 
	}

	size_t rfind_any(const TStrRef<T>& chars, size_t b = -1, size_t e = -1) const {
		const size_t max_used_size = detail::min<uint32_t>(b, used_size);
		const size_t limit  = detail::max<ssize_t>(0, max_used_size - 1);

		for(const T* p = ptr + limit; p >= ptr + (e+1); --p){
			for(char c : chars){
				if(c == *p) return p - ptr;
			}
		}

		return e; 
	}

	size_t find_not(const TStrRef<T>& chars, size_t b = 0, size_t e = -1) const {
		const size_t max_used_size = detail::min<uint32_t>(e, used_size);

		for(const T* p = ptr + b; p < ptr + max_used_size; ++p){
			for(char c : chars){
				if(c != *p) return p - ptr;
			}
		}

		return max_used_size; 
	}

	size_t rfind_not(const TStrRef<T>& chars, size_t b = -1, size_t e = -1) const {
		const size_t max_used_size = detail::min<uint32_t>(b, used_size);
		const size_t limit  = detail::max<ssize_t>(0, max_used_size - 1);

		for(const T* p = ptr + limit; p >= ptr + (e+1); --p){
			for(char c : chars){
				if(c != *p) return p - ptr;
			}
		}

		return e; 
	}

	bool cmp(const TStrRef<T>& other) const {
		return size() == other.size() && ALT_STR_MEMCMP(ptr, other.data(), size() * sizeof(T)) == 0;
	}
	
	bool operator==(const TStrRef<T>& other) const { return cmp(other); }
	bool operator!=(const TStrRef<T>& other) const { return !cmp(other); }
	
	TStrRef<T> substr(size_t index, size_t count) const {
		return substr(ptr + index, ptr + index + count);
	}
	TStrRef<T> substr(const T* begin, const T* end) const {
		return TStrRef<T>(begin, end);
	}

	bool empty() const { return used_size == 0; }

	T* c_str() const { return ptr; }

	T* data(){ return ptr; }
	const T* data() const { return ptr; }

	constexpr T* begin(){ return ptr; }
	constexpr T* end(){ return ptr + used_size; }
	
	constexpr size_t size() const { return used_size; }
	
	constexpr T& operator[](size_t index){ return ptr[index]; }

	constexpr T& front(){ return *ptr; }
	constexpr T& back(){ return ptr[used_size-1]; }
	
	~TStrMut(){
		if(ptr != local_str) delete [] ptr;
	}
private:
	T* ptr;
	uint32_t used_size;
	union {
		uint32_t allocated_size;
		T local_str[LOCAL_SZ];
	};
};


using StrMut   = TStrMut<char>;
using StrMut16 = TStrMut<char16_t>;
using StrMut32 = TStrMut<char32_t>;

template<class T>
constexpr TStrRef<T>::TStrRef(const TStrMut<T>& str) : ptr(str.data()), sz(str.size()){}

}

/********************************************************************
   Iostream << compat stuff, define ALT_STR_IOSTREAM if you want it
********************************************************************/

#ifdef ALT_STR_IOSTREAM
#include <iostream>

namespace alt {

template<class T>
std::ostream& operator<<(std::ostream& stream, const TStrRef<T>& str){
	str.pass_c_str([&](T* cstr){
		stream << cstr;
	});
}

template<class T>
std::ostream& operator<<(std::ostream& stream, const TStrMut<T>& str){
	stream << str.c_str();
}

}
#endif

#endif


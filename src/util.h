#ifndef _UTIL_H_
#define _UTIL_H_
#include <string>
#include <map>

namespace util {
template <class T> const T& max ( const T& a, const T& b ){ return (a>b)?a:b; }
template <class T> const T& min ( const T& a, const T& b ){ return (a<b)?a:b; }	
	template<class InputIterator, class Function, class Arg>
	void forEach(InputIterator first, InputIterator last, Function f, Arg& a) {
		for ( ; first!=last; ++first ) ((*first)->*f)(a);
	}
	
	template<class InputIterator, class Function, class Arg>
	void forEach2(InputIterator first, InputIterator last, Function f, Arg& a) {
		for ( ; first!=last; ++first ) ((*first).*f)(a);
	}
	
	template<class T, class Map, class Fn>
	T cache(Map& map, const std::string& key, Fn fn, const char* prefix){
		typename Map::const_iterator i = map.find(key);
		if(i != map.end()){
			return i->second;
		} else {
			T t;
			if(prefix){
				std::string path = prefix + key;
				t = fn(path.c_str());
			} else {
				t = fn(key.c_str());
			}
			if(t) map.insert(std::pair<std::string, T>(key, t)); 
			return t;
		}
	}
			
	struct deleter {
		template<typename T> 
		void operator()(T* t){
			delete t;
		}
	};
}

#endif

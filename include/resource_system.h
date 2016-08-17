#ifndef RESOURCE_SYSTEM_H_
#define RESOURCE_SYSTEM_H_
#include "common.h"
#include "util.h"
#include <unordered_map>

struct ResourceHandle {
	uint8_t* data;
	size_t size;
	ResourceSystem* res;
	operator bool(){ return data; }
	~ResourceHandle(){}
};

struct ResourceSystem {
	ResourceSystem(const char* argv0);
	ResourceHandle load(const char* name);

#if 0
	template<class T, class Tuple>
	T* load_convert(std::initializer_list<const char*> names, const Tuple& tup){
		Closure* func = nullptr;

		for(const char* n : names){
			auto key = make_pair(str_hash(n), TypeID<Tuple>());
			if(auto it = res_cache.find(key)){
				func = it->second;
				break;
			}
		}

		if(func){
			// loop through all multimap entries
			if(void* val = func(&tup)){
				return reinterpret_cast<T*>(val);
			} else {
				// no valid multimap entry?
				// add a new one like below, except we know the key already
			}
		} else {

			// - iterate names, call load until one works
			// - new T(unpack tup);
			// - store in closure along with [tup]
		}
	}
#endif

	size_t getUseCount(const char* name);
	~ResourceSystem();

private:
	ResourceHandle load_uncached(const char* name);

	std::unordered_map<strhash_t, ResourceHandle> res_data;
//	std::unordered_multimap<std::pair<strhash_t, size_t>, Closure>        res_cache;
};

#endif

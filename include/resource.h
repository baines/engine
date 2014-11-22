#ifndef RESOURCE_H_
#define RESOURCE_H_
#include "common.h"
#include "resource_system.h"
#include <utility>

template<class T>
struct Resource {
	Resource(const char* name)
	: t_storage()
	, t(*reinterpret_cast<T*>(&t_storage))
	, loaded(false)
	, res_name(name)
	, res_handle() {
		new(&t_storage) T();
	}

	bool load(Engine& e){
		if(loaded) return true;

		T* cached = e.res.cache<T>().get(res_name);
		if(cached){
			new(&t_storage) T(*cached);
			
			loaded = true;
		} else {			
			res_handle = e.res.load(res_name);
			if(t.load(res_handle)){
				loaded = true;
				e.res.cache<T>().add(res_name, t);
			}
		}
		
		return loaded;
	}

	bool isLoaded(void) const {
		return loaded;
	}

	T* operator->(void) const {
		return &t;
	}
	
	T& operator*(void) const {
		return t;
	}

private:
	typename std::aligned_storage<sizeof(T), alignof(T)>::type t_storage;
	T& t;
	bool loaded;
	const char* res_name;
	ResourceHandle res_handle;
};

#endif

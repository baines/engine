#ifndef RESOURCE_H_
#define RESOURCE_H_
#include "resource_system.h"

template<class T>
struct Resource {
	Resource(const char* name)
	: t_storage()
	, t(*reinterpret_cast<T*>(t_storage))
	, loaded(false)
	, res_name(name)
	, res_data() {

	}

	bool load(Engine& e){
		if(loaded) return true;

		T* cached = e.res.cache<T>().get(res_name);
		if(cached){
			new(&t_storage) T(*cached);
			
			loaded = true;
		} else {
			new(&t_storage) T();
			
			res_data = e.res.load(res_name);
			if(t.load(res_data)){
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
		if(!loaded){
			return nullptr;
		} else {
			return &t;
		}
	}
	
	T* get(void) const {
		if(!loaded){
			return nullptr;
		} else {
			return &t;
		}
	}

private:
	aligned_storage<sizeof(T), alignof(T)>::type t_storage;
	T& t;
	bool loaded;
	const char* res_name
	shared_ptr<Buffer> res_data;
};

};

#endif

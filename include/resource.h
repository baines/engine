#ifndef RESOURCE_H_
#define RESOURCE_H_
#include "common.h"
#include "resource_system.h"
#include <utility>

template<class T>
struct Resource {

	template<class... Args>
	Resource(Engine& e, std::initializer_list<const char*> names, Args&&... args)
	: t_storage()
	, t(*reinterpret_cast<T*>(&t_storage))
	, loaded(false)
	, load_result(false)
	, res_names(names)
	, res_handle()
	, cache_get_fn(std::bind(
		&ResourceSystem::Cache<T, Args...>::get,
		&e.res.cache<T, Args...>(),
		std::placeholders::_1,
		std::forward<Args>(args)...
	))
	, cache_put_fn(std::bind(
		&ResourceSystem::Cache<T, Args...>::add,
		&e.res.cache<T, Args...>(),
		std::placeholders::_1,
		std::placeholders::_2,
		std::forward<Args>(args)...
	)){
		new(&t_storage) T(std::forward<Args>(args)...);
	}

	bool load(Engine& e){
		if(loaded) return load_result;
		
		for(auto* n : res_names){
			if(T* cached = cache_get_fn(n)){
				new(&t_storage) T(*cached);
				loaded = load_result = true;
				break;
			}
		}

		if(!loaded){
			for(auto* n : res_names){
				if(ResourceHandle rh = e.res.load(n)){
					load_result = t.load(rh);
					loaded = true;
					cache_put_fn(t, n);
					res_handle = std::move(rh);
					break;
				}
			}
		}

		return load_result;
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
	bool loaded, load_result;
	std::vector<const char*> res_names;
	ResourceHandle res_handle;
	std::function<T*(const char*)> cache_get_fn;
	std::function<void(const T&, const char*)> cache_put_fn;
};

#endif

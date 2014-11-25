#ifndef RESOURCE_H_
#define RESOURCE_H_
#include "common.h"
#include "resource_system.h"
#include <utility>

template<class T, class... Args>
struct Resource {

	Resource(Engine& e, std::initializer_list<const char*> names, Args&&... args)
	: res_names(names)
	, res_handle()
	, data()
	, args(args...)
	, e(e) {
	
	}

	bool load(){
		if(data) return true;
		
		bool loaded = false;
		
		for(auto* n : res_names){
			if(e.res.cache<T, Args...>().get(n, data, args)){
				loaded = true;
				break;
			}
		}

		if(!data){
			for(auto* n : res_names){
				if(ResourceHandle rh = e.res.load(n)){
					create_data(typename gen_seq<sizeof...(Args)>::type());
					loaded = data->loadFromResource(e, rh);
					
					e.res.cache<T, Args...>().put(n, data, args);
					res_handle = std::move(rh);
					break;
				}
			}
		}

		return loaded;
	}

	bool isLoaded(void) const {
		return data.get() != nullptr;
	}

	const T* operator->(void) {
		if(!data) load();
		return data.get();
	}
	
	const T& operator*(void) {
		if(!data) load();
		return *data;
	}

private:

	template<unsigned... S>
	void create_data(seq<S...>){
		data = std::make_shared<T>(std::get<S>(args)...);
	}

	std::vector<const char*> res_names;
	ResourceHandle res_handle;
	std::shared_ptr<T> data;
	std::tuple<Args...> args;
	Engine& e;
};

#endif

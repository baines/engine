#ifndef RESOURCE_IMPL_H_
#define RESOURCE_IMPL_H_
#include "resource.h"

template<class T, class... Args>
Resource<T, Args...>::Resource(Engine& e, std::initializer_list<const char*> names, Args&&... args)
: res_names(names)
, res_handle()
, data()
, args(args...)
, e(e) {

}

template<class T, class... Args>
bool Resource<T, Args...>::load(){
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

template<class T, class... Args>
bool Resource<T, Args...>::isLoaded(void) const {
	return data.get() != nullptr;
}

template<class T, class... Args>
const T* Resource<T, Args...>::operator->(void) {
	if(!data) load();
	return data.get();
}
	
template<class T, class... Args>
const T& Resource<T, Args...>::operator*(void) {
	if(!data) load();
	return *data;
}

#endif


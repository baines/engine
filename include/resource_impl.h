#ifndef RESOURCE_IMPL_H_
#define RESOURCE_IMPL_H_
#include "resource.h"

template<class T, class... Args>
Resource<T, Args...>::Resource(Engine& e, std::initializer_list<const char*> names, Args&&... args)
: res_names(names)
, chosen_name()
, res_handle()
, resource()
, args(args...)
, e(e) {

}

inline void res_error(const std::vector<const char*>& res_names){
	std::string names = "[ ";
	for(auto& i : res_names) names.append(i).append(1, ' ');
	names.append(1, ']');
	log(logging::fatal, "Resource not found: %s", names.c_str());
}

template<class T, class... Args>
Resource<T, Args...>& Resource<T, Args...>::operator=(Resource<T, Args...>&& other){
	std::swap(res_names, other.res_names);
	std::swap(chosen_name, other.chosen_name);
	std::swap(res_handle, other.res_handle);
	std::swap(resource, other.resource);
	std::swap(args, other.args);

	if(!resource && !load()) res_error(res_names);

	return *this;
}

template<class T, class... Args>
bool Resource<T, Args...>::load(){
	if(isLoaded()) return true;
	
	for(auto* n : res_names){
		if(e.res.cache<T, Args...>().get(n, resource, args)){
			return true;
		}
	}

	for(auto* n : res_names){
		if(ResourceHandle rh = e.res.load(n)){
			create_res(std::make_index_sequence<sizeof...(Args)>());
			if(resource->loadFromResource(e, rh)){
				e.res.cache<T, Args...>().put(n, resource, args);
				res_handle = std::move(rh);
				chosen_name = n;
				return true;
			} else {
				delete resource;
				return false;
			}
		}
	}

	return false;
}

template<class T, class... Args>
bool Resource<T, Args...>::isLoaded() const {
	return resource != nullptr;
}

template<class T, class... Args>
bool Resource<T, Args...>::forceReload() {
	if(!resource){
		return load();
	} else {
		return resource->loadFromResource(e, res_handle);
	}
}

template<class T, class... Args>
const T* Resource<T, Args...>::operator->() {
	if(!resource && !load()) res_error(res_names);
	return resource;
}
	
template<class T, class... Args>
const T& Resource<T, Args...>::operator*() {
	if(!resource && !load()) res_error(res_names);
	return *resource;
}

template<class T, class... Args>
T** Resource<T, Args...>::getPtr(){
	if(!resource && !load()) res_error(res_names);
	return &resource;
}

template<class T, class... Args>
Resource<T, Args...>::~Resource(){
	if(resource && chosen_name) e.res.cache<T, Args...>().del(chosen_name, args);
}

#endif


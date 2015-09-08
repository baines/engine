#ifndef RESOURCE_H_
#define RESOURCE_H_
#include "common.h"
#include "engine.h"
#include "resource_system.h"
#include <utility>
#include <vector>
#include <tuple>

struct ResourceBase {
	virtual const void* getRawPtr() = 0;
	virtual ~ResourceBase(){}
};

template<class T, class... Args>
struct Resource : ResourceBase {
	Resource(Engine& e, std::initializer_list<const char*> names, Args&&... args)
	: res_names(names)
	, chosen_name()
	, res_handle()
	, resource()
	, args(std::forward<Args>(args)...)
	, e(e) {

	}

	Resource& operator=(Resource&& other){
		std::swap(res_names, other.res_names);
		std::swap(chosen_name, other.chosen_name);
		std::swap(res_handle, other.res_handle);
		std::swap(resource, other.resource);
		std::swap(args, other.args);

		if(!resource && !load()) res_error(res_names);

		return *this;
	}

	bool load(){
		if(isLoaded()) return true;

		for(auto* n : res_names){
			if(e.res->cache<T, Args...>().get(n, resource, args)){
				return true;
			}
		}

		for(auto* n : res_names){
			if(ResourceHandle rh = e.res->load(n)){
				create_res(std::make_index_sequence<sizeof...(Args)>(), rh);
				e.res->cache<T, Args...>().put(n, resource, args);
				res_handle = std::move(rh);
				chosen_name = n;
				return true;
			}
		}

		return false;
	}

	bool isLoaded() const {
		return resource != nullptr;
	}

	bool forceReload(){
		if(!resource){
			return load();
		} else {
			recreate_res(std::make_index_sequence<sizeof...(Args)>(), res_handle);
			return true;
		}
	}

	const T* operator->(){
		if(!resource && !load()) res_error(res_names);
		return resource;
	}
	const T& operator*(){
		if(!resource && !load()) res_error(res_names);
		return *resource;
	}

	const void* getRawPtr() override {
		if(!resource){
			DEBUGF("Resource '%s' being lazy loaded now...", res_names[0]);
			if(!load()) res_error(res_names);
		}
		return reinterpret_cast<const void*>(resource);
	}

	~Resource(){
		if(resource && chosen_name) e.res->cache<T, Args...>().del(chosen_name, args);
	}
private:

	std::vector<const char*> res_names;
	const char* chosen_name;
	ResourceHandle res_handle;
	T* resource;
	std::tuple<Args...> args;
	Engine& e;

	void res_error(const std::vector<const char*>& res_names){
		std::string names = "[ ";
		for(auto& i : res_names) names.append(i).append(1, ' ');
		names.append(1, ']');
		log(logging::fatal, "Resource not found: %s", names.c_str());
	}

	template<class U>
	struct needs_engine_param {
		template<class V> static std::true_type
			check(decltype(V(e, MemBlock{}, std::declval<Args>()...))*);
		template<class V> static std::false_type check(...);
		static const bool value = decltype(check<U>(nullptr))::value;
	};

	template<class U, class = void>
	struct Wrapper;

	template<class U> 
	struct Wrapper<U, typename std::enable_if<needs_engine_param<U>::value>::type> : public U{
		Wrapper(Engine& e, Resource* res, const ResourceHandle& rh, Args... args)
		: T(e, MemBlock{ rh.data(), rh.size() }, std::forward<Args>(args)...)
		, res(res){}

		virtual void onGLContextRecreate() {
			res->forceReload();
		}
		Resource* res;
	};

	template<class U>
	struct Wrapper<U, typename std::enable_if<!needs_engine_param<U>::value>::type> : public U{
		Wrapper(Engine& e, Resource* res, const ResourceHandle& rh, Args... args)
		: T(MemBlock { rh.data(), rh.size() }, std::forward<Args>(args)...)
		, res(res){}

		virtual void onGLContextRecreate() {
			res->forceReload();
		}
		Resource* res;
	};
	
	template<size_t... I>
	void create_res(std::index_sequence<I...>, const ResourceHandle& rh){
		resource = new Wrapper<T>(e, this, rh, std::get<I>(args)...);
	}
	
	template<size_t... I>
	void recreate_res(std::index_sequence<I...>, const ResourceHandle& rh){
		resource->~T();
		new (resource) Wrapper<T>(e, this, rh, std::get<I>(args)...);
	}

};

#endif

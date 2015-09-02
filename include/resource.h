#ifndef RESOURCE_H_
#define RESOURCE_H_
#include "common.h"
#include "resource_system.h"
#include <utility>
#include <vector>
#include <tuple>

struct ResourceBase {
	virtual void* getRawPtr() = 0;
	virtual ~ResourceBase(){}
};

template<class T, class... Args>
struct Resource : ResourceBase {
	Resource(Engine& e, std::initializer_list<const char*> names, Args&&... args);
	Resource& operator=(Resource&& other);

	bool load();
	bool isLoaded() const;
	bool forceReload();

	const T* operator->();
	const T& operator*();

	void* getRawPtr() override;

	~Resource();	
private:

	std::vector<const char*> res_names;
	const char* chosen_name;
	ResourceHandle res_handle;
	T* resource;
	std::tuple<Args...> args;
	Engine& e;

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

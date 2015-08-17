#ifndef RESOURCE_H_
#define RESOURCE_H_
#include "common.h"
#include "resource_system.h"
#include <utility>
#include <vector>
#include <tuple>

template<class T, class... Args>
struct Resource {
	Resource(Engine& e, std::initializer_list<const char*> names, Args&&... args);
	Resource& operator=(Resource&& other);

	bool load();
	bool isLoaded() const;
	bool forceReload();

	const T* operator->();
	const T& operator*();

	T** getPtr();

	~Resource();	
private:
	struct Wrapper : public T {
		Wrapper(Resource* res, Args... args) : T(std::forward<Args>(args)...), res(res){}
		virtual void onGLContextRecreate() {
			res->forceReload();
		}
		Resource* res;
	};

	template<size_t... I>
	void create_res(std::index_sequence<I...>){
		resource = NullOnMovePtr<T>(new Wrapper(this, std::get<I>(args)...));
	}

	std::vector<const char*> res_names;
	const char* chosen_name;
	ResourceHandle res_handle;
	T* resource;
	std::tuple<Args...> args;
	Engine& e;
};

#endif

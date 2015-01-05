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
	const std::shared_ptr<T>& operator*();
	
private:
	friend struct GLRes;
	struct GLRes : public T {
		GLRes(Resource* res, Args... args) : T(std::forward<Args>(args)...), res(res){}
		virtual void onGLContextRecreate() {
			res->forceReload();
		}
		Resource* res;
	};

	template<unsigned... S>
	void create_data(seq<S...>){
		data = std::make_shared<GLRes>(this, std::get<S>(args)...);
	}

	std::vector<const char*> res_names;
	ResourceHandle res_handle;
	std::shared_ptr<T> data;
	std::tuple<Args...> args;
	Engine& e;
};

#endif

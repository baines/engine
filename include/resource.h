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
	bool isLoaded(void) const;

	const T* operator->(void);
	const std::shared_ptr<T>& operator*(void);
	
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

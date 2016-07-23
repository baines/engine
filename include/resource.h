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
	Resource(Engine& e, std::initializer_list<const char*> names, Args&&... args){
		auto h = e.res->load(*names.begin());
		ptr = new T(MemBlock{ h.data, h.size }, args...);
	}

	Resource& operator=(Resource&& other){
		if(ptr) delete ptr;
		ptr = other.ptr;
		other.ptr = nullptr;
		return *this;
	}

	bool load(){ return true; }

	bool isLoaded() const{ return true;} 

	bool forceReload(){return true;}

	const T* operator->(){
		return ptr;
	}

	const T& operator*(){
		return *ptr;
	}

	const void* getRawPtr() override {
		return ptr;
	}

	~Resource(){
	}

private:
	T* ptr;

};

#endif

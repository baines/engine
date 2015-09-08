#ifndef PROXY_H_
#define PROXY_H_
#include <memory>
#include "resource.h"

template<class T>
struct Proxy {
	Proxy() : ptr(nullptr), type(RAW){}
	Proxy(T*& ptr) : ptr(&ptr), type(RAW){}
	template<class... Args>
	Proxy(Resource<T, Args...>& res) : ptr(&res), type(RES){}
	Proxy(std::shared_ptr<T>& sptr) : ptr(&sptr), type(SHARED){}

	const T& operator* () const{
		return type == RAW 
			? **reinterpret_cast<T**>(ptr)
			: type == RES 
			? *reinterpret_cast<const T*>(reinterpret_cast<ResourceBase*>(ptr)->getRawPtr()) 
			: **reinterpret_cast<std::shared_ptr<T>*>(ptr);
	}
	const T* operator->() const { return &(*(*this)); }
	
	operator bool() const {
		return ptr != nullptr;
	}
private:
	void* ptr;
	enum { RAW, SHARED, RES } type;
};

#endif

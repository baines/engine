#ifndef PROXY_H_
#define PROXY_H_
#include "resource.h"

template<class T>
struct Proxy {
	Proxy() : ptr(nullptr), type(RAW){}
	Proxy(T*& ptr) : ptr(&ptr), type(RAW){}
	Proxy(ResourceBase& res) : ptr(&res), type(RES){}

	const T& operator* () const{
		return type == RAW 
			? **reinterpret_cast<T**>(ptr)
			: *reinterpret_cast<const T*>(reinterpret_cast<ResourceBase*>(ptr)->getRawPtr());
	}
	const T* operator->() const { return &(*(*this)); }
	
	operator bool() const {
		return ptr != nullptr;
	}
private:
	void* ptr;
	enum { RAW, RES } type;
};

#endif

#ifndef PROXY_H_
#define PROXY_H_
#include <memory>
#include "resource.h"

template<class T>
struct Proxy {
	Proxy() : ptr(nullptr), type(RAW){}
	Proxy(T*& ptr) : ptr(&ptr), type(RAW){}
	template<class... Args>
	Proxy(Resource<T, Args...>& res) : ptr(res.getPtr()), type(RAW){}
	Proxy(std::shared_ptr<T>& sptr) : ptr(&sptr), type(SHARED){}

	const T& operator* () const{
		return type == RAW ?
			**reinterpret_cast<T**>(ptr) :
			**reinterpret_cast<std::shared_ptr<T>*>(ptr);

	}
	const T* operator->() const {
		return type == RAW ? 
			*reinterpret_cast<T**>(ptr) :
			reinterpret_cast<std::shared_ptr<T>*>(ptr)->get();
	}

	T& operator* (){
		return type == RAW ?
			**reinterpret_cast<T**>(ptr) :
			**reinterpret_cast<std::shared_ptr<T>*>(ptr);

	}
	T* operator->(){
		return type == RAW ? 
			*reinterpret_cast<T**>(ptr) :
			reinterpret_cast<std::shared_ptr<T>*>(ptr)->get();
	}

	operator bool() const {
		return ptr != nullptr;
	}
private:
	void* ptr;
	enum { RAW, SHARED } type;
};

#endif

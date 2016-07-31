#ifndef ALT_UTIL_H
#define ALT_UTIL_H
#include <cstdio>
#include <utility>

namespace alt {

template<class T>
struct UniquePtr {
	UniquePtr() : ptr(nullptr){};
	UniquePtr(T* p) : ptr(p){}

	UniquePtr(const UniquePtr&) = delete;
	UniquePtr(UniquePtr&& other) : ptr(nullptr) {
		*this = std::move(other);
	}

	UniquePtr& operator=(const UniquePtr&) = delete;
	UniquePtr& operator=(UniquePtr&& other){
		release();
		ptr = other.ptr;
		other.ptr = nullptr;
		return *this;
	}

	T* get(){ return ptr; }
	const T* get() const { return ptr; }

	T& operator*(){ return *ptr; }
	const T& operator*() const { return *ptr; }

	T* operator->(){ return ptr; }
	const T* operator->() const { return ptr; }

	void release(){	if(ptr) delete ptr;	}
	~UniquePtr(){ release(); }
private:
	T* ptr;
};

template<class> struct Closure{};

template<typename R, typename... Args>
struct Closure<R(Args...)> {
	Closure() = default;

	template<class C>
	Closure(C&& c) : closure(new ClosureInner<C>{std::forward<C>(c)}){}
	R operator()(Args... args){
		return closure->call(args...);
	}

	struct ClosureInnerBase {
		virtual R call(Args...) = 0;
		virtual ~ClosureInnerBase(){};
	};

	template<class C>
	struct ClosureInner : Closure<R(Args...)>::ClosureInnerBase {
		ClosureInner(C&& c) : closure(std::forward<C>(c)){}
		C closure;
		R call(Args... args) override { return closure(args...); }
	};

	UniquePtr<ClosureInnerBase> closure;
};


}

#endif

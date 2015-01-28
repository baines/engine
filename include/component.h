#ifndef COMPONENT_H_
#define COMPONENT_H_
#include "common.h"
#include <type_traits>

unsigned getNextComponentID();

template<class T>
class has_base_component {
	template<class C> struct S {};
	template<class X> static std::true_type check(S<typename X::BaseComponent>*);
	template<class X> static std::false_type check(...);
public:
	static const bool value = decltype(check<T>(0))::value;
};

template<class T, class = void>
struct Component {
	static unsigned getID(){
		static unsigned id = getNextComponentID();
		return id;
	}
	static bool hasID(unsigned id){
		return id == getID();
	}
};

template<class T>
struct Component<T, typename std::enable_if<has_base_component<T>::value>::type> {
	static unsigned getID(){
		static unsigned id = getNextComponentID();
		return id;
	}
	static bool hasID(unsigned id){
		return id == getID() || Component<typename T::BaseComponent>::hasID(id);
	}
};

#endif
#ifndef COMPONENT_H_
#define COMPONENT_H_
#include "common.h"
#include "glm/glm.hpp"
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

/* probably should be somewhere else */
struct Position2D {
	Position2D(glm::vec2 p);
	void initComponent(Engine&, Entity&);

	glm::vec2 get() const;
	void      set(glm::vec2 pos);
	void      add(glm::vec2 pos);
private:
	glm::vec2 pos;
	Entity* entity;
};

#endif

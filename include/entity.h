#ifndef ENTITY_H_
#define ENTITY_H_
#include "common.h"
#include "component.h"
struct Entity {

	template<class T>
	T* get(){
		void* ptr = getComponentByID(Component<T>::getID());
		if(ptr){
			return reinterpret_cast<T*>(ptr);
		} else {
			return nullptr;
		}
	}
	
	virtual ~Entity(){}

protected:
	virtual void* getComponentByID(unsigned id) = 0;
};

template<class T>
class has_component_init {
	template<class C, C> struct S {};
	template<class X> static std::true_type check(
		S<void (X::*)(Engine&, Entity&), &X::initComponent>*
	);
	template<class X> static std::false_type check(...);
public:
	static const bool value = decltype(check<T>(0))::value;
};

template<class... Components>
struct EntityWith : public Entity {

	EntityWith(Engine& e, Components&&... cs) : components(std::forward<Components>(cs)...){
		initComponents<0>(e);
	}

	template<class... Cs>
	EntityWith(Engine& e, Cs&&... cs) : components(std::forward<Cs>(cs)...){
		initComponents<0>(e);
	}
	
	template<class T>
	T& get(){
		//TODO: static_assert for better error message.
		return std::get<T>(components);
	}

	virtual ~EntityWith(){}
private:
	std::tuple<Components...> components;
	
	static const size_t SZ = sizeof...(Components) - 1;

	template<size_t N>
	typename std::enable_if<N == SZ && has_component_init<
		typename std::tuple_element<N, decltype(components)>::type
	>::value>::type initComponents(Engine& e){
		std::get<N>(components).initComponent(e, *this);
	}
	
	template<size_t N>
	typename std::enable_if<N == SZ && !has_component_init<
		typename std::tuple_element<N, decltype(components)>::type
	>::value>::type initComponents(Engine&){

	}

	template<size_t N>
	typename std::enable_if<N < SZ && has_component_init<
		typename std::tuple_element<N, decltype(components)>::type
	>::value>::type	initComponents(Engine& e){
		std::get<N>(components).initComponent(e, *this);
		initComponents<N+1>(e);
	}
	
	template<size_t N>
	typename std::enable_if<N < SZ && !has_component_init<
		typename std::tuple_element<N, decltype(components)>::type
	>::value>::type	initComponents(Engine& e){
		initComponents<N+1>(e);
	}

	template<size_t N>
	typename std::enable_if<N == sizeof...(Components), void*>::type getComponent(unsigned id) {
		return nullptr;
	}
	
	template<size_t N>
	typename std::enable_if<N < sizeof...(Components), void*>::type getComponent(unsigned id) {
		return Component<typename std::tuple_element<N, decltype(components)>::type>::hasID(id)
		     ? reinterpret_cast<void*>(&std::get<N>(components))
		     : getComponent<N+1>(id)
		     ;
	}

protected:
	void* getComponentByID(unsigned id) override {
		return getComponent<0>(id);
	}
};

#endif

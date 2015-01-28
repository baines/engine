#ifndef ENTITY_H_
#define ENTITY_H_
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
	
protected:
	virtual void* getComponentByID(unsigned id) = 0;
};

template<class... Components>
struct EntityWith : public Entity {

	EntityWith(Components... cs) : components(cs...){}
	
	template<class T>
	T& get(){
		//TODO: static_assert for better error message.
		return std::get<T>(components);
	}

protected:
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

	void* getComponentByID(unsigned id) override {
		return getComponent<0>(id);
	}
private:
	std::tuple<Components...> components;
};

#endif

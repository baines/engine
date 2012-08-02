#ifndef _VEC_H_
#define _VEC_H_
#include <cstdlib>
#include <cstring>
#include <stdint.h>

template<class T> class Vec {
public:
	Vec() : elements(NULL), tail(1), v_size(32){
		init(32);
	}
	explicit Vec(int size) : elements(NULL), tail(1), v_size(size){
		init(size);
	}
	~Vec(){
		free(elements);
	}
	int push(const T& t){
		int tmptail = elements[tail].next_tail;
		elements[tail].data = t;
		int r = tail;
		tail = (tmptail == 0) ? tail + 1 : tmptail;
		if(tail == v_size) resize(v_size * 2);
		return r;
	}
	void pop(int index){
		elements[index].next_tail = tail;
		tail = index;
	}
	int size() const {
		return v_size;
	}
	T& operator[](int index){
		return elements[index].data;
	}
private:
	Vec(const Vec& v){};
	Vec& operator=(const Vec& v){ return v; }
	void init(int size){
		tail = 1;
		v_size = size;
		elements = (item*)malloc(size * sizeof(item));
		memset(elements, 0, size * sizeof(item));
	}
	void resize(int newsize){
		elements = (item*)realloc(elements, newsize * sizeof(item));
		memset(elements + v_size, 0, (newsize - v_size) * sizeof(item));
		v_size = newsize;
	}
	typedef union {
		T data;
		uint32_t next_tail;
	} item;
	item* elements;
	uint32_t tail;
	size_t v_size;
};

#endif

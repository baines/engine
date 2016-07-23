#ifndef ALT_ARRAY_H_
#define ALT_ARRAY_H_

#ifndef ALT_ARRAY_MEMCPY
	#include <string.h>
	#define ALT_ARRAY_MEMCPY memcpy
#endif

namespace alt {

template<class T, size_t N, class = void>
struct Array {

	constexpr const T& operator[](size_t n) const {
		return _data[n];
	}

	T& operator[](size_t n){
		return _data[n];
	}
	
	T* begin(){ return _data; }
	T* end(){ return _data + N; }
	
	const T* begin() const { return _data; }
	const T* end() const { return _data + N; }
	
	bool operator==(const Array& other) const {
		return memcmp(_data, other._data, sizeof(_data)) == 0;
	}
	
	bool operator!=(const Array& other) const {
		return memcmp(_data, other._data, sizeof(_data)) != 0;
	}
	
	bool operator<(const Array& other) const {
		return memcmp(_data, other._data, sizeof(_data)) < 0;
	}
	
	bool operator>(const Array& other) const {
		return memcmp(_data, other._data, sizeof(_data)) > 0;
	}
	
	constexpr size_t size(){ return N; }

	T* data(){ return _data; }
	const T* data() const { return _data; }

	T _data[N];
};

}

#endif

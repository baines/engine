template<class T>
struct ALT_VTYPE {
	typedef T            value_type;
	typedef ALT_VTYPE<T> alt_vector_type;
	
	static constexpr size_t size = ALT_VSIZE;
	
	union {
		T data[ALT_VSIZE];
		ALT_VDATA
	};

	constexpr size_t count(){ return size; };
	constexpr size_t len(){ return size; };

	T&       operator[](size_t i)       { return data[i]; }
	const T& operator[](size_t i) const { return data[i]; }

	const T* begin(){ return data; }
	const T* end(){ return data + size; }

	inline bool operator==(const ALT_VTYPE& o) const {
		return memcmp(data, o.data, sizeof(data)) == 0;
	}
	inline bool operator!=(const ALT_VTYPE& o) const {
		return memcmp(data, o.data, sizeof(data)) != 0;
	}

	inline ALT_VTYPE& operator+=(const ALT_VTYPE& v){
		for(size_t i=0;i<size;++i) data[i]+=v[i]; return *this;
	}
	inline ALT_VTYPE& operator-=(const ALT_VTYPE& v){
		for(size_t i=0;i<size;++i) data[i]-=v[i]; return *this;
	}
	
	inline ALT_VTYPE& operator+=(T s){ for(size_t i=0;i<size;++i) data[i]+=s; return *this; }
	inline ALT_VTYPE& operator-=(T s){ for(size_t i=0;i<size;++i) data[i]+=s; return *this; }
	inline ALT_VTYPE& operator*=(T s){ for(size_t i=0;i<size;++i) data[i]*=s; return *this; }
	inline ALT_VTYPE& operator/=(T s){ for(size_t i=0;i<size;++i) data[i]/=s; return *this; }
	
	inline friend ALT_VTYPE operator+(ALT_VTYPE v1, const ALT_VTYPE& v2){ v1 += v2; return v1; }
	inline friend ALT_VTYPE operator-(ALT_VTYPE v1, const ALT_VTYPE& v2){ v1 -= v2; return v1; }
	
	inline friend ALT_VTYPE operator+(ALT_VTYPE v, T s){ v += s; return v; }
	inline friend ALT_VTYPE operator-(ALT_VTYPE v, T s){ v -= s; return v; }
	inline friend ALT_VTYPE operator*(ALT_VTYPE v, T s){ v *= s; return v; }
	inline friend ALT_VTYPE operator/(ALT_VTYPE v, T s){ v /= s; return v; }
};

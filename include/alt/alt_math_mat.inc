template<class T>
struct ALT_MTYPE {
	typedef T               value_type;
	typedef ALT_MTYPE<T>    alt_matrix_type;
	typedef ALT_MCOLTYPE<T> col_type;

	static constexpr size_t col_size = col_type::size;
	static constexpr size_t row_size = ALT_MROWS;
	static constexpr size_t size = col_size * row_size;
	
	union {
		T data[size];
		col_type col[row_size];
	};

	constexpr size_t count(){ return size; };
	constexpr size_t len(){ return size; };

	constexpr size_t col_count(){ return col_size; };
	constexpr size_t row_count(){ return row_size; };

	col_type&       operator[](size_t i)       { return col[i]; }
	const col_type& operator[](size_t i) const { return col[i]; }

	const T* begin(){ return data; }
	const T* end(){ return data + size; }

	inline bool operator==(const ALT_MTYPE& o) const {
		return memcmp(data, o.data, sizeof(data)) == 0;
	}
	inline bool operator!=(const ALT_MTYPE& o) const {
		return memcmp(data, o.data, sizeof(data)) != 0;
	}

	inline ALT_MTYPE& operator+=(const ALT_MTYPE& m){
		for(size_t i=0;i<size;++i) data[i]+=m[i]; return *this;
	}
	inline ALT_MTYPE& operator-=(const ALT_MTYPE& m){
		for(size_t i=0;i<size;++i) data[i]-=m[i]; return *this;
	}
	
	inline ALT_MTYPE& operator+=(T s){ for(size_t i=0;i<size;++i) data[i]+=s; return *this; }
	inline ALT_MTYPE& operator-=(T s){ for(size_t i=0;i<size;++i) data[i]+=s; return *this; }
	inline ALT_MTYPE& operator*=(T s){ for(size_t i=0;i<size;++i) data[i]*=s; return *this; }
	inline ALT_MTYPE& operator/=(T s){ for(size_t i=0;i<size;++i) data[i]/=s; return *this; }
	
	inline friend ALT_MTYPE operator+(ALT_MTYPE m1, const ALT_MTYPE& m2){ m1 += m2; return m1; }
	inline friend ALT_MTYPE operator-(ALT_MTYPE m1, const ALT_MTYPE& m2){ m1 -= m2; return m1; }
	
	inline friend ALT_MTYPE operator+(ALT_MTYPE m, T s){ m += s; return m; }
	inline friend ALT_MTYPE operator-(ALT_MTYPE m, T s){ m -= s; return m; }
	inline friend ALT_MTYPE operator*(ALT_MTYPE m, T s){ m *= s; return m; }
	inline friend ALT_MTYPE operator/(ALT_MTYPE m, T s){ m /= s; return m; }

	//TODO: non-square operators, mat-mat mult
};

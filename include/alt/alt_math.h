#ifndef ALT_MATH_H_
#define ALT_MATH_H_
#include <cstddef>
#include <cmath>

namespace alt {

/****************************************
         Vector type definitions
****************************************/

#define ALT_VSIZE 2
#define ALT_VTYPE vec2t
#define ALT_VDATA struct { T x, y; }; struct { T r, g; };
#include "alt_math_vec.inc"
#undef ALT_VSIZE
#undef ALT_VTYPE
#undef ALT_VDATA

#define ALT_VSIZE 3
#define ALT_VTYPE vec3t
#define ALT_VDATA struct { T x, y, z; }; struct { T r, g, b; };
#include "alt_math_vec.inc"
#undef ALT_VSIZE
#undef ALT_VTYPE
#undef ALT_VDATA

#define ALT_VSIZE 4
#define ALT_VTYPE vec4t
#define ALT_VDATA struct { T x, y, z, w; }; struct { T r, g, b, a; };
#include "alt_math_vec.inc"
#undef ALT_VSIZE
#undef ALT_VTYPE
#undef ALT_VDATA

/****************************************
           Vector type aliases
****************************************/

using vec2  = vec2t<float>;
using vec2f = vec2t<float>;
using vec2i = vec2t<int>;
using vec2d = vec2t<double>;

using vec3  = vec3t<float>;
using vec3f = vec3t<float>;
using vec3i = vec3t<int>;
using vec3d = vec3t<double>;

using vec4  = vec4t<float>;
using vec4f = vec4t<float>;
using vec4i = vec4t<int>;
using vec4d = vec4t<double>;

/****************************************
         Matrix type definitions
****************************************/

// TODO: non-square matrices

#define ALT_MROWS 2
#define ALT_MTYPE mat2t
#define ALT_MCOLTYPE vec2t
#include "alt_math_mat.inc"
#undef ALT_MROWS
#undef ALT_MCOLTYPE
#undef ALT_MTYPE

#define ALT_MROWS 3
#define ALT_MTYPE mat3t
#define ALT_MCOLTYPE vec3t
#include "alt_math_mat.inc"
#undef ALT_MROWS
#undef ALT_MCOLTYPE
#undef ALT_MTYPE

#define ALT_MROWS 4
#define ALT_MTYPE mat4t
#define ALT_MCOLTYPE vec4t
#include "alt_math_mat.inc"
#undef ALT_MROWS
#undef ALT_MCOLTYPE
#undef ALT_MTYPE

/****************************************
            Matrix type aliases
****************************************/

using mat2  = mat2t<float>;
using mat2f = mat2t<float>;
using mat2i = mat2t<int>;
using mat2d = mat2t<double>;

using mat3  = mat3t<float>;
using mat3f = mat3t<float>;
using mat3i = mat3t<int>;
using mat3d = mat3t<double>;

using mat4  = mat4t<float>;
using mat4f = mat4t<float>;
using mat4i = mat4t<int>;
using mat4d = mat4t<double>;

/****************************************
              Functions
****************************************/

template<class V>
inline typename V::value_type length(const V& v){
	typename V::value_type ret{};
	for(size_t i = 0; i < V::size; ++i){
		ret += (v[i] * v[i]);
	}
	return sqrt(ret);
}

template<class V>
inline V normalize(V v){
	typename V::value_type l = length(v);
	for(size_t i = 0; i < V::size; ++i){
		v[i] /= l;
	}
	return v;
}

template<class V>
inline typename V::value_type dot(const V& a, const V& b){
	typename V::value_type ret{};
	for(size_t i = 0; i < V::size; ++i){
		ret += a[i] * b[i];
	}
	return ret;
}

inline mat4 ortho(float l, float r, float b, float t, float n, float f){
	mat4 m = {};

	m[0][0] =  2.0f / (r - l);
	m[1][1] =  2.0f / (t - b);
	m[2][2] = -2.0f / (f - n);
	m[3][3] =  1.0f;

	m[3][0] = -(r + l) / (r - l);
	m[3][1] = -(t + b) / (t - b);
	m[3][2] = -(f + n) / (f - n);

	return m;
}

inline mat4 frustum(float l, float r, float b, float t, float n, float f){
	mat4 m = {};

	m[0][0] = (2.0f * n) / (r - l);
	m[1][1] = (2.0f * n) / (t - b);

	m[2][0] =  (r + l) / (r - l);
	m[2][1] =  (t + b) / (t - b);
	m[2][2] = -(f + n) / (f - n);
	m[2][3] = -1.0f;

	m[3][2] = (-2.0f * n * f) / (f - n);

	return m;
}

template<class V>
V swizzle(const V& vec, const char (&str)[V::size + 1]){
	V ret;

	for(size_t i = 0; i < V::size; ++i){
		size_t j = 0;

		switch(str[i]){
			case 'y':
			case 'g':
				j = 1;
				break;
			case 'z':
			case 'b':
				j = 2;
				break;
			case 'w':
			case 'a':
				j = 3;
				break;
		}

		ret[i] = vec[j];
	}

	return ret;
}

}
#endif

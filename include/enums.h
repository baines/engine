#ifndef ENUMS_H_
#define ENUMS_H_
#include <utility>

#define FE_1(WHAT, X) WHAT(X) 
#define FE_2(WHAT, X, ...) WHAT(X),FE_1(WHAT, __VA_ARGS__)
#define FE_3(WHAT, X, ...) WHAT(X),FE_2(WHAT, __VA_ARGS__)
#define FE_4(WHAT, X, ...) WHAT(X),FE_3(WHAT, __VA_ARGS__)
#define FE_5(WHAT, X, ...) WHAT(X),FE_4(WHAT, __VA_ARGS__)

#define GET_MACRO(_1,_2,_3,_4,_5,NAME,...) NAME 
#define FOR_EACH(action,...) GET_MACRO(__VA_ARGS__,FE_5,FE_4,FE_3,FE_2,FE_1)(action,__VA_ARGS__)

template<class T, class... Args>
constexpr typename alt::Array<T, sizeof...(Args)> make_array(Args&&... args){
	return {{ T(std::forward<Args>(args))... }};
}

template<class... Args>
constexpr typename alt::Array<str_const, sizeof...(Args)> make_enum(Args&&... args){
	return make_array<str_const>(std::forward<Args>(args)...);
}

#define MAKE_ENUM(name, ...) \
	constexpr str_const FOR_EACH(DEF_STRS_FN, __VA_ARGS__); \
	constexpr auto name = make_enum( FOR_EACH(GEN_ENUM_FN, __VA_ARGS__) );

#define DEF_STRS_FN(x) x(STRINGIFY(x))
#define GEN_ENUM_FN(x) STRINGIFY(x) 

MAKE_ENUM(
	gl_streaming_enum,

	BUFFER_INVALIDATE,
	BUFFER_DATA_NULL,
	MAP_INVALIDATE,
	DOUBLE_BUFFER,
	MAP_UNSYNC_APPEND
);

#endif


#ifndef ENUMS_H_
#define ENUMS_H_
#include <boost/preprocessor.hpp>
#include "util.h"

#define ENUM_TOSTR(x) #x

#define MAKE_ENUM(name, args) \
	constexpr str_const BOOST_PP_SEQ_FOR_EACH_I(DEF_STRS_FN, _, args); \
	constexpr auto name = make_enum( BOOST_PP_SEQ_FOR_EACH_I(GEN_ENUM_FN, _, args) )

#define DEF_STRS_FN(r, data, i, elem) \
	BOOST_PP_COMMA_IF(BOOST_PP_NOT_EQUAL(i, 0)) elem(ENUM_TOSTR(elem))
	
#define GEN_ENUM_FN(r, data, i, elem) \
	BOOST_PP_COMMA_IF(BOOST_PP_NOT_EQUAL(i, 0)) ENUM_TOSTR(elem)
	
MAKE_ENUM(gl_orphan_enum, (BUFFER_INVALIDATE)(BUFFER_DATA_NULL)(MAP_INVALIDATE));

#endif

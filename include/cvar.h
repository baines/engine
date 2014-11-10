#ifndef CVAR_H_
#define CVAR_H_
#include <string>
#include <vector>
#include "util.h"

enum CVarType {
	CVAR_INVALID,
	CVAR_INT,
	CVAR_FLOAT,
	CVAR_STRING,
	CVAR_ENUM,
	CVAR_BOOL,
	CVAR_FUNC
};

template<class T>
struct CVarNumeric {
	CVarNumeric() : CVarNumeric(0, 0, 0){}
	CVarNumeric(T init, T min, T max) : val(init), min(min), max(max){}

	bool set(T v){
		if(v >= min && v <= max){
			val = v;
			return true;
		} else {
			return false;
		}
	}

	T val, min, max;
};

typedef CVarNumeric<int>   CVarInt;
typedef CVarNumeric<float> CVarFloat;

struct CVarString {
	CVarString(const char* str) : str(str){}
	bool set(std::string&& s){
		s = std::move(str);
		return true;
	}
	std::string str;
};

struct CVarEnum {
	template<size_t N>
	CVarEnum(const std::array<str_const, N>& strs, size_t index)
	: strs(strs.begin(), strs.end())
	, index(index){

	}

	bool set(const str_const& s){
		size_t i = 0;
		for(const auto& str : strs){
			if(str == s){
				index = i;
				return true;
			}
			++i;
		}
		return false;
	}

	std::vector<str_const> strs;
	size_t index;
};

struct CVarBool {
	CVarBool(bool b) : val(b){}
	bool set(bool b){
		val = b;
		return true;
	}
	bool val;
};

template<class T> struct cvar_id {};
template<> struct cvar_id<CVarInt>    { static const int value = CVAR_INT; };
template<> struct cvar_id<CVarFloat>  { static const int value = CVAR_FLOAT; };
template<> struct cvar_id<CVarString> { static const int value = CVAR_STRING; };
template<> struct cvar_id<CVarEnum>   { static const int value = CVAR_ENUM; };
template<> struct cvar_id<CVarBool>   { static const int value = CVAR_BOOL; };

struct CVar {
	template<class Var>
	CVar(const char* name, Var&& v)
	: type(cvar_id<Var>::value)
	, name(name)
	, value(){
		new(&value) Var(std::move(v));
	}

	template<class T>
	T* get(void) {
		if(type == cvar_id<T>::value){
			return reinterpret_cast<T*>(&value);
		} else {
			return nullptr;
		}
	}

	~CVar(){
		switch(type){
			case CVAR_INT:    reinterpret_cast<CVarInt*>(&value)->~CVarInt();       break;
			case CVAR_FLOAT:  reinterpret_cast<CVarFloat*>(&value)->~CVarFloat();   break;
			case CVAR_STRING: reinterpret_cast<CVarString*>(&value)->~CVarString(); break;
			case CVAR_ENUM:   reinterpret_cast<CVarEnum*>(&value)->~CVarEnum();     break;
			case CVAR_BOOL:   reinterpret_cast<CVarBool*>(&value)->~CVarBool();     break;
		}
	}
	
	const int type;
	const std::string name;
	variant<CVarInt, CVarFloat, CVarString, CVarEnum, CVarBool>::type value;
};

#endif

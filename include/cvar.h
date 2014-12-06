#ifndef CVAR_H_
#define CVAR_H_
#include "common.h"
#include <string>
#include <vector>
#include <cstdlib>
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
	bool set(const char* s, size_t len = 0){
		if(!s) return false;
		if(len){
			str.assign(s, len);
		} else {
			str.assign(s);
		}
		return true;
	}
	bool set(std::string&& s){
		str = std::move(s);
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
		return set(s.hash);
	}

	bool set(uint32_t hash){
		size_t i = 0;
		for(const auto& str : strs){
			if(str.hash == hash){
				index = i;
				return true;
			}
			++i;
		}
		return false;
	}
	
	const str_const& get(){
		return strs[index];
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

struct CVarFunc {
	template<class F>
	CVarFunc(F&& fn) : func(std::forward<F>(fn)){}
	void call(const char* s, size_t len){
		if(!s) return;
		func(len ? std::string(s, len) : std::string(s));
	}
	std::function<void(std::string&&)> func;
};

template<class T> struct cvar_id {};
template<> struct cvar_id<CVarInt>    { static const int value = CVAR_INT; };
template<> struct cvar_id<CVarFloat>  { static const int value = CVAR_FLOAT; };
template<> struct cvar_id<CVarString> { static const int value = CVAR_STRING; };
template<> struct cvar_id<CVarEnum>   { static const int value = CVAR_ENUM; };
template<> struct cvar_id<CVarBool>   { static const int value = CVAR_BOOL; };
template<> struct cvar_id<CVarFunc>   { static const int value = CVAR_FUNC; };

struct CVar {
	template<class Var>
	CVar(const str_const& name, Var&& v)
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
	
	void eval(const char* val, size_t len = 0){
		if(!val && type != CVAR_FUNC) return;
		
		switch(type){
			case CVAR_INT:    get<CVarInt>()->set(strtol(val, nullptr, 0)); break;
			case CVAR_FLOAT:  get<CVarFloat>()->set(strtof(val, nullptr)); break;
			case CVAR_STRING: get<CVarString>()->set(val, len); break;
			case CVAR_ENUM: {
				uint32_t hash = len ? str_hash_len(val, len) : str_hash(val);
				get<CVarEnum>()->set(hash);
				break;
			}
			case CVAR_BOOL:   get<CVarBool>()->set(str_to_bool(val)); break;
			case CVAR_FUNC:   get<CVarFunc>()->call(val, len); break;
		}
	}

	~CVar(){
		switch(type){
			case CVAR_INT:    get<CVarInt>()->~CVarInt();       break;
			case CVAR_FLOAT:  get<CVarFloat>()->~CVarFloat();   break;
			case CVAR_STRING: get<CVarString>()->~CVarString(); break;
			case CVAR_ENUM:   get<CVarEnum>()->~CVarEnum();     break;
			case CVAR_BOOL:   get<CVarBool>()->~CVarBool();     break;
			case CVAR_FUNC:   get<CVarFunc>()->~CVarFunc();     break;
		}
	}
	
	const int type;
	const str_const name;
	variant<CVarInt, CVarFloat, CVarString, CVarEnum, CVarBool, CVarFunc>::type value;
};

#endif

#ifndef CVAR_H_
#define CVAR_H_
#include "common.h"
#include "util.h"
#include <vector>
#include <functional>

enum CVarType {
	CVAR_INVALID,
	CVAR_INT,
	CVAR_FLOAT,
	CVAR_STRING,
	CVAR_ENUM,
	CVAR_BOOL,
	CVAR_FUNC
};

template<class T> struct cvar_id {};

struct CVar {
	template<class Var>
	CVar(const str_const& name, const Var* v)
	: type(cvar_id<Var>::value)
	, name(name)
   	, reload_var(nullptr) {

	}
	
	template<class T>
	typename std::enable_if<!std::is_same<T, CVar>::value, T*>::type 
	get(void) {
		if(type == cvar_id<typename std::remove_const<T>::type>::value){
			return reinterpret_cast<T*>(this);
		} else {
			return nullptr;
		}
	}

	template<class T>
	typename std::enable_if<!std::is_same<T, CVar>::value, const T*>::type 
	get(void) const {
		if(type == cvar_id<typename std::remove_const<T>::type>::value){
			return reinterpret_cast<const T*>(this);
		} else {
			return nullptr;
		}
	}

	template<class T>
	typename std::enable_if<std::is_same<T, CVar>::value, T*>::type
	get(void) {
		return this;
	}

	template<class T>
	typename std::enable_if<std::is_same<T, CVar>::value, const T*>::type
	get(void) const {
		return this;
	}
	
	virtual bool eval(const StrRef& val) = 0;

	virtual void printInfo(ICLI& cli) const = 0;
	
	void setReloadVar(const char* str){
		reload_var = str;
	}
	
	const char* getReloadVar() const {
		return reload_var;
	}

	virtual const char* getErrorString() const {
		return "Value out of range.";
	}

	virtual ~CVar(){}

	const int type;
	const str_const name;
	const char* reload_var;
};

template<class T>
struct CVarNumeric : public CVar {
	CVarNumeric(const str_const& name) : CVarNumeric(0, 0, 0){}
	CVarNumeric(const str_const& name, T init, T min, T max)
	: CVar(name, this), init(init), val(init), min(min), max(max){}

	bool eval(const StrRef& s);
	void printInfo(ICLI& cli) const;

	bool set(T v){
		if(v >= min && v <= max){
			val = v;
			return true;
		} else {
			return false;
		}
	}

	const T init;
	T val, min, max;
};

struct CVarString : public CVar {
	CVarString(const str_const& name, const char* str);
	bool eval(const StrRef& s);
	void printInfo(ICLI& cli) const;
	bool set(const StrRef& s);
	bool set(StrMut&& s);

	const char* const init;
	StrMut str;
};

struct CVarEnum : public CVar {
	template<size_t N>
	CVarEnum(const str_const& name, const std::array<str_const, N>& strs, size_t index)
	: CVar(name, this)
	, strs(strs.begin(), strs.end())
	, init(index)
	, index(index){

	}

	bool eval(const StrRef& s);
	void printInfo(ICLI& cli) const;
	bool set(const str_const& s);
	bool set(uint32_t hash);
	const str_const& get() const;

	std::vector<str_const> strs;
	const size_t init;
	size_t index;
};

struct CVarBool : public CVar {
	CVarBool(const str_const& name, bool b);
	bool eval(const StrRef& val);
	void printInfo(ICLI& cli) const;
	bool set(bool b);

	const bool init;
	bool val;
};

struct CVarFunc : public CVar {
	template<class F>
	CVarFunc(const str_const& name, F&& fn, const char* usage = nullptr)
	: CVar(name, this), func(std::forward<F>(fn)), usage_str(usage){}	
	bool eval(const StrRef& str);
	void printInfo(ICLI& cli) const;
	const char* getErrorString() const;
	bool call(const StrRef& str);

	std::function<bool(const StrRef&)> func;
	const char* usage_str;
};

template<> struct cvar_id<CVarInt>    { static const int value = CVAR_INT; };
template<> struct cvar_id<CVarFloat>  { static const int value = CVAR_FLOAT; };
template<> struct cvar_id<CVarString> { static const int value = CVAR_STRING; };
template<> struct cvar_id<CVarEnum>   { static const int value = CVAR_ENUM; };
template<> struct cvar_id<CVarBool>   { static const int value = CVAR_BOOL; };
template<> struct cvar_id<CVarFunc>   { static const int value = CVAR_FUNC; };

#endif

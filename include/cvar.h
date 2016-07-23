#ifndef CVAR_H_
#define CVAR_H_
#include "common.h"
#include "util.h"
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

struct CVar {
	template<class Var>
	CVar(const str_const& name, const Var* v);

	template<class T> T* get();
	template<class T> const T* get() const;

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

template<> CVar* CVar::get<CVar>();
template<> const CVar* CVar::get<CVar>() const;

template<class T>
struct CVarNumeric : public CVar {
	CVarNumeric(const str_const& name);
	CVarNumeric(const str_const& name, T init, T min, T max);

	bool eval(const StrRef& s);
	void printInfo(ICLI& cli) const;

	bool set(T v);

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
	CVarEnum(const str_const& name, const Array<str_const, N>& strs, size_t index)
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

	Range<const str_const> strs;
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

#endif

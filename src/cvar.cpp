#include "cvar.h"
#include "cli.h"

/* CVarNumeric: int*/

template<> bool CVarNumeric<int>::eval(const string_view& str){
	return set(strtol(str.data(), nullptr, 0));
}

template<> void CVarNumeric<int>::printInfo(CLI& cli, char* buf, char* p, size_t sz) const {
	snprintf(
		p, sz,
		"%d (default %d) [int: %d < x < %d]",
		val, init, min, max
	);
	cli.echo(buf);
}

/* CVarNumeric: float */

template<> bool CVarNumeric<float>::eval(const string_view& str){
	return set(strtof(str.data(), nullptr));
}

template<> void CVarNumeric<float>::printInfo(CLI& cli, char* buf, char* p, size_t sz) const {
	snprintf(
		buf, sz,
		"%.2f (default %.2f) [float: %.2f < x < %.2f]",
		val, init, min, max
	);
	cli.echo(buf);
}

/* CVarString */

CVarString::CVarString(const str_const& name, const char* str)
: CVar(name, this)
, init(str)
, str(str){

}

bool CVarString::eval(const string_view& s){
	return set(s);
}

void CVarString::printInfo(CLI& cli, char* buf, char* p, size_t sz) const {
	snprintf(p, sz, "\"%s\" (default \"%s\") [string]", str.c_str(), init);
	cli.echo(buf);
}

bool CVarString::set(const string_view& s){
	str = std::move(s.to_string());
	return true;
}

bool CVarString::set(std::string&& s){
	str = std::move(s);
	return true;
}

/* CVarEnum */

bool CVarEnum::eval(const string_view& s){
	return set(str_hash_len(s.data(), s.size()));
}

void CVarEnum::printInfo(CLI& cli, char* buf, char* p, size_t sz) const {
	//TODO: needs better word wrapping...

	snprintf(p, sz, "'%s' (default '%s')", strs[index].str, strs[init].str);
	cli.echo(buf);

	sz += (p - buf);
	memcpy(buf, " [enum: ", std::min<size_t>(9, sz));

	size_t len = 0;

	for(size_t i = 0; i < strs.size(); ++i){
		if(len + strs[i].size > sz){
			cli.echo(buf);
			memcpy(buf, "        ", std::min<size_t>(9, sz));
			len = 0;
		}
		
		len = SDL_strlcat(buf, strs[i].str, sz);
		
		if(i != strs.size() - 1){
			len = SDL_strlcat(buf, ", ", sz);
		}
	}
	SDL_strlcat(buf, " ]", sz);

	cli.echo(buf);
}	

bool CVarEnum::set(const str_const& s){
	return set(s.hash);
}

bool CVarEnum::set(uint32_t hash){
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

const str_const& CVarEnum::get() const {
	return strs[index];
}

/* CVarBool */

CVarBool::CVarBool(const str_const& name, bool b)
: CVar(name, this), init(b), val(b){}

bool CVarBool::eval(const string_view& val){
	return set(str_to_bool(val));
}

void CVarBool::printInfo(CLI& cli, char* buf, char* p, size_t sz) const {
	const char* bstr[] = { "false", "true" };
	snprintf(p, sz, "%s (default %s) [bool]", bstr[val], bstr[init]);
	cli.echo(buf);
}

bool CVarBool::set(bool b){
	val = b;
	return true;
}

/* CVarFunc */

bool CVarFunc::eval(const string_view& str){
	return call(str);
}

void CVarFunc::printInfo(CLI& cli, char* buf, char* p, size_t sz) const {
	snprintf(p, sz, "[func] %s.", usage_str ? usage_str : "");
	cli.echo(buf);
}

const char* CVarFunc::getErrorString() const {
	return usage_str ? usage_str : "Function returned error.";
}

bool CVarFunc::call(const string_view& str){
	return func(str);
}



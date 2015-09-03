#include "cvar.h"
#include "cli.h"

/* CVarNumeric: int*/

template<> bool CVarNumeric<int>::eval(const string_view& str){
	return set(strtol(str.data(), nullptr, 0));
}

template<> void CVarNumeric<int>::printInfo(CLI& cli) const {
	cli.printf("%d (default %d) [int: %d < x < %d]\n", val, init, min, max);
}

/* CVarNumeric: float */

template<> bool CVarNumeric<float>::eval(const string_view& str){
	return set(strtof(str.data(), nullptr));
}

template<> void CVarNumeric<float>::printInfo(CLI& cli) const {
	cli.printf("%.2f (default %.2f) [int: %.2f < x < %.2f]\n", val, init, min, max);
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

void CVarString::printInfo(CLI& cli) const {
	cli.printf("\"%s\" (default \"%s\") [string]\n", str.c_str(), init);
}

bool CVarString::set(const string_view& view){
	auto s = view;
	if(  s.size() > 1
	&& ((s.front() == s.back() && s.front() == '\'')
	||  (s.front() == s.back() && s.front() == '"'))){
		s.remove_prefix(1);
		s.remove_suffix(1);
	}
	str = std::string(s.begin(), s.end());
	return true;
}

bool CVarString::set(std::string&& s){
	str = std::move(s);
	return true;
}

/* CVarEnum */

bool CVarEnum::eval(const string_view& s){
	const char* ptr = s.data();
	size_t len = s.size();
	if(len > 1
	&& ((s.front() == s.back() && s.front() == '\'')
	||  (s.front() == s.back() && s.front() == '"'))){
		++ptr;
		len -= 2;
	}

	return set(str_hash_len(ptr, len));
}

void CVarEnum::printInfo(CLI& cli) const {
	cli.printf("'%s' (default '%s')\n [enum: ", strs[index].str, strs[init].str);

	const size_t max_sz = 80;
	size_t cur_sz = 9;
	for(size_t i = 0; i < strs.size(); ++i){
		if(cur_sz + strs[i].size > max_sz){
			cli.printf("\n        ");
			cur_sz = 9;
		}
		
		cur_sz += strs[i].size;
		cli.printf("%s%s", strs[i].str, (i == strs.size() - 1) ? "" : ", ");
	}
	cli.printf(" ]\n");
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

bool CVarBool::eval(const string_view& s){
	const char* ptr = s.data();
	size_t len = s.size();
	if(len > 1
	&& ((s.front() == s.back() && s.front() == '\'')
	||  (s.front() == s.back() && s.front() == '"'))){
		++ptr;
		len -= 2;
	}
	return set(str_to_bool(string_view(ptr, len)));
}

void CVarBool::printInfo(CLI& cli) const {
	const char* bstr[] = { "false", "true" };
	cli.printf("%s (default %s) [bool]\n", bstr[val], bstr[init]);
}

bool CVarBool::set(bool b){
	val = b;
	return true;
}

/* CVarFunc */

bool CVarFunc::eval(const string_view& s){
	return call(s);
}

void CVarFunc::printInfo(CLI& cli) const {
	cli.printf("[func] %s\n", usage_str ? usage_str : "");
}

const char* CVarFunc::getErrorString() const {
	return usage_str ? usage_str : "Function returned error.";
}

bool CVarFunc::call(const string_view& s){
	const char* ptr = s.data();
	size_t len = s.size();
	if(len > 1
	&& ((s.front() == s.back() && s.front() == '\'')
	||  (s.front() == s.back() && s.front() == '"'))){
		++ptr;
		len -= 2;
	}
	return func(s);
}



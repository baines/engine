#include "config.h"
#include "engine.h"
#include "cli.h"

namespace {

struct ArgContext {
	void printHelp();
	bool getNextArg(int& output);
	bool getNextArg(const char*& output);
private:
	friend struct ::Config;
	ArgContext(int argc, char** argv);
	void parse(Config& c);
	int argc, idx;
	char** argv;
};

static struct Args {
	str_const short_name, long_name;
	const char* arg_desc;
	void(*func)(Config& c, ArgContext& ctx);
} args[] = {
	{
		{"-h"}, {"--help"}, nullptr,
		[](Config& c, ArgContext& ctx){
			ctx.printHelp();
			exit(0);
		}
	}, {
		{"-ll"}, {"--log-level"}, "<0-5>",
		[](Config& c, ArgContext& ctx){
			int level;
			if(ctx.getNextArg(level)){
				setVerbosity(static_cast<logging::level>(level));
			}
		}
	}, {
		{"-c"}, {"--config-file"}, "<path to file>",
		[](Config& c, ArgContext& ctx){
			const char* filename = nullptr;
			if(ctx.getNextArg(filename)){
				c.evalVar("cfg_filename", filename, true);
			}
		}
	}, {
		{"-fs"}, {"--full-screen"}, nullptr,
		[](Config& c, ArgContext& ctx){
			c.evalVar("vid_fullscreen", "1", true);
		}
	}, {
		{"-w"}, {"--windowed"}, nullptr,
		[](Config& c, ArgContext& ctx){
			c.evalVar("vid_fullscreen", "0", true);
		}
	}, {
		{"-r"}, {"--resolution"}, "<width> <height>",
		[](Config& c, ArgContext& ctx){
			const char *w = nullptr, *h = nullptr;
			if(ctx.getNextArg(w) && w){
				c.evalVar("vid_width", w, true);
			}
			if(ctx.getNextArg(h) && h){
				c.evalVar("vid_height", h, true);
			}
		}
	}
};

ArgContext::ArgContext(int argc, char** argv)
: argc(argc)
, idx(1)
, argv(argv){

}

void ArgContext::printHelp(){
	fprintf(stderr, "%s: Available arguments:\n", argv[0]);
	for(auto& a : args){
		fprintf(stderr, "  %s,\t%s\t%s\n",
			a.short_name.str, a.long_name.str, a.arg_desc ? a.arg_desc : "");
	}
}

bool ArgContext::getNextArg(int& i){
	if(idx + 1 >= argc) return false;
	
	if(sscanf(argv[idx+1], "%i", &i) == 1){
		idx++;
		return true;
	} else {
		return false;
	}
}

bool ArgContext::getNextArg(const char*& str){
	if(idx + 1 >= argc) return false;
	str = argv[idx+1];
	
	if((str[0] == '-' || str[0] == '+') && (str[1] < '0' || str[1] > '9')){
		return false;
	} else {
		++idx;
		return true;
	}
}

void ArgContext::parse(Config& c){
	for(idx = 1; idx < argc; ++idx){
		const char* str = argv[idx];
		const uint32_t hash = str_hash(str);
		
		// not a flag or cvar, skip it
		if(str[0] != '-' && str[0] != '+') continue;
		
		// don't parse negative numbers as flags.
		if(str[1] >= '0' && str[1] <= '9') continue;
		
		// stuff starting with + is parsed as a CVar instead of a flag.
		if(str[0] == '+'){
			const char* value = "";
			getNextArg(value);
			c.evalVar(str_hash(str+1), value, true);
		} else {
		
			bool found = false, long_name = (str[1] == '-');

			for(auto& a : args){
				if((long_name && (hash == a.long_name.hash)) 
				|| (!long_name && (hash == a.short_name.hash))){
					a.func(c, *this);
					found = true;
					break;
				}
			}
			if(!found){
				log(logging::warn, "Unknown command line argument \"%s\".", str);
			}
		}
	}
}

}

Config::Config(Engine& e, int argc, char** argv){
	/* TODO:
		if -c, SDL_RWFromFile config file + copy it to physfs write dir
	*/
	
	enum {
		GET_NAME_START,
		GET_NAME_END,
		GET_VALUE,
		SKIP_LINE
	} state = GET_NAME_START;
	
	
	if((cfg_file = e.res->load("settings.cfg"))){
		const char* data = reinterpret_cast<const char*>(cfg_file.data);
		size_t sz = cfg_file.size;
		
		uint32_t hash = 0;
		const char* name_start = data, *value = "";
		
		auto add_line = [&](const char* c){
			if(hash){ 
				const size_t sz = *value ? c - value : 0;
				evalVar(hash, { value, sz }, true);
			}
			hash = 0;
			value = "";
		};
		
		for(const char* c = data; c < data + sz; ++c){
		
			if(state == GET_NAME_START && !isspace(*c)){
				if(*c == '#' || *c == ';'){
					state = SKIP_LINE;
				} else {
					name_start = c;
					state = GET_NAME_END;
				}
			} 
			
			else if(state == GET_NAME_END && isspace(*c)){
				size_t name_sz = c - name_start;
				hash = str_hash_len(name_start, name_sz);
				state = GET_VALUE;
			}
			
			if(state == GET_VALUE){
				bool eol = (*c == '\n');
				if(eol || *c == '#' || *c == ';'){
					add_line(c);
					state = eol ? GET_NAME_START : SKIP_LINE;
				} else if(!*value && !isspace(*c)){
					value = c;
				}
			} 
			
			if(state == SKIP_LINE && *c == '\n'){
				state = GET_NAME_START;
			}
		}
		add_line(data+sz);
	} else {
		log(logging::error, "Unable to load config file.");
	}
	
	// parse command line args after, so they'll override the cfg file.
	ArgContext(argc, argv).parse(*this);
}

template<class Var>
Var* Config::getVar(strhash_t hash){
	CVar* cvar = nullptr;
	for(auto& up : cvars){
		if(up->name.hash == hash){
			cvar = up.get();
		}
	}

	return cvar ? cvar->template get<Var>() : nullptr;
}

template CVarInt* Config::getVar<CVarInt>(strhash_t);
template CVarFloat* Config::getVar<CVarFloat>(strhash_t);
template CVarString* Config::getVar<CVarString>(strhash_t);
template CVarEnum* Config::getVar<CVarEnum>(strhash_t);
template CVarBool* Config::getVar<CVarBool>(strhash_t);
template CVarFunc* Config::getVar<CVarFunc>(strhash_t);
template CVar* Config::getVar<CVar>(strhash_t);

template<class Var>
Var* Config::getVar(const StrRef& name){
	return getVar<Var>(str_hash_len(name.data(), name.size()));
}

template CVarInt* Config::getVar<CVarInt>(const StrRef&);
template CVarFloat* Config::getVar<CVarFloat>(const StrRef&);
template CVarString* Config::getVar<CVarString>(const StrRef&);
template CVarEnum* Config::getVar<CVarEnum>(const StrRef&);
template CVarBool* Config::getVar<CVarBool>(const StrRef&);
template CVarFunc* Config::getVar<CVarFunc>(const StrRef&);


template<class Var>
Var* Config::addVar(Var* v){
	cvars.emplace_back(v);
	cvar_trie.add(v->name.str, cvars.back().get());

	auto it_pair = cvar_hooks.equal_range(v->name.hash);
	for(auto& i = it_pair.first, &j = it_pair.second; i != j; ++i){
		cvars.back()->eval(i->second);
	}
	cvar_hooks.erase(it_pair.first, it_pair.second);

	return cvars.back()->get<Var>();
}

template CVarInt* Config::addVar<CVarInt>(CVarInt*);
template CVarFloat* Config::addVar<CVarFloat>(CVarFloat*);
template CVarString* Config::addVar<CVarString>(CVarString*);
template CVarEnum* Config::addVar<CVarEnum>(CVarEnum*);
template CVarBool* Config::addVar<CVarBool>(CVarBool*);
template CVarFunc* Config::addVar<CVarFunc>(CVarFunc*);

/* Cvar */

template<class T> struct cvar_id {};
template<> struct cvar_id<CVarInt>    { static const int value = CVAR_INT; };
template<> struct cvar_id<CVarFloat>  { static const int value = CVAR_FLOAT; };
template<> struct cvar_id<CVarString> { static const int value = CVAR_STRING; };
template<> struct cvar_id<CVarEnum>   { static const int value = CVAR_ENUM; };
template<> struct cvar_id<CVarBool>   { static const int value = CVAR_BOOL; };
template<> struct cvar_id<CVarFunc>   { static const int value = CVAR_FUNC; };

template<class Var>
CVar::CVar(const str_const& name, const Var* v)
: type(cvar_id<Var>::value)
, name(name)
, reload_var(nullptr) {

}

template<class T>
T* CVar::get(void) {
	if(type == cvar_id<typename std::remove_const<T>::type>::value){
		return reinterpret_cast<T*>(this);
	} else {
		return nullptr;
	}
}

template<class T>
const T* CVar::get(void) const {
	if(type == cvar_id<typename std::remove_const<T>::type>::value){
		return reinterpret_cast<T*>(this);
	} else {
		return nullptr;
	}
}

template<>
CVar* CVar::get<CVar>(void) {
	return this;
}

template<>
const CVar* CVar::get<CVar>(void) const {
	return this;
}

template CVar::CVar(const str_const& , const CVarInt* v);
template CVar::CVar(const str_const& , const CVarFloat* v);
template CVar::CVar(const str_const& , const CVarString* v);
template CVar::CVar(const str_const& , const CVarEnum* v);
template CVar::CVar(const str_const& , const CVarBool* v);
template CVar::CVar(const str_const& , const CVarFunc* v);

template CVarInt* CVar::get<CVarInt>();
template CVarFloat* CVar::get<CVarFloat>();
template CVarString* CVar::get<CVarString>();
template CVarEnum* CVar::get<CVarEnum>();
template CVarBool* CVar::get<CVarBool>();
template CVarFunc* CVar::get<CVarFunc>();

/* CvarNumeric */

template<class T>
CVarNumeric<T>::CVarNumeric(const str_const& name, T init, T min, T max)
: CVar(name, this)
, init(init)
, val(init)
, min(min)
, max(max){

}


template<class T>
CVarNumeric<T>::CVarNumeric(const str_const& name)
: CVarNumeric<T>::CVarNumeric(name, 0, 0, 0){

}

template<class T>
bool CVarNumeric<T>::set(T v){
	if(v >= min && v <= max){
		val = v;
		return true;
	} else {
		return false;
	}
}

template struct CVarNumeric<int>;
template struct CVarNumeric<float>;

/* CVarNumeric: int*/
template<> bool CVarNumeric<int>::eval(const StrRef& str){
	return set(strtol(str.data(), nullptr, 0));
}

template<> void CVarNumeric<int>::printInfo(ICLI& cli) const {
	cli.printf("%d (default %d) [int: %d < x < %d]\n", val, init, min, max);
}

/* CVarNumeric: float */

template<> bool CVarNumeric<float>::eval(const StrRef& str){
	return set(strtof(str.data(), nullptr));
}

template<> void CVarNumeric<float>::printInfo(ICLI& cli) const {
	cli.printf("%.2f (default %.2f) [int: %.2f < x < %.2f]\n", val, init, min, max);
}

/* CVarString */

CVarString::CVarString(const str_const& name, const char* str)
: CVar(name, this)
, init(str)
, str(str){

}

bool CVarString::eval(const StrRef& s){
	return set(s);
}

void CVarString::printInfo(ICLI& cli) const {
	cli.printf("\"%s\" (default \"%s\") [string]\n", str.c_str(), init);
}

bool CVarString::set(const StrRef& view){
	auto s = view;
	if(  s.size() > 1
	&& ((s.front() == s.back() && s.front() == '\'')
	||  (s.front() == s.back() && s.front() == '"'))){
		s.remove_prefix(1);
		s.remove_suffix(1);
	}
	str = StrMut(s.begin(), s.end());
	return true;
}

bool CVarString::set(StrMut&& s){
	str = std::move(s);
	return true;
}

/* CVarEnum */

bool CVarEnum::eval(const StrRef& s){
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

void CVarEnum::printInfo(ICLI& cli) const {
	cli.printf(
		"'%s' (default '%s')\n [enum: ",
		strs.begin()[index].str,
		strs.begin()[init].str
	);

	const size_t max_sz = 80;
	size_t cur_sz = 9;
	for(auto& s : strs){
		if(cur_sz + s.size > max_sz){
			cli.printf("\n        ");
			cur_sz = 9;
		}
		
		cur_sz += s.size;
		cli.printf("%s%s", s.str, (&s == strs.end() - 1) ? "" : ", ");
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
	return strs.begin()[index];
}

/* CVarBool */

CVarBool::CVarBool(const str_const& name, bool b)
: CVar(name, this), init(b), val(b){}

bool CVarBool::eval(const StrRef& s){
	const char* ptr = s.data();
	size_t len = s.size();
	if(len > 1
	&& ((s.front() == s.back() && s.front() == '\'')
	||  (s.front() == s.back() && s.front() == '"'))){
		++ptr;
		len -= 2;
	}
	return set(str_to_bool(StrRef(ptr, len)));
}

void CVarBool::printInfo(ICLI& cli) const {
	const char* bstr[] = { "false", "true" };
	cli.printf("%s (default %s) [bool]\n", bstr[val], bstr[init]);
}

bool CVarBool::set(bool b){
	val = b;
	return true;
}

/* CVarFunc */

bool CVarFunc::eval(const StrRef& s){
	return call(s);
}

void CVarFunc::printInfo(ICLI& cli) const {
	cli.printf("[func] %s\n", usage_str ? usage_str : "");
}

const char* CVarFunc::getErrorString() const {
	return usage_str ? usage_str : "Function returned error.";
}

bool CVarFunc::call(const StrRef& s){
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




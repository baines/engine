#include "config.h"

namespace {

struct ArgContext {
	void printHelp();
	bool getNextArg(int& output);
	bool getNextArg(const char*& output);
private:
	friend class ::Config;
	ArgContext(int argc, char** argv);
	void parse(Config& c);
	int argc, idx;
	char** argv;
};

static struct Args {
	str_const short_name, long_name;
	void(*func)(Config& c, ArgContext& ctx);
} args[] = {
	{ {"-h"}, {"--help"}, [](Config& c, ArgContext& ctx){
		ctx.printHelp();
		exit(0);
	}},
	{ {"-ll"}, {"--log-level"}, [](Config& c, ArgContext& ctx){
		int level;
		if(ctx.getNextArg(level)){
			setVerbosity(static_cast<logging::level>(level));
		}
	}}
};

ArgContext::ArgContext(int argc, char** argv)
: argc(argc)
, idx(1)
, argv(argv){

}

void ArgContext::printHelp(){
	fprintf(stderr, "%s: Available arguments:\n", argv[0]);
	for(auto& a : args){
		fprintf(stderr, "\t%s, %s\n", a.short_name.str, a.long_name.str);
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
	str = argv[++idx];
	return true;
}

void ArgContext::parse(Config& c){
	for(idx = 1; idx < argc; ++idx){
		const char* str = argv[idx];
		const size_t sz = strlen(str);
		const uint32_t hash = djb2(str);
		
		if(sz >= 2 && str[0] == '-'){
		
			// don't parse negative numbers as flags.
			if(str[1] >= '0' && str[1] <= '9') continue;
			
			// long name
			if(str[1] == '-'){
				for(auto a : args){
					if(hash == a.long_name.hash){
						a.func(c, *this);
					}
				}
			} else {
				for(auto a : args){
					if(hash == a.short_name.hash){
						a.func(c, *this);
					}
				}
			}
		}
	}
}

}

Config::Config(int argc, char** argv){	
	ArgContext(argc, argv).parse(*this);
}

void Config::loadConfigFile(Engine& e){
	//TODO;
}

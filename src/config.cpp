#include "config.h"
#include <string>
#include <sstream>
#include "engine.h"

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
		const char* data = reinterpret_cast<const char*>(cfg_file.data());
		size_t sz = cfg_file.size();
		
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


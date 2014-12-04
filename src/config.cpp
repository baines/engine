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
	friend class ::Config;
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
		{"-ll"}, {"--log-level"}, "<0-4>",
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
				c.overrideVar("cfg_filename", filename);
			}
		}
	}, {
		{"-fs"}, {"--full-screen"}, nullptr,
		[](Config& c, ArgContext& ctx){
			c.overrideVar("vid_fullscreen", true);
		}
	}, {
		{"-w"}, {"--windowed"}, nullptr,
		[](Config& c, ArgContext& ctx){
			c.overrideVar("vid_fullscreen", false);
		}
	}, {
		{"-r"}, {"--resolution"}, "<width> <height>",
		[](Config& c, ArgContext& ctx){
			int w = 0, h = 0;
			if(ctx.getNextArg(w) && w){
				c.overrideVar("vid_width", w);
			}
			if(ctx.getNextArg(h) && h){
				c.overrideVar("vid_height", h);
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
	str = argv[++idx];
	//XXX: make sure it's not another flag? (i.e. check for '-', but not -number)
	return true;
}

void ArgContext::parse(Config& c){
	for(idx = 1; idx < argc; ++idx){
		const char* str = argv[idx];
		const size_t sz = strlen(str);
		const uint32_t hash = djb2(str);
		
		// not a flag, skip it
		if(sz < 2 || str[0] != '-') continue;
		
		// don't parse negative numbers as flags.
		if(str[1] >= '0' && str[1] <= '9') continue;
		
		bool found = false, long_name = (str[1] == '-');

		for(auto& a : args){
			if(long_name && hash == a.long_name.hash 
			|| !long_name && hash == a.short_name.hash){
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

struct Override {
	str_const name;
	int type;
	variant<int, bool, float, str_const, std::string> data;
};

}

Config::Config(Engine& e, int argc, char** argv){
	ArgContext(argc, argv).parse(*this);
	
	/* TODO:
		if -c, SDL_RWFromFile config file + copy it to physfs write dir
	*/
	
	enum {
		GET_NAME_START,
		GET_NAME_END,
		GET_VALUE
	} state = GET_NAME_START;
	
	
	if(cfg_file = e.res.load("settings.cfg")){
		const char* data = reinterpret_cast<const char*>(cfg_file.data());
		size_t sz = cfg_file.size();
		
		uint32_t hash = 0;
		const char* name_start = data, *value = nullptr;
		
		auto add_line = [&](const char* c){
			if(hash) cfg_file_cmds.emplace(hash, value ? std::string(value, c) : "");
			hash = 0;
			value = nullptr;
		};
		
		for(const char* c = data; c < data + sz; ++c){
		
			if(state == GET_NAME_START && !isspace(*c)){
				name_start = c;
				state = GET_NAME_END;
			} 
			
			else if(state == GET_NAME_END && isspace(*c)){
				size_t name_sz = c - name_start;
				hash = djb2(name_start, name_sz);
				DEBUGF("Got name: %.*s = %d", name_sz, name_start, hash);
				state = GET_VALUE;
			}
			
			if(state == GET_VALUE){
				if(*c == '\n'){
					add_line(c);
					state = GET_NAME_START;
				} else if(!value && !isspace(*c)){
					value = c;
				}
			} 
		}
		add_line(data+sz);
	}
}


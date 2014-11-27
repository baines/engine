#include "log.h"
#include <list>
#include <tuple>
#include <algorithm>
#include <stdarg.h>
#include <cassert>
#include <cstring>
#include <SDL2/SDL.h>

namespace {

	static std::list<logging::log_fn> sinks;
	static logging::level verbosity = 
#ifdef DEBUG
		logging::debug;
#else	
		logging::fatal;
#endif

	static SDL_MessageBoxColorScheme colors = {{
		{ 27 , 24 , 37  }, // bg
		{ 200, 200, 200 }, // text
		{ 200, 200, 200 }, // button border
		{ 27 , 24 , 37  }, // button bg
		{ 255, 255, 255 }  // button selected
	}};
	
	static SDL_MessageBoxButtonData button = { 3, 0, "Ok :(" };

	static const char* get_level_prefix(logging::level l){
		switch(l){
			case logging::fatal : return "[FATAL] ";
			case logging::error : return "[ERROR] ";
			case logging::warn  : return "[WARN]  ";
			case logging::info  : return "[INFO]  ";
			case logging::debug : return "[DEBUG] ";
			default             : return "";
		}
	}

	void msgbox(const char* text){
	
		SDL_MessageBoxData msg = {
			SDL_MESSAGEBOX_ERROR,
			nullptr,
			"Fatal Error!",
			text,
			1,
			&button,
			&colors
		};
	
		SDL_ShowMessageBox(&msg, nullptr);
	}
}

namespace logging {

	void log(level l, const char* fmt, ...){
		va_list args;
		va_start(args, fmt);
		
		if(l <= verbosity){
		
			char msg[4096] = {};
		
			int msg_len = vsnprintf(msg, sizeof(msg), fmt, args);
		
			for(auto& s : sinks){
				s(l, msg, msg_len);
			}
		
			fprintf(stderr, "%s%s\n", get_level_prefix(l), msg);
			//TODO: output to file also?
			
			if(l == fatal){
				msgbox(msg);
				exit(1);
			}
		}
		
	}

	void setVerbosity(level l){
		verbosity = l;
	}

	logsink* addSink(log_fn&& fn){
		sinks.push_back(std::move(fn));
		return static_cast<logsink*>(&sinks.back());
	}

	void delSink(logsink* handle){
		log_fn* fn = reinterpret_cast<log_fn*>(handle);
		
		for(auto i = sinks.begin(), j = sinks.end(); i != j; ++i){
			if(&(*i) == fn){
				sinks.erase(i);
				break;
			}
		}
	}
}


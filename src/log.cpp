#include "log.h"
#include <list>
#include <tuple>
#include <algorithm>
#include <stdarg.h>
#include <cassert>
#include <cstring>

namespace {

	static std::list<logging::log_fn> sinks;
	static logging::level verbosity = 
#ifdef DEBUG
		logging::debug;
#else	
		logging::fatal;
#endif
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
		
			fprintf(stderr, "%s%s", get_level_prefix(l), msg);
			//TODO: output to file also?
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


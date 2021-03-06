#ifndef LOG_H_
#define LOG_H_
#include "common.h"
#include <functional>

namespace logging {

	typedef void logsink;

	enum level : uint32_t {
		fatal = 0,
		error = 1,
		warn  = 2,
		info  = 3,
		debug = 4,
		trace = 5
	};

	const char* lvl_str(level l);

	void log(level l, const char* fmt, ...) __attribute__ ((format (printf, 2, 3)));
	
	void setVerbosity(level l);
	
	typedef std::function<void(level, const char* msg, size_t msg_sz)> log_fn;
	
	logsink* addSink(log_fn&& fn);
	void delSink(logsink* handle);
}

#endif


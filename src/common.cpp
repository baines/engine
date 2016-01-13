#include "common.h"
#include <list>
#include <algorithm>
#include <stdarg.h>
#include <cstdio>
#include <cstdlib>
#include <SDL.h>

namespace {

	struct LogSink {
		logging::log_fn fn;
		void* usr;
	};

	static std::vector<LogSink> sinks;
	static logging::level verbosity = 
#ifdef DEBUG
		logging::debug;
#else	
		logging::fatal;
#endif


#ifndef __EMSCRIPTEN__
	static SDL_MessageBoxColorScheme colors = {{
		{ 27 , 24 , 37  }, // bg
		{ 200, 200, 200 }, // text
		{ 200, 200, 200 }, // button border
		{ 27 , 24 , 37  }, // button bg
		{ 255, 255, 255 }  // button selected
	}};

	static SDL_MessageBoxButtonData button = { 3, 0, "Ok :(" };
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
#else
	void msgbox(const char* text){
		char buf[1024];
		int text_len = std::min<int>(1000, strlen(text));

		snprintf(buf, sizeof(buf), "alert('Error: %.*s');", text_len, text);
		for(int i = 14; i < 14 + text_len; ++i){
			if(buf[i] == '\'') buf[i] = ' ';
		}

		emscripten_run_script(buf);
	}
#endif
}

namespace logging {

	const char* lvl_str(logging::level l){
		switch(l){
			case logging::fatal : return "[FATAL] ";
			case logging::error : return "[ERROR] ";
			case logging::warn  : return "[WARN]  ";
			case logging::info  : return "[INFO]  ";
			case logging::debug : return "[DEBUG] ";
			case logging::trace : return "[TRACE] ";
			default             : return "";
		}
	}

	void log(level l, const char* fmt, ...){
		va_list args;
		va_start(args, fmt);

		if(l <= verbosity){
			char msg[4096] = {};

			int msg_len = vsnprintf(msg, sizeof(msg), fmt, args);

			for(auto& s : sinks){
				s.fn(l, msg, msg_len, s.usr);
			}

			fprintf(stderr, "%s%s\n", lvl_str(l), msg);
			//TODO: output to file also?

			if(l == fatal){
				msgbox(msg);
				va_end(args);
				exit(1);
			}
		}
		va_end(args);
	}

	void setVerbosity(level l){
		verbosity = l;
	}

	log_handle addSink(log_fn fn, void* usr){
		sinks.push_back({ fn, usr });
		return sinks.size() - 1;
	}

	void delSink(log_handle handle){
		sinks.erase(sinks.begin() + handle, sinks.begin() + handle + 1);
	}
}

StrMut32 to_utf32(const StrRef& s){
	size_t u32str_max = s.size() * sizeof(char32_t);
	
	char32_t* u32str = SDL_stack_alloc(char32_t, s.size());
	
	char* out        = reinterpret_cast<char*>(u32str);
	const char* in   = s.data();
	size_t out_sz    = u32str_max;
	size_t in_sz     = s.size();
	
	auto ctx = SDL_iconv_open("UTF-32LE", "UTF-8");
	SDL_iconv(ctx, &in, &in_sz, &out, &out_sz);
	SDL_iconv_close(ctx);
	
	StrMut32 ret(u32str, (u32str_max - out_sz) / sizeof(char32_t));
	SDL_stack_free(u32str);

	return ret;
}

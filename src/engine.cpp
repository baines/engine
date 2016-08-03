#include "engine.h"
#include "resource_system.h"
#include "config.h"
#include "input_private.h"
#include "renderer_private.h"
#include "text_system_private.h"
#include "collision_system.h"
#include "state_system.h"
#include "root_state.h"
#include "cli_private.h"
#include <locale.h>
#include <SDL.h>

template class alt::TStrRef<char>;
template class alt::TStrRef<char32_t>;
template class alt::TStrMut<char>;
template class alt::TStrMut<char32_t>;

template class alt::vec2t<float>;
template class alt::vec2t<int>;
template class alt::vec4t<float>;

namespace {
	static int get_ticks(){
#ifdef __EMSCRIPTEN__
		return emscripten_get_now() + 0.5;
#else
		return SDL_GetTicks();
#endif
	}
}

Engine::Engine(int argc, char** argv, const char* name){

	setlocale(LC_ALL, "");

	SDL_Init(0);

	res        = UniquePtr<ResourceSystem>  (new ResourceSystem(argv[0]));
	cfg        = UniquePtr<Config>          (new Config(*this, argc, argv));
	input      = UniquePtr<IInput>          (new Input(*this));
	renderer   = UniquePtr<IRenderer>       (new Renderer(*this, name));
	text       = UniquePtr<ITextSystem>     (new TextSystem(*this));
	collision  = UniquePtr<CollisionSystem> (new CollisionSystem());
	state      = UniquePtr<StateSystem>     (new StateSystem());
	cli        = UniquePtr<ICLI>            (new CLI(*this));
	max_fps    = cfg->addVar<CVarInt>("max_fps", 200, 1, 1000);
	running    = true;
	prev_ticks = 0;
	root_state = UniquePtr<RootState>(new RootState(*this));

	addState(root_state.get());
}

void Engine::addState(GameState* s){
	state->push(s);
}

bool Engine::run(void){

	TRACEF("---------- Frame Begin ----------");

	const int min_delta = 1000 / std::max(1, max_fps->val);
	int delta = std::max<int>(0, get_ticks() - prev_ticks);

	if(delta > 3000){
		puts("wtf big ass delta");
		exit(1);
	}

	if(delta < min_delta){
		SDL_Delay(min_delta - delta);
		delta = min_delta;
	}
	prev_ticks += delta;

	delta = std::min(delta, 100);

	state->processStateChanges(*this);

	SDL_Event e;
	
	while(SDL_PollEvent(&e)){
		switch(e.type){
			case SDL_KEYDOWN:
			case SDL_KEYUP:
			case SDL_CONTROLLERBUTTONDOWN:
			case SDL_CONTROLLERBUTTONUP:
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
			case SDL_MOUSEWHEEL:
				state->onInput(*this, e);
				break;
			case SDL_TEXTINPUT:
				state->onText(*this, e.text);
				break;
			case SDL_MOUSEMOTION:
			case SDL_CONTROLLERAXISMOTION:
				state->onMotion(*this, e);
				break;
			case SDL_CONTROLLERDEVICEADDED:
			case SDL_CONTROLLERDEVICEREMOVED:
			case SDL_CONTROLLERDEVICEREMAPPED:
				input->onDeviceChange(e.cdevice);
				break;
			case SDL_WINDOWEVENT: {
				if(e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED){
					renderer->handleResize(e.window.data1, e.window.data2);
					state->onResize(*this, e.window.data1, e.window.data2);
				}
				break;
			}
			case SDL_QUIT:
				running = false;
			default:
				break;
		}
	}
	
	state->update(*this, delta);
	collision->update(delta);
	
//	renderer->beginFrame();
	state->draw(*renderer);	
	renderer->drawFrame();

	TRACEF("---------- Frame End ----------");

	return running;
}

void Engine::quit(){
	running = false;
}

Engine::~Engine(){
	SDL_Quit();
}

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


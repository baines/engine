#ifndef COMMON_H_
#define COMMON_H_
#include <array>
#include <cstdint>
#include <experimental/string_view>
using string_view = std::experimental::string_view;

#include "log.h"

#ifdef DEBUG
	#define DEBUGF(fmt, ...) do { log(logging::debug, fmt, ##__VA_ARGS__); } while (0)
#else
	#define DEBUGF(fmt, ...) do { } while(0)
#endif

struct Engine;
struct Renderable;
struct Renderer;
struct GameState;
struct ResourceHandle;

#endif


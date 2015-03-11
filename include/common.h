#ifndef COMMON_H_
#define COMMON_H_
#include <array>
#include <cstdint>
#include <glm/glm.hpp>
#if defined(__GNUC__) && __GNUC_MINOR__ < 9
	#include "compat.h"
#else
	#include <experimental/string_view>
#endif

using string_view = std::experimental::string_view;
using u32string_view = std::experimental::u32string_view;

#include "log.h"

#ifdef DEBUG
	#define DEBUGF(fmt, ...) do { log(logging::debug, fmt, ##__VA_ARGS__); } while (0)
	#define TRACEF(fmt, ...) do { log(logging::trace, fmt, ##__VA_ARGS__); } while (0)
#else
	#define DEBUGF(fmt, ...) do { } while(0)
	#define TRACEF(fmt, ...) do { } while(0)
#endif

struct Engine;
struct Renderable;
struct Renderer;
struct CollisionSystem;
struct GameState;
struct ResourceHandle;
struct CLI;
struct GLObject;
struct Sprite;
struct SpriteBatch;
struct Material;
struct Entity;

#endif


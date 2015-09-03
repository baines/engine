#ifndef COMMON_H_
#define COMMON_H_
#include <array>
#include <cstdint>
#include <cmath>
#include <glm/glm.hpp>
#include "altlib/alt.h"

struct string_view : alt::StrRef {
	using alt::StrRef::StrRef;
	string_view(const std::string& s) : alt::StrRef(s.data(), s.size()) {}
};
struct u32string_view : alt::StrRef32 {
	using alt::StrRef32::StrRef32;
	u32string_view(const std::u32string& s) : alt::StrRef32(s.data(), s.size()){}
};

#include "log.h"

#ifndef M_PI
	#define M_PI 3.14159265358979323846
#endif

#ifdef __EMSCRIPTEN__
#include "emscripten.h"
#endif

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
struct RenderState;
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


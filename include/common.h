#ifndef COMMON_H_
#define COMMON_H_
#include <cstdint>
#include <cstdlib>
#include <memory>
#include "altlib/alt.h"

struct Engine;
struct Renderable;
struct ResourceSystem;
struct Config;
struct Input;
struct Renderer;
struct TextSystem;
struct CollisionSystem;
struct StateSystem;
struct CLI;
struct RenderState;
struct GameState;
struct ResourceHandle;
struct GLObject;
struct Sprite;
struct SpriteBatch;
struct Material;
struct Entity;
struct RootState;
struct str_const;
struct CVar;
struct Font;
struct VertexState;
struct ShaderProgram;
struct IndexBuffer;
struct Texture;
struct Sampler;
struct Material;
struct VertexBuffer;
struct Text;
struct ShaderUniforms;

template<class T> struct CVarNumeric;
using CVarInt = CVarNumeric<int>;
using CVarFloat = CVarNumeric<float>;
struct CVarEnum;
struct CVarString;
struct CVarBool;
struct CVarFunc;

struct string_view : alt::StrRef {
	using alt::StrRef::StrRef;
	string_view(const std::string& s) : alt::StrRef(s.data(), s.size()) {}
};
struct u32string_view : alt::StrRef32 {
	using alt::StrRef32::StrRef32;
	u32string_view(const std::u32string& s) : alt::StrRef32(s.data(), s.size()){}
};

#ifdef __EMSCRIPTEN__
#include "emscripten.h"
#endif

#include "log.h"

#ifdef DEBUG
	#define DEBUGF(fmt, ...) do { log(logging::debug, fmt, ##__VA_ARGS__); } while (0)
	#define TRACEF(fmt, ...) do { log(logging::trace, fmt, ##__VA_ARGS__); } while (0)
#else
	#define DEBUGF(fmt, ...) do { } while(0)
	#define TRACEF(fmt, ...) do { } while(0)
#endif

#endif


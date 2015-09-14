#ifndef COMMON_H_
#define COMMON_H_
#include <cstdint>
#include <cstdlib>
#include <memory>
#include "altlib/alt.h"

#if defined(_WIN32) && defined(__GNUC__) && __GNUC_MINOR__ < 9
	#include "compat.h"
	#include <stdio.h>
#endif

#ifndef M_PI
	#define M_PI 3.14159265358979323846	
#endif

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

using strhash_t = uint32_t;

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


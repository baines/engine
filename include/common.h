#ifndef COMMON_H_
#define COMMON_H_
#include <stdint.h>
#include <stdlib.h>
#include "alt/alt_str.h"
#include "alt/alt_array.h"
#include "alt/alt_math.h"

using alt::StrRef;
using alt::StrRef32;
using alt::StrMut;
using alt::StrMut32;
using alt::vec2;
using alt::vec2i;
using alt::vec4;
using alt::Array;

#if defined(_WIN32) && defined(__GNUC__) && __GNUC_MINOR__ < 9
	#include "compat.h"
	#include <stdio.h>
#endif

#ifndef M_PI
	#define M_PI 3.14159265358979323846	
#endif

struct IInput;
struct IRenderer;
struct ITextSystem;
struct ICLI;

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

namespace logging {

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
	
	typedef void (*log_fn)(level, const char* msg, size_t msg_sz, void* usr);
	typedef int log_handle;

	log_handle addSink(log_fn fn, void* usr);
	void delSink(log_handle handle);
}

#ifdef DEBUG
	#define DEBUGF(fmt, ...) do { log(logging::debug, fmt, ##__VA_ARGS__); } while (0)
	#define TRACEF(fmt, ...) do { log(logging::trace, fmt, ##__VA_ARGS__); } while (0)
#else
	#define DEBUGF(fmt, ...) do { } while(0)
	#define TRACEF(fmt, ...) do { } while(0)
#endif

#endif


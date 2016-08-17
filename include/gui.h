#ifndef GUI_H_
#define GUI_H_
#include "common.h"
#include "vertex_buffer.h"
#include "index_buffer.h"
#include "vertex_state.h"
#include "shader.h"
#include "texture.h"

#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_BUTTON_TRIGGER_ON_RELEASE
#define NK_MEMSET memset
#define NK_MEMCOPY memcpy
#define NK_SQRT sqrt
#define NK_SIN sin
#define NK_COS cos
#include "nuklear.h"

struct GUI {
	GUI  (Engine& e);
	~GUI ();

	int  initInput (Engine& e, GameState* state, int max_id);

	// this class should probably be a GameState somehow so all this stuff
	// doesn't need to be forwarded...

	void onInput   (int key, bool pressed);
	void onMotion  (int axis, int val);
	void onText    (const char* txt);

	void draw      (IRenderer&);

	nk_context* begin();
	void end();

private:
	nk_context* ctx;
	nk_user_font nk_font;

	DynamicVertexBuffer verts;
	DynamicIndexBuffer<uint16_t> indices;
	VertexState state;

	Resource<VertShader> vs;
	Resource<FragShader> fs;

	ShaderProgram shader;

	Resource<Font, uint16_t> font;

	vec2i cursor;
	int input_id;

	bool dirty;

	// temporary until i rewrite the renderer::addRenderable to not store pointers
	std::vector<Renderable> renderables;
};

#endif

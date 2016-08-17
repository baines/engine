#ifndef GUI_H_
#define GUI_H_
#include "common.h"
#include "vertex_buffer.h"
#include "index_buffer.h"
#include "vertex_state.h"
#include "shader.h"
#include "texture.h"

struct nk_context;
struct nk_buffer;

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

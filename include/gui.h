#ifndef GUI_H_
#define GUI_H_
#include "common.h"
#include "vertex_buffer.h"
#include "index_buffer.h"
#include "vertex_state.h"
#include "shader.h"
#include "texture.h"

struct nk_context;

struct GUI {
	GUI  (Engine& e);
	~GUI ();

	int  initInput (Engine& e, GameState* state, int max_id);
	void onInput   (int key, bool pressed);
	void onMotion  (int axis, int val);
	void draw      (IRenderer&);

	nk_context* ctx;
private:
	DynamicVertexBuffer verts;
	DynamicIndexBuffer<uint16_t> indices;
	VertexState state;

	Resource<VertShader> vs;
	Resource<FragShader> fs;

	ShaderProgram shader;

	Texture2D null_tex;

	Resource<Font, uint16_t> font;

	vec2i cursor;
	int input_id;
};

#endif

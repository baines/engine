#include "gui.h"
#include "blend_mode.h"
#include "renderable.h"
#include "renderer.h"
#include <cmath>

#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_BUTTON_TRIGGER_ON_RELEASE
#define NK_MEMSET memset
#define NK_MEMCOPY memcpy
#define NK_SQRT sqrt
#define NK_SIN sin
#define NK_COS cos
#define NK_IMPLEMENTATION
#include "nuklear.h"

#define MAX_VERT_SIZE (1024 * 1024)
#define MAX_INDX_SIZE MAX_VERT_SIZE

// FIXME should be cached
static const uint32_t white_rgba = UINT32_C(0xffffffff);

GUI::GUI(Engine& e)
: verts    ("pos:2f|tex:2f|col:4BN", MAX_VERT_SIZE)
, indices  ()
, state    ({ &verts }, &indices)
, vs       (e, {"gui.glslv"})
, fs       (e, {"gui.glslf"})
, shader   (vs, fs)
, null_tex (GL_RGBA, GL_RGBA, 1, 1, &white_rgba) {

}

void GUI::draw(IRenderer& r){
	struct nk_convert_config cfg = {};
	cfg.global_alpha = 1.0f;
	cfg.circle_segment_count = cfg.arc_segment_count = cfg.curve_segment_count = 22;
	cfg.null.texture.ptr = &null_tex;
	cfg.null.uv = nk_vec2(0.5f, 0.5f);

	//TODO: fonts?

	char cmd_buffer[4096];

	uint8_t* vptr = verts.beginWrite(MAX_VERT_SIZE);
	uint8_t* iptr = indices.beginWrite(MAX_INDX_SIZE);

	struct nk_buffer vb, ib, cmds;
	nk_buffer_init_fixed(&vb, vptr, MAX_VERT_SIZE);
	nk_buffer_init_fixed(&ib, iptr, MAX_INDX_SIZE);
	nk_buffer_init_fixed(&cmds, cmd_buffer, sizeof(cmd_buffer));

	nk_convert(ctx, &cmds, &vb, &ib, &cfg);

	verts.endWrite(vb.size);
	indices.endWrite(ib.size);

	Renderable obj = {};
	obj.prim_type = GL_TRIANGLES;
	obj.shader = &shader;
	obj.vertex_state = &state;
	obj.blend_mode = BlendMode({ GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA });

	const struct nk_draw_command* cmd;

	nk_draw_foreach(cmd, ctx, &cmds){
		if(!cmd->elem_count) continue;

		obj.textures[0] = (Texture*)cmd->texture.ptr;
//		obj.clip = cmd->clip_rect; TODO
		obj.count = cmd->elem_count;
	
		r.addRenderable(obj);

		obj.offset += obj.count;
	}
	nk_clear(ctx);
}


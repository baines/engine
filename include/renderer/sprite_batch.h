#ifndef SPRITE_BATCH_H_
#define SPRITE_BATCH_H_
#include "common.h"
#include "index_buffer.h"
#include "vertex_buffer.h"
#include "vertex_state.h"
#include "renderable.h"
#include <map>

struct SpriteBatch : public BufferInvalidateListener {

	SpriteBatch() = default;
	SpriteBatch(Material& m, glm::ivec2 tex_cells = { 1, 1 });

	void addSprite(const Sprite& s);
	void delSprite(const Sprite& s);
	void updateSprite(const Sprite& s);

	const Material* getMaterial(){
		return material;
	}

	void draw(IRenderer& r);

	void onBufferRangeInvalidated(size_t off, size_t len) override;

private:
	struct SpriteInfo {
		size_t v_off, i_off;
		bool dirty, hide;
	};

	std::map<const Sprite*, SpriteInfo> sprites;
	VertexState vao;
	DynamicVertexBuffer vertices;
	DynamicIndexBuffer<uint16_t> indices;
	Material* material;
	Renderable renderable;
	glm::ivec2 tex_cells;
};

#endif


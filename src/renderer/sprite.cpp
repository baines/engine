#include "sprite.h"
#include "sprite_batch.h"
#include "material.h"
#include "renderer.h"
#include <limits.h>
#include <assert.h>

Sprite::Sprite(SpriteBatch& b, vec2i pos, vec2i sz, vec2i frame)
: batch(&b)
, position(pos)
, size(sz)
, tex_frame(frame) {
	if(batch){
		batch->addSprite(*this);
	}
}

Sprite::Sprite(std::tuple<SpriteBatch&, vec2i&&, vec2i&&>&& t)
: batch(&std::get<0>(t))
, position(std::get<1>(t))
, size(std::get<2>(t))
, tex_frame({ 0, 0 }){
	if(batch){
		batch->addSprite(*this);
	}
}

Sprite::Sprite(Sprite&& other)
: batch(other.batch)
, position(other.position)
, size(other.size)
, tex_frame(other.tex_frame){
	if(batch){
		batch->addSprite(*this);
	}
}

void Sprite::setPosition(vec2i pos){
	position = pos;
	if(batch){
		batch->updateSprite(*this);
	}
}

void Sprite::setSize(vec2i sz){
	size = sz;
	if(batch){
		batch->updateSprite(*this);
	}
}

void Sprite::setFrame(vec2i frame){
	tex_frame = frame;
	if(batch){
		batch->updateSprite(*this);
	}
}

Sprite::~Sprite(){
	if(batch){
		batch->delSprite(*this);
	}
}

struct Vert {
	Vert() = default;
	Vert(int16_t x, int16_t y, uint16_t tx, uint16_t ty)
		: x(x), y(y), tx(tx), ty(ty){}
	int16_t x, y;
	uint16_t tx, ty;
};

SpriteBatch::SpriteBatch(Material& m, vec2i tex_cells)
: vertices("a_pos:2s|a_tex:2SN", 512)
, material(&m)
, tex_cells(tex_cells) {
	assert(tex_cells.x > 0 && tex_cells.y > 0);

	renderable.vertex_state = &vao;
	renderable.shader = m.shader;
	renderable.uniforms = &m.uniforms;
	renderable.prim_type = GL_TRIANGLES;

	vao.setVertexBuffers({ &vertices });
	vao.setIndexBuffer(&indices);

	renderable.textures[0] = m.texture;
	renderable.samplers[0] = m.sampler;
}

void SpriteBatch::addSprite(const Sprite& s){
	sprites[&s].dirty = true;
}

void SpriteBatch::delSprite(const Sprite& s){
	sprites.erase(&s);
}

void SpriteBatch::updateSprite(const Sprite& s){
	auto it = sprites.find(&s);
	if(it != sprites.end()){
		it->second.dirty = true;
	}
}

void SpriteBatch::draw(IRenderer& r){
	for(auto& pair : sprites){
		if(!pair.second.dirty) continue;

		auto pos   = pair.first->getPosition();
		auto sz    = pair.first->getSize() / 2;
		auto frame = pair.first->getFrame();

		vec4 tex_coords = {
			frame.x       / (float) tex_cells.x,
			frame.y       / (float) tex_cells.y,
			(frame.x + 1) / (float) tex_cells.x,
			(frame.y + 1) / (float) tex_cells.y
		};

		uint16_t tx0 = tex_coords.x * USHRT_MAX,
				 ty0 = tex_coords.y * USHRT_MAX,
				 tx1 = tex_coords.z * USHRT_MAX,
				 ty1 = tex_coords.w * USHRT_MAX;

		if(pair.second.v_off > 0){
			vertices.invalidate(BufferRange{
				(pair.second.v_off - 1) * sizeof(Vert),
				4 * sizeof(Vert),
				this
			});
		}
		
		const size_t v_sz  = vertices.getSize(),
		             i_sz  = indices.getSize(),
		             i_off = pair.second.i_off;

		size_t index = i_off - 1;	
		for(auto i : { 0, 1, 2, 2, 1, 3 }){
			if(i_off > 0){
				indices.replace(index++, v_sz + i);
			} else {
				indices.push(v_sz + i);
			}
		}

		if(i_off == 0){
			pair.second.i_off = i_sz + 1;
		}
		pair.second.v_off = v_sz + 1;

		vertices.push(Vert(pos.x - sz.x, pos.y - sz.y, tx0, ty0));
		vertices.push(Vert(pos.x - sz.x, pos.y + sz.y, tx0, ty1));
		vertices.push(Vert(pos.x + sz.x, pos.y - sz.y, tx1, ty0));
		vertices.push(Vert(pos.x + sz.x, pos.y + sz.y, tx1, ty1));

		pair.second.dirty = false;
	}
	renderable.count = indices.getSize();

	r.addRenderable(renderable);
}

void SpriteBatch::onBufferRangeInvalidated(size_t off, size_t len){
	for(auto& pair : sprites){
		size_t& v_off = pair.second.v_off;
		size_t& i_off = pair.second.i_off;
		if((v_off - 1) * sizeof(Vert) <= off) continue;
		
		v_off -= (len / sizeof(Vert));

		assert(i_off > 0);

		size_t index = i_off - 1;
		for(auto i : { 0, 1, 2, 2, 1, 3 }){
			indices.replace(index++, (v_off - 1) + i);
		}
	}
}

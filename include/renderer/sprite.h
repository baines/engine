#ifndef SPRITE_H_
#define SPRITE_H_
#include "common.h"
#include <tuple>

struct Sprite {

	Sprite() = default;
	Sprite(SpriteBatch& batch, vec2i position  = { 0, 0 },
	                           vec2i size      = { 0, 0 },
	                           vec2i tex_frame = { 0, 0 });

	Sprite(std::tuple<SpriteBatch&, vec2i&&, vec2i&&>&&);
	Sprite(Sprite&&);

	void setPosition(vec2i pos);
	void setSize(vec2i size);
	void setFrame(vec2i frame);

	vec2i getPosition() const { return position; }
	vec2i getSize()     const { return size; }
	vec2i getFrame()    const { return tex_frame; }

	~Sprite();
private:
	SpriteBatch* batch;
	vec2i position, size, tex_frame;
};

#endif

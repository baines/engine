#ifndef SPRITE_H_
#define SPRITE_H_
#include "common.h"
#include "glm/glm.hpp"
#include <tuple>

struct Sprite {

	Sprite() = default;
	Sprite(SpriteBatch& batch, glm::ivec2 position  = { 0, 0 },
	                           glm::ivec2 size      = { 0, 0 },
	                           glm::ivec2 tex_frame = { 0, 0 });

	Sprite(std::tuple<SpriteBatch&, glm::ivec2&&, glm::ivec2&&>&&);
	Sprite(Sprite&&);

	void setPosition(glm::ivec2 pos);
	void setSize(glm::ivec2 size);
	void setFrame(glm::ivec2 frame);

	glm::ivec2 getPosition() const { return position; }
	glm::ivec2 getSize()     const { return size; }
	glm::ivec2 getFrame()    const { return tex_frame; }

	~Sprite();
private:
	SpriteBatch* batch;
	glm::ivec2 position, size, tex_frame;
};

#endif

#include "sprite.h"
#include "sprite_batch.h"

Sprite::Sprite(SpriteBatch& batch, glm::ivec2 pos, glm::ivec2 sz, glm::ivec2 frame)
: batch(&batch)
, position(pos)
, size(sz)
, tex_frame(frame) {
	batch.addSprite(*this);
}

void Sprite::setPosition(glm::ivec2 pos){
	position = pos;
	if(batch){
		batch->updateSprite(*this);
	}
}

void Sprite::setSize(glm::ivec2 sz){
	size = sz;
	if(batch){
		batch->updateSprite(*this);
	}
}

void Sprite::setFrame(glm::ivec2 frame){
	tex_frame = frame;
	if(batch){
		batch->updateSprite(*this);
	}
}



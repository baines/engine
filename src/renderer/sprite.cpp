#include "sprite.h"
#include "sprite_batch.h"

Sprite::Sprite(SpriteBatch& b, glm::ivec2 pos, glm::ivec2 sz, glm::ivec2 frame)
: batch(&b)
, position(pos)
, size(sz)
, tex_frame(frame) {
	if(batch){
		batch->addSprite(*this);
	}
}

Sprite::Sprite(std::tuple<SpriteBatch&, glm::ivec2&&, glm::ivec2&&>&& t)
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

Sprite::~Sprite(){
	if(batch){
		batch->delSprite(*this);
	}
}



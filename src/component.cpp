#include "component.h"
#include "entity.h"
#include "collision_system.h"
#include "sprite.h"
#include <atomic>

static std::atomic<unsigned> id(0);

unsigned getNextComponentID(){
	return id++;
}

Position2D::Position2D(glm::vec2 p)
: pos(p)
, entity(nullptr) {

}

void Position2D::initComponent(Engine& e, Entity& ent){
	entity = &ent;
	
	set(pos);

	if(auto* a = entity->get<AABB>()){
		a->setPrevPosition(pos);
	}
}

glm::vec2 Position2D::get() const {
	return pos;
}

void Position2D::set(glm::vec2 p){
	pos = p;

	if(auto* s = entity->get<Sprite>()){
		s->setPosition({ pos.x, pos.y });
	}

	if(auto* a = entity->get<AABB>()){
		a->setPosition(pos);
	}
}

void Position2D::add(glm::vec2 p){
	set(pos + p);
}


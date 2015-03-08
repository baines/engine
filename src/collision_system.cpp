#include "collision_system.h"
#include "entity.h"
#include "engine.h"
#include <cmath>
#include <algorithm>
using glm::vec2;
using std::abs;
namespace {

bool point_test_sweep(vec2 p, vec2 q, float& t){

	if(p[0] > q[0]){
		t = 0.0f;
	} else {
		float num   = (p[0] - q[0]);
		float denom = (q[1] - q[0]) - (p[1] - p[0]);
		
		if(abs(denom) <= FLT_EPSILON){
			return false;
		}
		
		t = num / denom;
	}

	return t >= 0.0f && t <= 1.0f;
}

/*bool aabb_test(vec2 pos0, vec2 size0, vec2 pos1, vec2 size1){
	return pos0.x + size0.x >= pos1.x           &&
	       pos0.x           <= pos1.x + size1.x &&
	       pos0.y + size0.y >= pos1.y           &&
	       pos0.y           <= pos1.y + size1.y;
}*/


}

AABB::AABB(){

}

AABB::AABB(vec2 size, uint32_t group)
: pos()
, prev_pos()
, size(size)
, collision_group(group) {

}

void AABB::setPosition(vec2 p){
	pos = p - (size / 2.f);
}

void AABB::setPrevPosition(vec2 p){
	prev_pos = p - (size / 2.f);
}

void AABB::initComponent(Engine& e, Entity& ent){
	e.collision.addEntity(ent);
}

CollisionSystem::CollisionSystem()
: boxes() {

}

void CollisionSystem::addEntity(Entity& e){
	if(AABB* aabb = e.get<AABB>()){
		auto it = std::find(boxes.begin(), boxes.end(), aabb);

		if(it == boxes.end()){
			entities.push_back(&e);
			boxes.push_back(aabb);
		}
	}
}

void CollisionSystem::onCollision(uint32_t a, uint32_t b, CollisionFunc&& f){

	uint32_t lo = std::min(a, b), hi = std::max(a, b);

	funcs[{ lo, hi }] = std::move(f);
}

void CollisionSystem::update(uint32_t delta){

	for(size_t i = 0; i < boxes.size(); ++i){
		for(size_t j = i + 1; j < boxes.size(); ++j){

			const auto& it = funcs.find({
				std::min(boxes[i]->collision_group, boxes[j]->collision_group),
				std::max(boxes[i]->collision_group, boxes[j]->collision_group)
			});

			if(it == funcs.end()) continue;

			auto& collision_fn = it->second;

			vec2 i_x = { boxes[i]->prev_pos.x, boxes[i]->pos.x },
			     i_y = { boxes[i]->prev_pos.y, boxes[i]->pos.y },
			     j_x = { boxes[j]->prev_pos.x, boxes[j]->pos.x },
			     j_y = { boxes[j]->prev_pos.y, boxes[j]->pos.y };

			bool i_x_smaller = i_x[0] < j_x[0],
			     i_y_smaller = i_y[0] < j_y[0];

			vec2 min_x = i_x_smaller ? i_x + boxes[i]->size.x : j_x + boxes[j]->size.x,
			     min_y = i_y_smaller ? i_y + boxes[i]->size.y : j_y + boxes[j]->size.y,
			     max_x = i_x_smaller ? j_x : i_x,
			     max_y = i_y_smaller ? j_y : i_y;

			vec2 i_collide, j_collide;

			float t1 = 0.0f, t2 = 0.0f;

			if(!point_test_sweep(min_x, max_x, t1)) continue;
			if(!point_test_sweep(min_y, max_y, t2)) continue;

			//TODO: sub t1 / t2 into each other's equations to find if there was
			//      actually a collision, instead of sending both x and y separately.
			collision_fn(entities[i], entities[j], t1);
			collision_fn(entities[i], entities[j], t2);
		}
	}

	for(auto* aabb : boxes){
		aabb->prev_pos = aabb->pos;
	}
}

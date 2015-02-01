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

bool aabb_test(vec2 pos0, vec2 size0, vec2 pos1, vec2 size1){
	return pos0.x + size0.x >= pos1.x           &&
	       pos0.x           <= pos1.x + size1.x &&
		   pos0.y + size0.y >= pos1.y           &&
		   pos0.y           <= pos1.y + size1.y;
}

template<class T>
T lerp(T a, T b, float t){
	return a + (b - a) * t;
}

}

AABB::AABB(){

}

AABB::AABB(vec2 size)
: pos()
, prev_pos()
, size(size) {

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
			boxes.push_back(aabb);
		}
	}
}

void CollisionSystem::update(uint32_t delta){

	for(size_t i = 0; i < boxes.size(); ++i){
		for(size_t j = i + 1; j < boxes.size(); ++j){
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

			float t = 0.0f;

			if(!point_test_sweep(min_x, max_x, t)) continue;

			i_collide = { lerp(i_x[0], i_x[1], t), lerp(i_y[0], i_y[1], t) };
			j_collide = { lerp(j_x[0], j_x[1], t), lerp(j_y[0], j_y[1], t) };
				
			if(aabb_test(i_collide, boxes[i]->size, j_collide, boxes[j]->size)){
				// notify collision at time t.
				printf("[1] COLLISION [ %.1f, %.1f ] [ %.1f, %.1f ] t: %.2f\n",
				        i_collide.x, i_collide.y, j_collide.x, j_collide.y, t);
				continue;
			}

			if(!point_test_sweep(min_y, max_y, t)) continue;

			i_collide = { lerp(i_x[0], i_x[1], t), lerp(i_y[0], i_y[1], t) };
			j_collide = { lerp(j_x[0], j_x[1], t), lerp(j_y[0], j_y[1], t) };

			if(aabb_test(i_collide, boxes[i]->size, j_collide, boxes[j]->size)){
				// notify collision at time t.
				printf("[2] COLLISION [ %.1f, %.1f ] [ %.1f, %.1f ] t: %.2f\n",
				        i_collide.x, i_collide.y, j_collide.x, j_collide.y, t);
				continue;
			}

			/*if(aabb_test(*boxes[i]->pos, boxes[i]->size, *boxes[j]->pos, boxes[j]->size)){
				puts("COLLISION");
			}*/
		}
	}

	for(auto* aabb : boxes){
		aabb->prev_pos = aabb->pos;
	}
}

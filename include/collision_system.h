#ifndef COLLISION_SYSTEM_H_
#define COLLISION_SYSTEM_H_
#include "common.h"
#include <vector>
#include <unordered_map>

struct AABB {
	AABB();
	AABB(vec2 size, uint32_t group = 0);

	void setPosition(vec2 p);
	void setPrevPosition(vec2 p);

	void initComponent(Engine&, Entity&);
	//TODO: offset;
	vec2 pos, prev_pos, size;

	uint32_t collision_group;
};

struct CollisionSystem {
	CollisionSystem();
	void addEntity(Entity& e);
	void update(uint32_t delta);
	
	typedef Closure<void(Entity* a, Entity* b, float t)> CollisionFunc;
	
	template<class F>
	void onCollision(uint32_t a, uint32_t b, F&& f){
		uint64_t key = (a > b) ? (uint64_t(a) << 32ULL | b) : (uint64_t(b) << 32ULL | a);
		funcs[key] = CollisionFunc(std::forward<F>(f));
	}
private:
	std::vector<AABB*> boxes;
	std::vector<Entity*> entities;
	std::unordered_map<uint64_t, CollisionFunc> funcs;
};

#endif

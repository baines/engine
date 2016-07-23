#ifndef COLLISION_SYSTEM_H_
#define COLLISION_SYSTEM_H_
#include "common.h"
#include <vector>
#include <map>
#include <functional>

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

//extern template class std::vector<AABB*>;
//extern template class std::vector<Entity*>;
//extern template struct alt::Array<uint32_t, 2>;

struct CollisionSystem {

	CollisionSystem();

	void addEntity(Entity& e);

	void update(uint32_t delta);

	typedef std::function<void(Entity* a, Entity* b, float t)> CollisionFunc;

	void onCollision(uint32_t a, uint32_t b, CollisionFunc&& f);

private:
	std::vector<AABB*> boxes;
	std::vector<Entity*> entities;
	std::map<Array<uint32_t, 2>, CollisionFunc> funcs;
};

//extern template class std::map<Array<uint32_t, 2>, CollisionSystem::CollisionFunc>;

#endif

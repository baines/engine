#ifndef COLLISION_SYSTEM_H_
#define COLLISION_SYSTEM_H_
#include "common.h"
#include "glm/glm.hpp"
#include <vector>
#include <map>

struct AABB {
	AABB();
	AABB(glm::vec2 size, uint32_t group = 0);

	void setPosition(glm::vec2 p);
	void setPrevPosition(glm::vec2 p);

	void initComponent(Engine&, Entity&);
	//TODO: offset;
	glm::vec2 pos, prev_pos, size;

	uint32_t collision_group;
};

struct CollisionSystem {

	CollisionSystem();

	void addEntity(Entity& e);

	void update(uint32_t delta);

	typedef std::function<void(Entity* a, Entity* b, float t)> CollisionFunc;

	void onCollision(uint32_t a, uint32_t b, CollisionFunc&& f);

private:
	std::vector<AABB*> boxes;
	std::vector<Entity*> entities;
	std::map<std::array<uint32_t, 2>, CollisionFunc> funcs;
};

#endif

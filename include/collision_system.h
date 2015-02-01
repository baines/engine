#ifndef COLLISION_SYSTEM_H_
#define COLLISION_SYSTEM_H_
#include "common.h"
#include <vector>

struct AABB {
	AABB();
	AABB(glm::vec2 size);

	void setPosition(glm::vec2 p);
	void setPrevPosition(glm::vec2 p);

	void initComponent(Engine&, Entity&);
	//TODO: offset;
	glm::vec2 pos, prev_pos, size;
};

struct CollisionSystem {

	CollisionSystem();

	void addEntity(Entity& e);

	void update(uint32_t delta);

private:
	std::vector<AABB*> boxes;
};

#endif

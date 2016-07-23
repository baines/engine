#ifndef TEST_COLLISION_STATE_H_
#define TEST_COLLISION_STATE_H_
#include "engine_all.h"
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum {
	ACT_LEFT,
	ACT_RIGHT,
	ACT_UP,
	ACT_DOWN,
	ACT_SWITCH,
	ACT_CLICK,
	ACT_CURSOR_X,
	ACT_CURSOR_Y
};

struct TestEntity : public EntityWith<Position2D, Sprite, AABB> {
	TestEntity(Engine& e, SpriteBatch& b, vec2i pos)
	: EntityWith(e,	
		vec2{ float(pos.x), float(pos.y) },
		{ b, pos + 32, vec2i { 64, 64 }},
		vec2{ 64.f, 64.f }
	){
	}
};

struct TestCollisionState : public GameState {

	TestCollisionState(Engine& e)
	: sprite_vs     (e, {"sprite.glslv"})
	, sprite_fs     (e, {"sprite.glslf"})
	, sprite_shader (sprite_vs, sprite_fs)
	, samp_nearest  ({{ GL_TEXTURE_MAG_FILTER, GL_NEAREST }})
	, sprite_tex    (e, {"test_sprite.png"})
	, sprite_mat    (sprite_shader, *sprite_tex, samp_nearest)
	, sprite_batch  (sprite_mat)
	, entities      {{{ e, sprite_batch, { 100, 100 }}, {e, sprite_batch, { 200, 200 }}}}
	, active_entity (0)
	, prev_pos      {{{ 100, 100 }, { 100, 100 }}}
	, move          ({ 0.f, 0.f })
	, cursor        ({ 0.f, 0.f })
	, canvas        (e) {
		e.input->subscribe(this, "left" , ACT_LEFT);
		e.input->subscribe(this, "right", ACT_RIGHT);
		e.input->subscribe(this, "up"   , ACT_UP);
		e.input->subscribe(this, "down" , ACT_DOWN);

		e.input->subscribe(this, "switch", ACT_SWITCH);
		e.input->subscribe(this, "click", ACT_CLICK);

		e.input->subscribe(this, "cursor_x", ACT_CURSOR_X);
		e.input->subscribe(this, "cursor_y", ACT_CURSOR_Y);

		e.collision->onCollision(0, 0, [&](Entity* a, Entity* b, float t){
			for(auto* e : { a, b }){
				if(auto* aabb = e->get<AABB>()){
					vec2 pos = lerp(aabb->prev_pos + 32.f, aabb->pos + 32.f, t);
					canvas.addBox(pos,{ 64.f, 64.f }, 0xff0000ff);
				}
			}
		});
	}
	
	bool onInit(Engine& e){
		sprite_shader.link();
		
		return true;
	}

	bool onInput(Engine& e, int action, bool pressed){

		switch(action){
			case ACT_LEFT:   move.x = pressed ? -4 : move.x == -4 ? 0 : move.x; break;
			case ACT_RIGHT:  move.x = pressed ?  4 : move.x ==  4 ? 0 : move.x; break;
			case ACT_UP:     move.y = pressed ? -4 : move.y == -4 ? 0 : move.y; break;
			case ACT_DOWN:   move.y = pressed ?  4 : move.y ==  4 ? 0 : move.y; break;
			case ACT_SWITCH: if(pressed) active_entity ^= 1; break;
			case ACT_CLICK:  if(pressed) prev_pos[active_entity] = cursor; break;
		}

		return true;
	}

	bool onMotion(Engine& e, int action, int value, bool rel){
		switch(action){
			case ACT_CURSOR_X: cursor.x = value; break;
			case ACT_CURSOR_Y: cursor.y = value; break;
		}
		return true;
	}
	
	void update(Engine& e, uint32_t delta){
		entities[active_entity].get<Position2D>().add(move);

		canvas.clear();

		for(int i = 0; i < 2; ++i){
			entities[i].get<AABB>().prev_pos = prev_pos[i] - 32.f;
			canvas.addLine(entities[i].get<Position2D>().get(), prev_pos[i], 0x00ff00ff);
		}
	}
	
	void draw(IRenderer& renderer){
		sprite_batch.draw(renderer);
		canvas.draw(renderer);
	}
private:

	Resource<VertShader> sprite_vs;
	Resource<FragShader> sprite_fs;
	ShaderProgram sprite_shader;

	Sampler samp_nearest;
	Resource<Texture2D> sprite_tex;
	Material sprite_mat;
	SpriteBatch sprite_batch;
	
	Array<TestEntity, 2> entities;
	int active_entity;
	Array<vec2, 2> prev_pos;

	vec2 move, cursor;
	Canvas canvas;
};

#endif

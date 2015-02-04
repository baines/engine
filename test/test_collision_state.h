#ifndef TEST_COLLISION_STATE_H_
#define TEST_COLLISION_STATE_H_
#include "engine.h"
#include "game_state.h"
#include "resource.h"
#include "shader.h"
#include "font.h"
#include "text.h"
#include "texture.h"
#include "material.h"
#include "sprite_batch.h"
#include "sprite.h"
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "entity.h"
#include "canvas.h"

enum {
	ACT_LEFT,
	ACT_RIGHT,
	ACT_UP,
	ACT_DOWN
};

struct TestEntity : public EntityWith<Position2D, Sprite, AABB> {
	TestEntity(Engine& e, SpriteBatch& b, glm::ivec2 pos)
	: EntityWith(e,	
		glm::vec2{ pos.x, pos.y },
		{ b, pos + 32, glm::ivec2{ 64, 64 }},
		glm::vec2{ 64.f, 64.f }
	){
	}
};

struct TestCollisionState : public GameState {

	TestCollisionState(Engine& e)
	: sprite_vs     (e, {"sprite.glslv"})
	, sprite_fs     (e, {"sprite.glslf"})
	, sprite_shader (*sprite_vs, *sprite_fs)
	, samp_nearest  ({{ GL_TEXTURE_MAG_FILTER, GL_NEAREST }})
	, sprite_tex    (e, {"test_sprite.png"})
	, sprite_mat    (sprite_shader, *(*sprite_tex), samp_nearest)
	, sprite_batch  (sprite_mat)
	, entity        (e, sprite_batch, { 100, 100 })
	, entity2       (e, sprite_batch, { 320, 240 })
	, move          ({ 0.f, 0.f })
	, canvas        (e) {
		e.input.watchAction(this, "left" , ACT_LEFT);
		e.input.watchAction(this, "right", ACT_RIGHT);
		e.input.watchAction(this, "up"   , ACT_UP);
		e.input.watchAction(this, "down" , ACT_DOWN);

		canvas.addLine({ 0, 0 }, { 100, 100 }, 0x00ff00ff);
	}
	
	bool onInit(Engine& e){
		sprite_shader.link();
		
		return true;
	}

	bool onInput(Engine& e, int action, bool pressed){

		switch(action){
			case ACT_LEFT:  move.x = pressed ? -4 : move.x == -4 ? 0 : move.x; break;
			case ACT_RIGHT: move.x = pressed ?  4 : move.x ==  4 ? 0 : move.x; break;
			case ACT_UP:    move.y = pressed ? -4 : move.y == -4 ? 0 : move.y; break;
			case ACT_DOWN:  move.y = pressed ?  4 : move.y ==  4 ? 0 : move.y; break;
		}

		return true;
	}
	
	void update(Engine& e, uint32_t delta){
		entity.get<Position2D>().add(move);
	}
	
	void draw(Renderer& renderer){
		sprite_batch.draw(renderer);
		//canvas.draw(renderer);
	}
private:

	Resource<VertShader> sprite_vs;
	Resource<FragShader> sprite_fs;
	ShaderProgram sprite_shader;

	Sampler samp_nearest;
	Resource<Texture2D> sprite_tex;
	Material sprite_mat;
	SpriteBatch sprite_batch;
	
	TestEntity entity, entity2;

	glm::vec2 move;
	Canvas canvas;
};

#endif
